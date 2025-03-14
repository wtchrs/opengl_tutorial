#include <cstddef>
#include <format>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/geometric.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/trigonometric.hpp>
#include <imgui.h>
#include <memory>
#include <random>
#include <spdlog/spdlog.h>
#include "glex/common.h"
#include "glex/context.h"
#include "glex/framebuffer.h"
#include "glex/image.h"
#include "glex/mesh.h"
#include "glex/model.h"
#include "glex/texture.h"

struct DeferLight {
    glm::vec3 position;
    glm::vec3 color;
};

struct Object {
    glm::vec3 pos;
    glm::vec3 scale;
    glm::vec3 rotDir;
    float rotAngle;
    std::shared_ptr<Mesh> mesh;
    std::shared_ptr<Material> material;
    bool outline;
};

struct ContextInternal {
    std::unique_ptr<Program> simple_program_, deferred_geo_program_, deferred_light_program_, ssao_program_,
            blur_program_;
    std::unique_ptr<FrameBuffer> geo_framebuffer_, ssao_framebuffer_, blur_framebuffer_;

    std::unique_ptr<Model> backpack_model_;
    std::unique_ptr<Texture> ssao_noise_texture_;
    std::shared_ptr<Mesh> cube_mesh_, plain_mesh_;
    std::shared_ptr<Material> floor_material_, cube_material1_, cube_material2_;
};
auto ctx = std::make_unique<ContextInternal>();

std::vector<glm::vec3> ssao_samples{16};
float ssao_radius{1.0f};
float ssao_power{1.0f};
bool use_ssao{false};

///@{
/// Default parameters
static constexpr float CAMERA_PITCH{-40.0f}; ///< Camera pitch
static constexpr float CAMERA_YAW{40.0f}; ///< Camera yaw

static constexpr glm::vec3 CAMERA_POS{3.0f, 6.0f, 6.0f}; ///< Camera position
static constexpr glm::vec3 CAMERA_FRONT{0.0f, 0.0f, -1.0f}; ///< Direction that camera is looking
static constexpr glm::vec3 CAMERA_UP{0.0f, 1.0f, 0.0f}; ///< Camera up vector
///@}

/// Lighting parameters
std::vector<DeferLight> deferred_lights{32};

///@{
/// Camera parameters
float camera_pitch{CAMERA_PITCH}; ///< Camera pitch
float camera_yaw{CAMERA_YAW}; ///< Camera yaw

glm::vec3 camera_pos{CAMERA_POS}; ///< Camera position
glm::vec3 camera_front{CAMERA_FRONT}; ///< Direction that camera is looking
glm::vec3 camera_up{CAMERA_UP}; ///< Camera up vector

bool camera_rot_control{false}; ///< Camera control flag
glm::vec2 prev_mouse_pos{0.0f}; ///< Previous mouse position
///@}

/// Window width and height
int window_width{WINDOW_WIDTH}, window_height{WINDOW_HEIGHT};
/// Aspect ratio of window
float aspect_ratio{static_cast<float>(WINDOW_WIDTH) / static_cast<float>(WINDOW_HEIGHT)};

std::unique_ptr<Context> Context::create() {
    auto context = std::unique_ptr<Context>{new Context{}};
    if (!context->init()) {
        SPDLOG_ERROR("Failed to create context");
        return nullptr;
    }
    SPDLOG_INFO("Context has been created");
    return std::move(context);
}

Context::~Context() {
    SPDLOG_INFO("Delete ctx");
    ctx.reset();
}

bool Context::init() {
    // Create meshes.
    ctx->cube_mesh_ = Mesh::create_cube();
    ctx->plain_mesh_ = Mesh::create_plain();

    // Load model.
    ctx->backpack_model_ = Model::load("./model/backpack/backpack.obj");

    // Load programs.
    ctx->simple_program_ = Program::create("./shader/simple.vs", "./shader/simple.fs");
    ctx->deferred_geo_program_ = Program::create("./shader/defer_geo.vs", "./shader/defer_geo.fs");
    ctx->deferred_light_program_ = Program::create("./shader/defer_light.vs", "./shader/defer_light.fs");
    ctx->ssao_program_ = Program::create("./shader/ssao.vs", "./shader/ssao.fs");
    ctx->blur_program_ = Program::create("./shader/blur_5x5.vs", "./shader/blur_5x5.fs");

    if (!ctx->simple_program_ || !ctx->deferred_geo_program_ || !ctx->deferred_light_program_ || !ctx->ssao_program_ ||
        !ctx->blur_program_) {
        SPDLOG_ERROR("Failed to initialize context");
        return false;
    }

    // Create dark gray single color texture.
    auto dark_gray_image = Image::create(512, 512);
    dark_gray_image->set_single_color_image({0.2f, 0.2f, 0.2f, 1.0f});
    std::shared_ptr dark_gray_texture = Texture::create(*dark_gray_image);
    // Create gray single color texture.
    auto gray_image = Image::create(512, 512);
    gray_image->set_single_color_image({0.5f, 0.5f, 0.5f, 1.0f});
    std::shared_ptr gray_texture = Texture::create(*gray_image);

    // Create plain material.
    std::shared_ptr plain_diffuse = Texture::create(*Image::load("./image/marble.jpg"));
    ctx->floor_material_ = std::make_shared<Material>(plain_diffuse, gray_texture, 8.0f);

    // Create cube1 material.
    std::shared_ptr cube_diffuse1 = Texture::create(*Image::load("./image/container.jpg"));
    ctx->cube_material1_ = std::make_shared<Material>(cube_diffuse1, dark_gray_texture, 16.0f);

    // Create cube2 material.
    std::shared_ptr cube_diffuse2 = Texture::create(*Image::load("./image/container2.png"));
    std::shared_ptr cube_specular2 = Texture::create(*Image::load("./image/container2_specular.png"));
    ctx->cube_material2_ = std::make_shared<Material>(cube_diffuse2, cube_specular2, 64.0f);

    std::random_device rd;
    std::mt19937 gen{rd()};
    std::uniform_real_distribution<float> dis_xz{-10.0f, 10.0f};
    std::uniform_real_distribution<float> dis_y{1.0f, 4.0f};
    std::uniform_real_distribution<float> dis_color{0.05f, 0.3f};
    std::uniform_real_distribution<float> dis_neg_one_to_one{-1.0f, 1.0f};
    std::uniform_real_distribution<float> dis_zero_to_one{0.0f, 1.0f};

    for (size_t i = 0; i < deferred_lights.size(); ++i) {
        auto &light = deferred_lights[i];
        light.position = glm::vec3{dis_xz(gen), dis_y(gen), dis_xz(gen)};
        light.color =
                glm::vec3{i < 3 ? dis_color(gen) : 0.0f, i < 3 ? dis_color(gen) : 0.0f, i < 3 ? dis_color(gen) : 0.0f};
    }

    std::vector<glm::vec3> ssao_noise{16};
    for (auto &noise : ssao_noise) {
        noise = glm::vec3{dis_neg_one_to_one(gen), dis_neg_one_to_one(gen), 0.0f};
    }
    ctx->ssao_noise_texture_ = Texture::create(4, 4, GL_RGB16F, GL_FLOAT);
    ctx->ssao_noise_texture_->bind();
    ctx->ssao_noise_texture_->set_filter(GL_NEAREST, GL_NEAREST);
    ctx->ssao_noise_texture_->set_wrap(GL_REPEAT, GL_REPEAT);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 4, 4, GL_RGB, GL_FLOAT, ssao_noise.data());

    for (size_t i = 0; i < ssao_samples.size(); ++i) {
        auto sample = glm::vec3{dis_neg_one_to_one(gen), dis_neg_one_to_one(gen), dis_zero_to_one(gen)};
        sample = glm::normalize(sample) * dis_zero_to_one(gen);
        // Slightly shift to center.
        float t = static_cast<float>(i) / static_cast<float>(ssao_samples.size());
        float t2 = t * t;
        float scale = (1.0f - t2) * 0.1f + t2;
        ssao_samples[i] = sample * scale;
    }

    // Enable depth test and cull face.
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    // Set clear color.
    glClearColor(0.0f, 0.1f, 0.2f, 0.0f);

    return true;
}

/// Same function already exists in glm library (glm::lookAt)
[[maybe_unused]]
static glm::mat4 get_view_transform(const glm::vec3 &position, const glm::vec3 &target, const glm::vec3 &upvector) {
    auto z = glm::normalize(position - target);
    auto x = glm::normalize(glm::cross(upvector, z));
    auto y = glm::cross(z, x);
    auto cameraMat = glm::mat4{glm::vec4{x, 0.0f}, glm::vec4{y, 0.0f}, glm::vec4{z, 0.0f}, glm::vec4{position, 1.0f}};
    return glm::inverse(cameraMat);
}

void Context::render() {
    // Clear color buffer with `glClearColor` and depth buffer with 1.0.
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    // Dear ImGui UI
    draw_ui();

    // Calculate camera front direction.
    camera_front = glm::rotate(glm::mat4{1.0f}, glm::radians(camera_yaw), glm::vec3{0.0f, 1.0f, 0.0f}) *
                   glm::rotate(glm::mat4{1.0f}, glm::radians(camera_pitch), glm::vec3{1.0f, 0.0f, 0.0f}) *
                   glm::vec4{0.0f, 0.0f, -1.0f, 0.0f};

    // Projection and view matrix
    // When the `Near` value is too small, inaccurate depth test, known as "z-fighting", arise on far objects,
    // due to the z-value distortion introduced by the projection transform.
    const auto projection = glm::perspective(glm::radians(45.0f), aspect_ratio, 0.1f, 100.0f);
    const auto view = glm::lookAt(camera_pos, camera_pos + camera_front, camera_up);

    // Render first path.
    ctx->geo_framebuffer_->bind();
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0, window_width, window_height);
    draw_scene(view, projection, *ctx->deferred_geo_program_);

    // SSAO path.
    ctx->ssao_framebuffer_->bind();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0, window_width, window_height);
    ctx->ssao_program_->use();
    glActiveTexture(GL_TEXTURE0);
    ctx->geo_framebuffer_->get_color_attachment(0)->bind();
    glActiveTexture(GL_TEXTURE1);
    ctx->geo_framebuffer_->get_color_attachment(1)->bind();
    glActiveTexture(GL_TEXTURE2);
    ctx->ssao_noise_texture_->bind();
    glActiveTexture(GL_TEXTURE0);
    ctx->ssao_program_->set_uniform("gPosition", 0);
    ctx->ssao_program_->set_uniform("gNormal", 1);
    ctx->ssao_program_->set_uniform("texNoise", 2);
    const auto noise_scale = glm::vec2{
            static_cast<float>(window_width) / static_cast<float>(ctx->ssao_noise_texture_->get_width()),
            static_cast<float>(window_height) / static_cast<float>(ctx->ssao_noise_texture_->get_height()),
    };
    ctx->ssao_program_->set_uniform("noiseScale", noise_scale);
    ctx->ssao_program_->set_uniform("radius", ssao_radius);
    ctx->ssao_program_->set_uniform("power", ssao_power);
    for (size_t i = 0; i < ssao_samples.size(); ++i) {
        auto sample_name = std::format("samples[{}]", i);
        ctx->ssao_program_->set_uniform(sample_name, ssao_samples[i]);
    }
    ctx->ssao_program_->set_uniform("transform", glm::scale(glm::mat4{1.0f}, glm::vec3{2.0f}));
    ctx->ssao_program_->set_uniform("view", view);
    ctx->ssao_program_->set_uniform("projection", projection);
    ctx->plain_mesh_->draw(*ctx->ssao_program_);

    // Blur SSAO result.
    ctx->blur_framebuffer_->bind();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0, window_width, window_height);
    ctx->blur_program_->use();
    glActiveTexture(GL_TEXTURE0);
    ctx->ssao_framebuffer_->get_color_attachment()->bind();
    ctx->blur_program_->set_uniform("tex", 0);
    ctx->blur_program_->set_uniform("transform", glm::scale(glm::mat4{1.0f}, glm::vec3{2.0f}));
    ctx->plain_mesh_->draw(*ctx->blur_program_);

    // Set to default framebuffer.
    FrameBuffer::bind_to_default();
    glViewport(0, 0, window_width, window_height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    // Render last path.
    ctx->deferred_light_program_->use();
    for (size_t i = 0; i < 3; ++i) {
        glActiveTexture(GL_TEXTURE0 + i);
        ctx->geo_framebuffer_->get_color_attachment(i)->bind();
    }
    glActiveTexture(GL_TEXTURE3);
    ctx->blur_framebuffer_->get_color_attachment()->bind();
    glActiveTexture(GL_TEXTURE0);
    ctx->deferred_light_program_->set_uniform("gPosition", 0);
    ctx->deferred_light_program_->set_uniform("gNormal", 1);
    ctx->deferred_light_program_->set_uniform("gAlbedoSpec", 2);
    ctx->deferred_light_program_->set_uniform("ssao", 3);
    ctx->deferred_light_program_->set_uniform("useSsao", use_ssao);
    for (size_t i = 0; i < deferred_lights.size(); ++i) {
        auto pos_name = std::format("lights[{}].position", i);
        auto color_name = std::format("lights[{}].color", i);
        ctx->deferred_light_program_->set_uniform(pos_name, deferred_lights[i].position);
        ctx->deferred_light_program_->set_uniform(color_name, deferred_lights[i].color);
    }
    ctx->deferred_light_program_->set_uniform("transform", glm::scale(glm::mat4{1.0f}, glm::vec3{2.0f}));
    ctx->plain_mesh_->draw(*ctx->deferred_light_program_);

    // Copy depth buffer to the default framebuffer.
    glBindFramebuffer(GL_READ_FRAMEBUFFER, ctx->geo_framebuffer_->get());
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBlitFramebuffer(
            0, 0, window_width, window_height, 0, 0, window_width, window_height, GL_DEPTH_BUFFER_BIT, GL_NEAREST
    );
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Draw cube for indicating light positions.
    ctx->simple_program_->use();
    for (const auto &light : deferred_lights) {
        const auto light_model =
                glm::translate(glm::mat4{1.0f}, light.position) * glm::scale(glm::mat4{1.0}, glm::vec3{0.1f});
        ctx->simple_program_->set_uniform("color", glm::vec4{light.color, 1.0f});
        ctx->simple_program_->set_uniform("transform", projection * view * light_model);
        ctx->cube_mesh_->draw(*ctx->simple_program_);
    }
}

void Context::draw_scene(const glm::mat4 &view, const glm::mat4 &projection, const Program &program) {
    static const Object cubes[] = {
            {{0.0f, -0.5f, 0.0f},
             {40.0f, 1.0f, 40.0f},
             {1.0f, 0.0f, 0.0f},
             0.0f,
             ctx->cube_mesh_,
             ctx->floor_material_,
             false},
            {{-1.0f, 0.75f, -4.0f},
             {1.5f, 1.5f, 1.5f},
             {0.0f, 1.0f, 0.0f},
             30.0f,
             ctx->cube_mesh_,
             ctx->cube_material1_,
             false},
            {{0.0f, 0.75f, 2.0f},
             {1.5f, 1.5f, 1.5f},
             {0.0f, 1.0f, 0.0f},
             20.0f,
             ctx->cube_mesh_,
             ctx->cube_material2_,
             false},
            {{3.0f, 1.75f, -2.0f},
             {1.5f, 1.5f, 1.5f},
             {0.0f, 1.0f, 0.0f},
             50.0f,
             ctx->cube_mesh_,
             ctx->cube_material2_,
             false},
    };

    program.use();

    for (const auto &[pos, scale, rot_dir, angle, mesh, material, outline] : cubes) {
        auto model_transform = glm::translate(glm::mat4{1.0f}, pos) * glm::scale(glm::mat4{1.0f}, scale) *
                               glm::rotate(glm::mat4{1.0f}, glm::radians(angle), rot_dir);
        auto transform = projection * view * model_transform;
        program.set_uniform("transform", transform);
        program.set_uniform("modelTransform", model_transform);
        material->set_to_program(program);
        mesh->draw(program);
    }

    auto model_transform = glm::translate(glm::mat4{1.0f}, glm::vec3{0.0f, 0.55f, 0.0f}) *
                           glm::rotate(glm::mat4{1.0f}, glm::radians(-90.0f), glm::vec3{1.0f, 0.0f, 0.0f}) *
                           glm::scale(glm::mat4{1.0f}, glm::vec3{0.5f});
    auto transform = projection * view * model_transform;
    program.set_uniform("transform", transform);
    program.set_uniform("modelTransform", model_transform);
    ctx->backpack_model_->draw(program);
}

void Context::draw_ui() {
    // ImGui Components.
    if (ImGui::Begin("UI")) {
        if (ImGui::CollapsingHeader("Camera", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::DragFloat3("Position", glm::value_ptr(camera_pos), 0.1f);
            ImGui::DragFloat("Yaw", &camera_yaw, 0.5f);
            ImGui::DragFloat("Pitch", &camera_pitch, 0.5f, -89.0f, 89.0f);
        }
        ImGui::Separator();
        if (ImGui::CollapsingHeader("Lighting", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::Checkbox("Use SSAO", &use_ssao);
            ImGui::DragFloat("SSAO radius", &ssao_radius, 0.01f, 0.0f, 5.0f);
            ImGui::DragFloat("SSAO power", &ssao_power, 0.01f, 0.0f, 5.0f);
        }
        ImGui::Separator();
        if (ImGui::Button("Reset")) {
            camera_pos = CAMERA_POS;
            camera_yaw = CAMERA_YAW;
            camera_pitch = CAMERA_PITCH;
        }
    }
    ImGui::End();

    // G-buffer
    if (ImGui::Begin("G-buffer")) {
        const char *buffer_names[] = {"Position", "Normal", "Albedo/Specular"};
        static int buffer_select = 0;
        ImGui::Combo("buffer", &buffer_select, buffer_names, 3);
        float width = ImGui::GetContentRegionAvail().x;
        float height = width / aspect_ratio;
        auto attachment = ctx->geo_framebuffer_->get_color_attachment(buffer_select);
        ImGui::Image(static_cast<ImTextureID>(attachment->get()), ImVec2{width, height}, ImVec2{0, 1}, ImVec2{1, 0});
    }
    ImGui::End();

    // SSAO buffer
    if (ImGui::Begin("SSAO")) {
        const char *buffer_names[] = {"Original", "Blurred"};
        static int buffer_select = 0;
        ImGui::Combo("Buffer", &buffer_select, buffer_names, 2);
        float width = ImGui::GetContentRegionAvail().x;
        float height = width / aspect_ratio;
        auto attachment =
                (buffer_select == 0 ? ctx->ssao_framebuffer_ : ctx->blur_framebuffer_)->get_color_attachment();
        ImGui::Image(static_cast<ImTextureID>(attachment->get()), ImVec2{width, height}, ImVec2{0, 1}, ImVec2{1, 0});
    }
    ImGui::End();
}

void Context::process_input(GLFWwindow *window) {
    constexpr auto camera_speed = 0.05f;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        camera_pos += camera_speed * camera_front;
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        camera_pos -= camera_speed * camera_front;
    }

    auto camera_right = -glm::normalize(glm::cross(camera_up, camera_front));
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        camera_pos += camera_speed * camera_right;
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        camera_pos -= camera_speed * camera_right;
    }

    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
        camera_pos += camera_speed * camera_up;
    }
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
        camera_pos -= camera_speed * camera_up;
    }
}

void Context::reshape(const int width, const int height) {
    window_width = width;
    window_height = height;
    aspect_ratio = static_cast<float>(width) / static_cast<float>(height);
    ctx->geo_framebuffer_ = FrameBuffer::create({
            Texture::create(width, height, GL_RGBA16F, GL_FLOAT),
            Texture::create(width, height, GL_RGBA16F, GL_FLOAT),
            Texture::create(width, height, GL_RGBA),
    });
    ctx->ssao_framebuffer_ = FrameBuffer::create({Texture::create(width, height, GL_RED, GL_FLOAT)});
    ctx->blur_framebuffer_ = FrameBuffer::create({Texture::create(width, height, GL_RED, GL_FLOAT)});
}

void Context::mouse_move(const double x, const double y) {
    if (!camera_rot_control)
        return;

    const glm::vec2 cur{static_cast<float>(x), static_cast<float>(y)};
    const auto delta_pos = cur - prev_mouse_pos;

    constexpr float CAMERA_ROT_SPEED = 0.4f;
    camera_yaw -= delta_pos.x * CAMERA_ROT_SPEED;
    camera_pitch -= delta_pos.y * CAMERA_ROT_SPEED;

    if (camera_yaw < 0.0f) {
        camera_yaw += 360.0f;
    }
    if (camera_yaw > 360.0f) {
        camera_yaw -= 360.0f;
    }
    if (camera_pitch > 89.0f) {
        camera_pitch = 89.0f;
    }
    if (camera_pitch < -89.0f) {
        camera_pitch = -89.0f;
    }

    prev_mouse_pos = cur;
}

void Context::mouse_button(const int button, const int action, const double x, const double y) {
    if (button == GLFW_MOUSE_BUTTON_RIGHT) {
        if (action == GLFW_PRESS) {
            prev_mouse_pos = glm::vec2{x, y};
            camera_rot_control = true;
        } else {
            camera_rot_control = false;
        }
    }
}
