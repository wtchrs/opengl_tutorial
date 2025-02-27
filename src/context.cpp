#include "glex/context.h"
#include <GLFW/glfw3.h>
#include <cstddef>
#include <cstdint>
#include <glm/geometric.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/trigonometric.hpp>
#include <imgui.h>
#include <memory>
#include <spdlog/spdlog.h>
#include <vector>
#include "glex/common.h"
#include "glex/image.h"
#include "glex/texture.h"
#include "glex/vertex_layout.h"

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
    // Declare vertices and indices for drawing a cube.
    constexpr float VERTICES[] = {
            // position.xyz, normal.xyz, texCoord.uv
            -0.5f, -0.5f, -0.5f, 0.0f,  -1.0f, 0.0f,  0.0f, 1.0f, // bottom
            -0.5f, -0.5f, 0.5f,  0.0f,  -1.0f, 0.0f,  0.0f, 0.0f, //
            0.5f,  -0.5f, 0.5f,  0.0f,  -1.0f, 0.0f,  1.0f, 0.0f, //
            0.5f,  -0.5f, -0.5f, 0.0f,  -1.0f, 0.0f,  1.0f, 1.0f, //

            -0.5f, 0.5f,  -0.5f, 0.0f,  0.0f,  -1.0f, 0.0f, 1.0f, // back
            -0.5f, -0.5f, -0.5f, 0.0f,  0.0f,  -1.0f, 0.0f, 0.0f, //
            0.5f,  -0.5f, -0.5f, 0.0f,  0.0f,  -1.0f, 1.0f, 0.0f, //
            0.5f,  0.5f,  -0.5f, 0.0f,  0.0f,  -1.0f, 1.0f, 1.0f, //

            0.5f,  0.5f,  -0.5f, 1.0f,  0.0f,  0.0f,  0.0f, 1.0f, // right
            0.5f,  -0.5f, -0.5f, 1.0f,  0.0f,  0.0f,  0.0f, 0.0f, //
            0.5f,  -0.5f, 0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, //
            0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, //

            0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 1.0f, // front
            0.5f,  -0.5f, 0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f, //
            -0.5f, -0.5f, 0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 0.0f, //
            -0.5f, 0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 1.0f, //

            -0.5f, 0.5f,  0.5f,  -1.0f, 0.0f,  0.0f,  0.0f, 1.0f, // left
            -0.5f, -0.5f, 0.5f,  -1.0f, 0.0f,  0.0f,  0.0f, 0.0f, //
            -0.5f, -0.5f, -0.5f, -1.0f, 0.0f,  0.0f,  1.0f, 0.0f, //
            -0.5f, 0.5f,  -0.5f, -1.0f, 0.0f,  0.0f,  1.0f, 1.0f, //

            0.5f,  0.5f,  -0.5f, 0.0f,  1.0f,  0.0f,  0.0f, 1.0f, // top
            0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f, //
            -0.5f, 0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f, //
            -0.5f, 0.5f,  -0.5f, 0.0f,  1.0f,  0.0f,  1.0f, 1.0f, //
    };
    constexpr uint32_t INDICES[] = {
            0,  1,  2,  0,  2,  3, //
            4,  5,  6,  4,  6,  7, //
            8,  9,  10, 8,  10, 11, //
            12, 13, 14, 12, 14, 15, //
            16, 17, 18, 16, 18, 19, //
            20, 21, 22, 20, 22, 23, //
    };
    constexpr size_t STRIDE = 8; // number of floats in one vertex

    // Generate VAO, Vertex Array Object.
    // VAO must be generated before VBO generated.
    vertex_layout_ = VertexLayout::create();

    // Generate VBO, Vertex Buffer Object.
    // GL_ARRAY_BUFFER means VBO.
    // usage in `glBufferData` can be "GL_(STATIC|DYNAMIC|STREAM)_(DRAW|COPY|READ)"
    // GL_STATIC_DRAW means that this vertices will not be changed.
    vertex_buffer_ = Buffer::create_with_data(GL_ARRAY_BUFFER, GL_STATIC_DRAW, VERTICES, sizeof(VERTICES));

    // Set and enable VAO attribute.
    vertex_layout_->set_attrib(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * STRIDE, 0);
    vertex_layout_->set_attrib(1, 3, GL_FLOAT, GL_FALSE, sizeof(float) * STRIDE, sizeof(float) * 3);
    vertex_layout_->set_attrib(2, 2, GL_FLOAT, GL_FALSE, sizeof(float) * STRIDE, sizeof(float) * 6);

    // Generate EBO, Element Buffer Object.
    // GL_ELEMENT_ARRAY_BUFFER means EBO.
    index_buffer_ = Buffer::create_with_data(GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW, INDICES, sizeof(INDICES));

    // Load and compile shaders.
    std::shared_ptr vertex_shader = Shader::create_from_file("./shader/lighting.vs", GL_VERTEX_SHADER);
    std::shared_ptr fragment_shader = Shader::create_from_file("./shader/lighting.fs", GL_FRAGMENT_SHADER);
    if (!vertex_shader || !fragment_shader) {
        SPDLOG_ERROR("Failed to initialize context");
        return false;
    }

    // Link program.
    program_ = Program::create({vertex_shader, fragment_shader});
    if (!program_) {
        SPDLOG_ERROR("Failed to initialize context");
        return false;
    }

    // Set clear color.
    glClearColor(0.0f, 0.1f, 0.2f, 0.0f);

    // Load an image for texture
    auto image1 = Image::load("./image/container.jpg");
    auto image2 = Image::load("./image/awesomeface.png");
    if (!image1 || !image2) {
        SPDLOG_ERROR("Failed to initialize context");
        return false;
    }

    // Generate a texture to use.
    // Use default linear filtering and clamp wrapping.
    texture1_ = Texture::create();
    texture1_->set_texture_image(0, *image1);
    texture2_ = Texture::create();
    texture2_->set_texture_image(0, *image2);

    // Bind textures to texture slots.
    program_->use();
    program_->set_texture("tex1", 0, *texture1_);
    program_->set_texture("tex2", 1, *texture2_);

    // Enable depth test.
    glEnable(GL_DEPTH_TEST);

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
        ImGui::Separator();
        if (ImGui::CollapsingHeader("Camera", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::DragFloat3("Position", glm::value_ptr(camera_pos_), 0.1f);
            ImGui::DragFloat("Yaw", &camera_yaw_, 0.5f);
            ImGui::DragFloat("Pitch", &camera_pitch_, 0.5f, -89.0f, 89.0f);
        }
        ImGui::Separator();
        if (ImGui::CollapsingHeader("Lighting", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::DragFloat3("Light position", glm::value_ptr(light_pos_), 0.01f);
            ImGui::ColorEdit3("Light color", glm::value_ptr(light_color_));
            ImGui::ColorEdit3("Object color", glm::value_ptr(object_color_));
            ImGui::SliderFloat("Ambient strength", &ambient_strength_, 0.0f, 1.0f);
            ImGui::DragFloat("Specular strength", &specular_strength_, 0.01, 0.0f, 1.0f);
            ImGui::DragFloat("Specular shininess", &specular_shininess_, 0.5f, 1.0f, 256.0f);
        }
        ImGui::Separator();
        ImGui::Checkbox("Animation", &animation_);
        ImGui::Separator();
        if (ImGui::Button("Reset")) {
            camera_pos_ = CAMERA_POS;
            camera_yaw_ = CAMERA_YAW;
            camera_pitch_ = CAMERA_PITCH;
            light_pos_ = LIGHT_POS;
            light_color_ = LIGHT_COLOR;
            object_color_ = OBJECT_COLOR;
            ambient_strength_ = AMBIENT_STRENGTH;
            specular_strength_ = SPECULAR_STRENGTH;
            specular_shininess_ = SPECULAR_SHININESS;
        }
    }
    ImGui::End();

    static const std::vector<glm::vec3> cube_positions = {
            glm::vec3{0.0f, 0.0f, 0.0f},     glm::vec3{2.0f, 5.0f, -15.0f}, glm::vec3{-1.5f, -2.2f, -2.5f},
            glm::vec3{-3.8f, -2.0f, -12.3f}, glm::vec3{2.4f, -0.4f, -3.5f}, glm::vec3{-1.7f, 3.0f, -7.5f},
            glm::vec3{1.3f, -2.0f, -2.5f},   glm::vec3{1.5f, 2.0f, -2.5f},  glm::vec3{1.5f, 0.2f, -1.5f},
            glm::vec3{-1.3f, 1.0f, -1.5f},
    };

    // Clear with color that has been defined with `glClearColor`.
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    static auto current_time = static_cast<float>(glfwGetTime());
    if (animation_) {
        current_time = static_cast<float>(glfwGetTime());
    }

    // # Transform matrix
    // - Model matrix: Local space -> World space
    // - View matrix: World space -> View space
    // - Projection matrix: View space -> Canonical space
    // - MVP matrix: Model-View-Projection matrix

    // Calculate camera front direction.
    camera_front_ = glm::rotate(glm::mat4{1.0f}, glm::radians(camera_yaw_), glm::vec3{0.0f, 1.0f, 0.0f}) *
                    glm::rotate(glm::mat4{1.0f}, glm::radians(camera_pitch_), glm::vec3{1.0f, 0.0f, 0.0f}) *
                    glm::vec4{0.0f, 0.0f, -1.0f, 0.0f};

    // Projection and view matrix
    auto projection = glm::perspective(glm::radians(45.0f), aspect_ratio_, 0.01f, 30.0f);
    auto view = glm::lookAt(camera_pos_, camera_pos_ + camera_front_, camera_up_);

    static constexpr auto obj_rotate_direction = glm::vec3{1.0f, 0.5f, 0.0f};

    program_->use();

    // Draw one cube for light position.
    auto light_model = glm::translate(glm::mat4{1.0f}, light_pos_) * glm::scale(glm::mat4{1.0}, glm::vec3{0.1f});
    program_->set_uniform("lightPos", light_pos_);
    program_->set_uniform("lightColor", glm::vec3{1.0f});
    program_->set_uniform("objectColor", glm::vec3{1.0f});
    program_->set_uniform("ambientStrength", 1.0f);
    program_->set_uniform("modelTransform", light_model);
    program_->set_uniform("transform", projection * view * light_model);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, nullptr);

    // Restore light settings.
    program_->set_uniform("viewPos", camera_pos_);
    program_->set_uniform("lightColor", light_color_);
    program_->set_uniform("objectColor", object_color_);
    program_->set_uniform("ambientStrength", ambient_strength_);
    program_->set_uniform("specularStrength", specular_strength_);
    program_->set_uniform("specularShininess", specular_shininess_);

    // Draw each cubes.
    for (size_t i = 0; i < cube_positions.size(); ++i) {
        const glm::vec3 &pos = cube_positions[i];
        auto model = glm::translate(glm::mat4{1.0f}, pos);
        model = glm::rotate(
                model, glm::radians(current_time * 120.0f + 20.0f * static_cast<float>(i)), obj_rotate_direction
        );

        auto transform = projection * view * model;
        program_->set_uniform("modelTransform", model);
        program_->set_uniform("transform", transform);

        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, nullptr);
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
