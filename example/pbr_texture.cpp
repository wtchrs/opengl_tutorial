#include <cstddef>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/geometric.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/trigonometric.hpp>
#include <imgui.h>
#include <memory>
#include <spdlog/spdlog.h>
#include "glex/common.h"
#include "glex/context.h"
#include "glex/image.h"
#include "glex/mesh.h"

namespace {
    struct Object {
        glm::vec3 pos;
        glm::vec3 scale;
        glm::vec3 rotDir;
        float rotAngle;
        std::shared_ptr<Mesh> mesh;
    };

    struct Material {
        std::unique_ptr<Texture> albedo;
        std::unique_ptr<Texture> normal;
        std::unique_ptr<Texture> metallic;
        std::unique_ptr<Texture> roughness;
        float ao;
    };

    struct ContextInternal {
        std::unique_ptr<Program> simple_program_, pbr_program_;
        std::shared_ptr<Mesh> cube_mesh_, plain_mesh_, sphere_mesh_;
        Material material_;
    };

    auto ctx = std::make_unique<ContextInternal>();

    struct Light {
        glm::vec3 position;
        glm::vec3 color;
    };
    std::vector<Light> lights;

    ///@{
    /// Default parameters
    constexpr float CAMERA_PITCH{0.0f};
    constexpr float CAMERA_YAW{0.0f};

    constexpr glm::vec3 CAMERA_POS{0.0f, 0.0f, 3.0f};
    constexpr glm::vec3 CAMERA_FRONT{0.0f, 0.0f, -1.0f};
    constexpr glm::vec3 CAMERA_UP{0.0f, 1.0f, 0.0f};
    ///@}

    ///@{
    /// Camera parameters
    float camera_pitch{CAMERA_PITCH};
    float camera_yaw{CAMERA_YAW};

    glm::vec3 camera_pos{CAMERA_POS};
    glm::vec3 camera_front{CAMERA_FRONT};
    glm::vec3 camera_up{CAMERA_UP};

    bool camera_rot_control{false};
    glm::vec2 prev_mouse_pos{0.0f};
    ///@}

    int width{WINDOW_WIDTH}, height{WINDOW_HEIGHT};
    float aspect_ratio{static_cast<float>(WINDOW_WIDTH) / static_cast<float>(WINDOW_HEIGHT)};
} // namespace

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
    ctx->sphere_mesh_ = Mesh::create_sphere();

    ctx->material_.albedo = Texture::create(*Image::load("./image/rusted_iron/rustediron2_basecolor.png", false));
    ctx->material_.normal = Texture::create(*Image::load("./image/rusted_iron/rustediron2_normal.png", false));
    ctx->material_.metallic = Texture::create(*Image::load("./image/rusted_iron/rustediron2_metallic.png", false));
    ctx->material_.roughness = Texture::create(*Image::load("./image/rusted_iron/rustediron2_roughness.png", false));

    // Load programs.
    ctx->simple_program_ = Program::create("./shader/simple.vs", "./shader/simple.fs");
    ctx->pbr_program_ = Program::create("./shader/pbr_texture.vs", "./shader/pbr_texture.fs");

    if (!ctx->simple_program_ || !ctx->pbr_program_) {
        SPDLOG_ERROR("Failed to initialize context");
        return false;
    }

    lights.emplace_back(glm::vec3{5.0f, 5.0f, 6.0f}, glm::vec3{40.0f, 40.0f, 40.0f});
    lights.emplace_back(glm::vec3{-4.0f, 5.0f, 7.0f}, glm::vec3{40.0f, 40.0f, 40.0f});
    lights.emplace_back(glm::vec3{-4.0f, -6.0f, 8.0f}, glm::vec3{40.0f, 40.0f, 40.0f});
    lights.emplace_back(glm::vec3{5.0f, -6.0f, 9.0f}, glm::vec3{40.0f, 40.0f, 40.0f});

    // Enable depth test and cull face.
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    return true;
}

void Context::render() {
    // Clear color buffer with `glClearColor` and depth buffer with 1.0.
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Dear ImGui UI
    draw_ui();

    // Calculate camera front direction.
    camera_front = glm::rotate(glm::mat4{1.0f}, glm::radians(camera_yaw), glm::vec3{0.0f, 1.0f, 0.0f}) *
                   glm::rotate(glm::mat4{1.0f}, glm::radians(camera_pitch), glm::vec3{1.0f, 0.0f, 0.0f}) *
                   glm::vec4{0.0f, 0.0f, -1.0f, 0.0f};

    // Projection and view matrix
    // When the `Near` value is too small, inaccurate depth test, known as "z-fighting", arise on far objects,
    // due to the z-value distortion introduced by the projection transform.
    const auto projection = glm::perspective(glm::radians(45.0f), aspect_ratio, 0.01f, 150.0f);
    const auto view = glm::lookAt(camera_pos, camera_pos + camera_front, camera_up);

    const auto &program = *ctx->pbr_program_;
    program.use();
    for (size_t i = 0; i < lights.size(); ++i) {
        const auto pos_name = std::format("lights[{}].position", i);
        const auto color_name = std::format("lights[{}].color", i);
        program.set_uniform(pos_name, lights[i].position);
        program.set_uniform(color_name, lights[i].color);
    }
    program.set_uniform("viewPos", camera_pos);
    glActiveTexture(GL_TEXTURE0);
    ctx->material_.albedo->bind();
    glActiveTexture(GL_TEXTURE1);
    ctx->material_.normal->bind();
    glActiveTexture(GL_TEXTURE2);
    ctx->material_.metallic->bind();
    glActiveTexture(GL_TEXTURE3);
    ctx->material_.roughness->bind();
    program.set_uniform("material.albedo", 0);
    program.set_uniform("material.normal", 1);
    program.set_uniform("material.metallic", 2);
    program.set_uniform("material.roughness", 3);
    program.set_uniform("material.ao", ctx->material_.ao);
    draw_scene(view, projection, program);
}

void Context::draw_scene(const glm::mat4 &view, const glm::mat4 &projection, const Program &program) {
    program.use();
    const int sphere_count = 7;
    const float offset = 1.2f;
    for (size_t j = 0; j < sphere_count; ++j) {
        const float y = (static_cast<float>(j) - static_cast<float>(sphere_count - 1) * 0.5f) * offset;
        for (size_t i = 0; i < sphere_count; ++i) {
            const float x = (static_cast<float>(i) - static_cast<float>(sphere_count - 1) * 0.5f) * offset;
            const auto model_transform = glm::translate(glm::mat4{1.0f}, glm::vec3{x, y, 0.0f});
            const auto transform = projection * view * model_transform;
            program.set_uniform("modelTransform", model_transform);
            program.set_uniform("transform", transform);
            ctx->sphere_mesh_->draw(program);
        }
    }
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
        if (ImGui::CollapsingHeader("Lights", ImGuiTreeNodeFlags_DefaultOpen)) {
            static int idx = 0;
            ImGui::DragInt("Light index", &idx, 1, 0, 3);
            ImGui::DragFloat3("Light position", glm::value_ptr(lights[idx].position), 0.01f);
            ImGui::DragFloat3("Light color", glm::value_ptr(lights[idx].color), 0.1f);
        }
        ImGui::Separator();
        if (ImGui::CollapsingHeader("Material", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::SliderFloat("Material AO", &ctx->material_.ao, 0.0f, 1.0f);
        }
        ImGui::Separator();
        if (ImGui::Button("Reset")) {
            camera_pos = CAMERA_POS;
            camera_yaw = CAMERA_YAW;
            camera_pitch = CAMERA_PITCH;
        }
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
    ::width = width;
    ::height = height;
    aspect_ratio = static_cast<float>(width) / static_cast<float>(height);
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
