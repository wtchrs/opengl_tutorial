#include "glex/context.h"
#include <GLFW/glfw3.h>
#include <glm/geometric.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/trigonometric.hpp>
#include <imgui.h>
#include <memory>
#include <spdlog/spdlog.h>
#include <vector>
#include "glex/common.h"
#include "glex/image.h"
#include "glex/mesh.h"
#include "glex/texture.h"

std::unique_ptr<Context> Context::create() {
    auto context = std::unique_ptr<Context>{new Context{}};
    if (!context->init()) {
        SPDLOG_ERROR("Failed to create context");
        return nullptr;
    }
    SPDLOG_INFO("Context has been created");
    return std::move(context);
}

bool Context::init() {
    // Create cube mesh.
    cube_mesh_ = Mesh::create_cube();

    // Load programs.
    simple_program_ = Program::create("./shader/simple.vs", "./shader/simple.fs");
    program_ = Program::create("./shader/lighting.vs", "./shader/lighting.fs");

    if (!simple_program_ || !program_) {
        SPDLOG_ERROR("Failed to initialize context");
        return false;
    }

    program_->use();

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
    plain_material_ = std::make_shared<Material>(plain_diffuse, gray_texture, 128.0f);

    // Create cube1 material.
    std::shared_ptr cube_diffuse1 = Texture::create(*Image::load("./image/container.jpg"));
    cube_material1_ = std::make_shared<Material>(cube_diffuse1, dark_gray_texture, 16.0f);

    // Create cube2 material.
    std::shared_ptr cube_diffuse2 = Texture::create(*Image::load("./image/container2.png"));
    std::shared_ptr cube_specular2 = Texture::create(*Image::load("./image/container2_specular.png"));
    cube_material2_ = std::make_shared<Material>(cube_diffuse2, cube_specular2, 64.0f);

    // Enable depth test.
    glEnable(GL_DEPTH_TEST);

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
    // ImGui Components.
    if (ImGui::Begin("UI")) {
        if (ImGui::ColorEdit4("Clear color", glm::value_ptr(clear_color_))) {
            glClearColor(clear_color_.x, clear_color_.y, clear_color_.z, clear_color_.w);
        }
        ImGui::DragFloat("Scale", &scale_, 0.1f, 0.0f, 1.0f);
        ImGui::Separator();
        if (ImGui::CollapsingHeader("Camera", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::DragFloat3("Position", glm::value_ptr(camera_pos_), 0.1f);
            ImGui::DragFloat("Yaw", &camera_yaw_, 0.5f);
            ImGui::DragFloat("Pitch", &camera_pitch_, 0.5f, -89.0f, 89.0f);
        }
        ImGui::Separator();
        if (ImGui::CollapsingHeader("Lighting", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::Text("Light");
            ImGui::Checkbox("Flash light mode", &flash_light_mode_);
            if (!flash_light_mode_) {
                ImGui::DragFloat3("l.position", glm::value_ptr(light_.position), 0.01f);
                ImGui::DragFloat3("l.direction", glm::value_ptr(light_.direction), 0.01f);
            }
            ImGui::DragFloat("l.distance(attenuation)", &light_.distance, 0.5f, 0.0f, 3000.0f);
            ImGui::DragFloat2("l.cutoff(degree)", glm::value_ptr(light_.cutoff), 0.5f, 0.0f, 180.0f);
            ImGui::ColorEdit3("l.ambient", glm::value_ptr(light_.ambient));
            ImGui::ColorEdit3("l.diffuse", glm::value_ptr(light_.diffuse));
            ImGui::ColorEdit3("l.specular", glm::value_ptr(light_.specular));
            ImGui::Separator();
            ImGui::Text("Material");
        }
        ImGui::Separator();
        ImGui::Checkbox("Animation", &animation_);
        ImGui::Separator();
        if (ImGui::Button("Reset")) {
            camera_pos_ = CAMERA_POS;
            camera_yaw_ = CAMERA_YAW;
            camera_pitch_ = CAMERA_PITCH;
            light_ = LIGHT;
        }
    }
    ImGui::End();

    // Clear color buffer with `glClearColor` and depth buffer with 1.0.
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Calculate camera front direction.
    camera_front_ = glm::rotate(glm::mat4{1.0f}, glm::radians(camera_yaw_), glm::vec3{0.0f, 1.0f, 0.0f}) *
                    glm::rotate(glm::mat4{1.0f}, glm::radians(camera_pitch_), glm::vec3{1.0f, 0.0f, 0.0f}) *
                    glm::vec4{0.0f, 0.0f, -1.0f, 0.0f};

    // Projection and view matrix

    // When the `Near` value is too small, inaccurate depth test, known as "z-fighting", arise on far objects,
    // due to the z-value distortion introduced by the projection transform.
    /* const auto projection = glm::perspective(glm::radians(45.0f), aspect_ratio_, 0.01f, 30.0f); */
    const auto projection = glm::perspective(glm::radians(45.0f), aspect_ratio_, 0.1f, 100.0f);

    const auto view = glm::lookAt(camera_pos_, camera_pos_ + camera_front_, camera_up_);

    static constexpr auto obj_rotate_direction = glm::vec3{1.0f, 0.5f, 0.0f};

    auto light_pos = light_.position;
    auto light_dir = light_.direction;
    if (flash_light_mode_) {
        light_pos = camera_pos_;
        light_dir = camera_front_;
    } else {
        // Draw one cube with simple_program_ for indicating the light position.
        const auto light_model =
                glm::translate(glm::mat4{1.0f}, light_pos) * glm::scale(glm::mat4{1.0}, glm::vec3{0.1f});
        simple_program_->use();
        simple_program_->set_uniform("color", glm::vec4{light_.ambient + light_.diffuse, 0.0f});
        simple_program_->set_uniform("transform", projection * view * light_model);
        cube_mesh_->draw(*simple_program_);
    }

    // Set lighting.
    program_->use();
    program_->set_uniform("viewPos", camera_pos_);
    program_->set_uniform("light.position", light_pos);
    program_->set_uniform("light.direction", light_dir);
    program_->set_uniform("light.attenuation", get_attenuation_coefficient(light_.distance));
    program_->set_uniform(
            "light.cutoff", glm::cos(glm::radians(glm::vec2{light_.cutoff.x, light_.cutoff.x + light_.cutoff.y}))
    );
    program_->set_uniform("light.ambient", light_.ambient);
    program_->set_uniform("light.diffuse", light_.diffuse);
    program_->set_uniform("light.specular", light_.specular);

    struct CubeObject {
        glm::vec3 pos;
        glm::vec3 scale;
        glm::vec3 rotDir;
        float rotAngle;
        std::shared_ptr<Material> material;
    };

    CubeObject cubes[] = {
            {{0.0f, -0.5f, 0.0f}, {10.0f, 1.0f, 10.0f}, {1.0f, 0.0f, 0.0f}, 0.0f, plain_material_},
            {{-1.0f, 0.75f, -4.0f}, {1.5f, 1.5f, 1.5f}, {0.0f, 1.0f, 0.0f}, 30.0f, cube_material1_},
            {{0.0f, -0.749f, 2.0f}, {1.5f, 1.5f, 1.5f}, {0.0f, 1.0f, 0.0f}, 20.0f, cube_material2_},
    };

    for (const auto &[pos, scale, rotDir, rotAngle, material] : cubes) {
        auto model_transform = glm::translate(glm::mat4{1.0f}, pos) * glm::rotate(glm::mat4{1.0f}, rotAngle, rotDir) *
                               glm::scale(glm::mat4{1.0f}, scale);
        auto transform = projection * view * model_transform;
        program_->set_uniform("transform", transform);
        program_->set_uniform("modelTransform", model_transform);
        material->set_to_program(*program_);
        cube_mesh_->draw(*program_);
    }
}

void Context::process_input(GLFWwindow *window) {
    constexpr auto camera_speed = 0.05f;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        camera_pos_ += camera_speed * camera_front_;
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        camera_pos_ -= camera_speed * camera_front_;
    }

    auto camera_right = -glm::normalize(glm::cross(camera_up_, camera_front_));
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        camera_pos_ += camera_speed * camera_right;
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        camera_pos_ -= camera_speed * camera_right;
    }

    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
        camera_pos_ += camera_speed * camera_up_;
    }
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
        camera_pos_ -= camera_speed * camera_up_;
    }
}

void Context::reshape(const int width, const int height) {
    aspect_ratio_ = static_cast<float>(width) / static_cast<float>(height);
}

void Context::mouse_move(const double x, const double y) {
    if (!camera_rot_control_)
        return;

    const glm::vec2 cur{static_cast<float>(x), static_cast<float>(y)};
    const auto delta_pos = cur - prev_mouse_pos_;

    constexpr float CAMERA_ROT_SPEED = 0.4f;
    camera_yaw_ -= delta_pos.x * CAMERA_ROT_SPEED;
    camera_pitch_ -= delta_pos.y * CAMERA_ROT_SPEED;

    if (camera_yaw_ < 0.0f) {
        camera_yaw_ += 360.0f;
    }
    if (camera_yaw_ > 360.0f) {
        camera_yaw_ -= 360.0f;
    }
    if (camera_pitch_ > 89.0f) {
        camera_pitch_ = 89.0f;
    }
    if (camera_pitch_ < -89.0f) {
        camera_pitch_ = -89.0f;
    }

    prev_mouse_pos_ = cur;
}

void Context::mouse_button(const int button, const int action, const double x, const double y) {
    if (button == GLFW_MOUSE_BUTTON_RIGHT) {
        if (action == GLFW_PRESS) {
            prev_mouse_pos_ = glm::vec2{x, y};
            camera_rot_control_ = true;
        } else {
            camera_rot_control_ = false;
        }
    }
}
