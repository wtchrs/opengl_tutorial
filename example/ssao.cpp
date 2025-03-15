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

struct Object {
    glm::vec3 pos;
    glm::vec3 scale;
    glm::vec3 rotDir;
    float rotAngle;
    std::shared_ptr<Mesh> mesh;
    std::shared_ptr<Material> material;
    bool outline;
};

class SSAO : Context {
    std::unique_ptr<Program> simple_program_, deferred_geo_program_, deferred_light_program_, ssao_program_,
            blur_program_;
    std::unique_ptr<FrameBuffer> geo_framebuffer_, ssao_framebuffer_, blur_framebuffer_;

    std::unique_ptr<Model> backpack_model_;
    std::unique_ptr<Texture> ssao_noise_texture_;
    std::shared_ptr<Mesh> cube_mesh_, plain_mesh_;
    std::shared_ptr<Material> floor_material_, cube_material1_, cube_material2_;

    struct DeferLight {
        glm::vec3 position;
        glm::vec3 color;
    };
    std::vector<DeferLight> deferred_lights{32};

    std::vector<glm::vec3> ssao_samples{16};
    float ssao_radius{1.0f};
    float ssao_power{1.0f};
    bool use_ssao{false};

public:
    bool init();
    void render();
    void draw_ui();
    void draw_scene(const glm::mat4 &view, const glm::mat4 &projection, const Program &program);
    void reshape(int width, int height);
};

std::unique_ptr<Context> Context::create() {
    auto context = std::unique_ptr<Context>{reinterpret_cast<Context *>(new SSAO{})};
    if (!context->init()) {
        SPDLOG_ERROR("Failed to create context");
        return nullptr;
    }
    SPDLOG_INFO("Context has been created");
    return std::move(context);
}

bool SSAO::init() {
    // Create meshes.
    cube_mesh_ = Mesh::create_cube();
    plain_mesh_ = Mesh::create_plain();

    // Load model.
    backpack_model_ = Model::load("./model/backpack/backpack.obj");

    // Load programs.
    simple_program_ = Program::create("./shader/simple.vs", "./shader/simple.fs");
    deferred_geo_program_ = Program::create("./shader/defer_geo.vs", "./shader/defer_geo.fs");
    deferred_light_program_ = Program::create("./shader/defer_light.vs", "./shader/defer_light.fs");
    ssao_program_ = Program::create("./shader/ssao.vs", "./shader/ssao.fs");
    blur_program_ = Program::create("./shader/blur_5x5.vs", "./shader/blur_5x5.fs");

    if (!simple_program_ || !deferred_geo_program_ || !deferred_light_program_ || !ssao_program_ || !blur_program_) {
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
    floor_material_ = std::make_shared<Material>(plain_diffuse, gray_texture, 8.0f);

    // Create cube1 material.
    std::shared_ptr cube_diffuse1 = Texture::create(*Image::load("./image/container.jpg"));
    cube_material1_ = std::make_shared<Material>(cube_diffuse1, dark_gray_texture, 16.0f);

    // Create cube2 material.
    std::shared_ptr cube_diffuse2 = Texture::create(*Image::load("./image/container2.png"));
    std::shared_ptr cube_specular2 = Texture::create(*Image::load("./image/container2_specular.png"));
    cube_material2_ = std::make_shared<Material>(cube_diffuse2, cube_specular2, 64.0f);

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
    ssao_noise_texture_ = Texture::create(4, 4, GL_RGB16F, GL_FLOAT);
    ssao_noise_texture_->bind();
    ssao_noise_texture_->set_filter(GL_NEAREST, GL_NEAREST);
    ssao_noise_texture_->set_wrap(GL_REPEAT, GL_REPEAT);
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

void SSAO::render() {
    // Clear color buffer with `glClearColor` and depth buffer with 1.0.
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    // Dear ImGui UI
    draw_ui();

    // Calculate camera front direction.
    camera_front_ = glm::rotate(glm::mat4{1.0f}, glm::radians(camera_yaw_), glm::vec3{0.0f, 1.0f, 0.0f}) *
                    glm::rotate(glm::mat4{1.0f}, glm::radians(camera_pitch_), glm::vec3{1.0f, 0.0f, 0.0f}) *
                    glm::vec4{0.0f, 0.0f, -1.0f, 0.0f};

    // Projection and view matrix
    // When the `Near` value is too small, inaccurate depth test, known as "z-fighting", arise on far objects,
    // due to the z-value distortion introduced by the projection transform.
    const auto projection = glm::perspective(glm::radians(45.0f), aspect_ratio_, 0.1f, 100.0f);
    const auto view = glm::lookAt(camera_pos_, camera_pos_ + camera_front_, camera_up_);

    // Render first path.
    geo_framebuffer_->bind();
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0, width_, height_);
    draw_scene(view, projection, *deferred_geo_program_);

    // SSAO path.
    ssao_framebuffer_->bind();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0, width_, height_);
    ssao_program_->use();
    glActiveTexture(GL_TEXTURE0);
    geo_framebuffer_->get_color_attachment(0)->bind();
    glActiveTexture(GL_TEXTURE1);
    geo_framebuffer_->get_color_attachment(1)->bind();
    glActiveTexture(GL_TEXTURE2);
    ssao_noise_texture_->bind();
    glActiveTexture(GL_TEXTURE0);
    ssao_program_->set_uniform("gPosition", 0);
    ssao_program_->set_uniform("gNormal", 1);
    ssao_program_->set_uniform("texNoise", 2);
    const auto noise_scale = glm::vec2{
            static_cast<float>(width_) / static_cast<float>(ssao_noise_texture_->get_width()),
            static_cast<float>(height_) / static_cast<float>(ssao_noise_texture_->get_height()),
    };
    ssao_program_->set_uniform("noiseScale", noise_scale);
    ssao_program_->set_uniform("radius", ssao_radius);
    ssao_program_->set_uniform("power", ssao_power);
    for (size_t i = 0; i < ssao_samples.size(); ++i) {
        auto sample_name = std::format("samples[{}]", i);
        ssao_program_->set_uniform(sample_name, ssao_samples[i]);
    }
    ssao_program_->set_uniform("transform", glm::scale(glm::mat4{1.0f}, glm::vec3{2.0f}));
    ssao_program_->set_uniform("view", view);
    ssao_program_->set_uniform("projection", projection);
    plain_mesh_->draw(*ssao_program_);

    // Blur SSAO result.
    blur_framebuffer_->bind();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0, width_, height_);
    blur_program_->use();
    glActiveTexture(GL_TEXTURE0);
    ssao_framebuffer_->get_color_attachment()->bind();
    blur_program_->set_uniform("tex", 0);
    blur_program_->set_uniform("transform", glm::scale(glm::mat4{1.0f}, glm::vec3{2.0f}));
    plain_mesh_->draw(*blur_program_);

    // Set to default framebuffer.
    FrameBuffer::bind_to_default();
    glViewport(0, 0, width_, height_);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    // Render last path.
    deferred_light_program_->use();
    for (size_t i = 0; i < 3; ++i) {
        glActiveTexture(GL_TEXTURE0 + i);
        geo_framebuffer_->get_color_attachment(i)->bind();
    }
    glActiveTexture(GL_TEXTURE3);
    blur_framebuffer_->get_color_attachment()->bind();
    glActiveTexture(GL_TEXTURE0);
    deferred_light_program_->set_uniform("gPosition", 0);
    deferred_light_program_->set_uniform("gNormal", 1);
    deferred_light_program_->set_uniform("gAlbedoSpec", 2);
    deferred_light_program_->set_uniform("ssao", 3);
    deferred_light_program_->set_uniform("useSsao", use_ssao);
    for (size_t i = 0; i < deferred_lights.size(); ++i) {
        auto pos_name = std::format("lights[{}].position", i);
        auto color_name = std::format("lights[{}].color", i);
        deferred_light_program_->set_uniform(pos_name, deferred_lights[i].position);
        deferred_light_program_->set_uniform(color_name, deferred_lights[i].color);
    }
    deferred_light_program_->set_uniform("transform", glm::scale(glm::mat4{1.0f}, glm::vec3{2.0f}));
    plain_mesh_->draw(*deferred_light_program_);

    // Copy depth buffer to the default framebuffer.
    glBindFramebuffer(GL_READ_FRAMEBUFFER, geo_framebuffer_->get());
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBlitFramebuffer(0, 0, width_, height_, 0, 0, width_, height_, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Draw cube for indicating light positions.
    simple_program_->use();
    for (const auto &light : deferred_lights) {
        const auto light_model =
                glm::translate(glm::mat4{1.0f}, light.position) * glm::scale(glm::mat4{1.0}, glm::vec3{0.1f});
        simple_program_->set_uniform("color", glm::vec4{light.color, 1.0f});
        simple_program_->set_uniform("transform", projection * view * light_model);
        cube_mesh_->draw(*simple_program_);
    }
}

void SSAO::draw_scene(const glm::mat4 &view, const glm::mat4 &projection, const Program &program) {
    static const Object cubes[] = {
            {{0.0f, -0.5f, 0.0f}, {40.0f, 1.0f, 40.0f}, {1.0f, 0.0f, 0.0f}, 0.0f, cube_mesh_, floor_material_, false},
            {{-1.0f, 0.75f, -4.0f}, {1.5f, 1.5f, 1.5f}, {0.0f, 1.0f, 0.0f}, 30.0f, cube_mesh_, cube_material1_, false},
            {{0.0f, 0.75f, 2.0f}, {1.5f, 1.5f, 1.5f}, {0.0f, 1.0f, 0.0f}, 20.0f, cube_mesh_, cube_material2_, false},
            {{3.0f, 1.75f, -2.0f}, {1.5f, 1.5f, 1.5f}, {0.0f, 1.0f, 0.0f}, 50.0f, cube_mesh_, cube_material2_, false},
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
    backpack_model_->draw(program);
}

void SSAO::draw_ui() {
    // ImGui Components.
    if (ImGui::Begin("UI")) {
        if (ImGui::CollapsingHeader("Camera", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::DragFloat3("Position", glm::value_ptr(camera_pos_), 0.1f);
            ImGui::DragFloat("Yaw", &camera_yaw_, 0.5f);
            ImGui::DragFloat("Pitch", &camera_pitch_, 0.5f, -89.0f, 89.0f);
        }
        ImGui::Separator();
        if (ImGui::CollapsingHeader("Lighting", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::Checkbox("Use SSAO", &use_ssao);
            ImGui::DragFloat("SSAO radius", &ssao_radius, 0.01f, 0.0f, 5.0f);
            ImGui::DragFloat("SSAO power", &ssao_power, 0.01f, 0.0f, 5.0f);
        }
        ImGui::Separator();
        if (ImGui::Button("Reset")) {
            camera_pos_ = CAMERA_POS;
            camera_yaw_ = CAMERA_YAW;
            camera_pitch_ = CAMERA_PITCH;
        }
    }
    ImGui::End();

    // G-buffer
    if (ImGui::Begin("G-buffer")) {
        const char *buffer_names[] = {"Position", "Normal", "Albedo/Specular"};
        static int buffer_select = 0;
        ImGui::Combo("buffer", &buffer_select, buffer_names, 3);
        float width = ImGui::GetContentRegionAvail().x;
        float height = width / aspect_ratio_;
        auto attachment = geo_framebuffer_->get_color_attachment(buffer_select);
        ImGui::Image(static_cast<ImTextureID>(attachment->get()), ImVec2{width, height}, ImVec2{0, 1}, ImVec2{1, 0});
    }
    ImGui::End();

    // SSAO buffer
    if (ImGui::Begin("SSAO")) {
        const char *buffer_names[] = {"Original", "Blurred"};
        static int buffer_select = 0;
        ImGui::Combo("Buffer", &buffer_select, buffer_names, 2);
        float width = ImGui::GetContentRegionAvail().x;
        float height = width / aspect_ratio_;
        auto attachment = (buffer_select == 0 ? ssao_framebuffer_ : blur_framebuffer_)->get_color_attachment();
        ImGui::Image(static_cast<ImTextureID>(attachment->get()), ImVec2{width, height}, ImVec2{0, 1}, ImVec2{1, 0});
    }
    ImGui::End();
}

void SSAO::reshape(const int width, const int height) {
    width_ = width;
    height_ = height;
    aspect_ratio_ = static_cast<float>(width) / static_cast<float>(height);
    geo_framebuffer_ = FrameBuffer::create({
            Texture::create(width, height, GL_RGBA16F, GL_FLOAT),
            Texture::create(width, height, GL_RGBA16F, GL_FLOAT),
            Texture::create(width, height, GL_RGBA),
    });
    ssao_framebuffer_ = FrameBuffer::create({Texture::create(width, height, GL_RED, GL_FLOAT)});
    blur_framebuffer_ = FrameBuffer::create({Texture::create(width, height, GL_RED, GL_FLOAT)});
}
