#include "glex/context.h"

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
