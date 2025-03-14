#include <imgui.h>
#include <spdlog/spdlog.h>
// glad/glad.h must be included before including GLFW/glfw3.h.
#include <glad/glad.h>
#include <glfw/glfw3.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include "glex/context.h"

void on_frame_buffer_size_changed(GLFWwindow *window, int width, int height);
void on_key_event(GLFWwindow *window, int key, int scancode, int action, int mods);
void on_cursor_pos(GLFWwindow *window, double x, double y);
void on_mouse_button(GLFWwindow *window, int button, int action, int mods);
void on_char_event(GLFWwindow *window, unsigned int ch);
void on_scroll(GLFWwindow *window, double xoffset, double yoffset);

int main() {
    SPDLOG_INFO("Start main");

    SPDLOG_INFO("Initialize glfw");
    if (!glfwInit()) {
        const char *description = nullptr;
        glfwGetError(&description);
        SPDLOG_ERROR("Failed to initialize glfw: {}", description);
        return -1;
    }

    // OpenGL 3.3 is guaranteed to support forward compatibility.
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    SPDLOG_INFO("Create glfw window");
    const auto window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_NAME, nullptr, nullptr);
    if (!window) {
        SPDLOG_ERROR("Failed to create glfw window");
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Load OpenGl functions using GLAD.
    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
        SPDLOG_ERROR("Failed to initialize glad");
        glfwTerminate();
        return -1;
    }

    // OpenGL functions can be called above here.

    const auto glVersion = glGetString(GL_VERSION);
    SPDLOG_INFO("OpenGL context version: {}", reinterpret_cast<const char *>(glVersion));

    // Initialize ImGui.
    const auto imgui_context = ImGui::CreateContext();
    ImGui::SetCurrentContext(imgui_context);
    ImGui_ImplGlfw_InitForOpenGL(window, false);
    ImGui_ImplOpenGL3_Init();
    ImGui_ImplOpenGL3_CreateFontsTexture();
    ImGui_ImplOpenGL3_CreateDeviceObjects();
    SPDLOG_INFO("ImGui context loaded");

    // `Context::create()` will load shaders, compile shaders, and link a pipeline program.
    auto context = Context::create();
    if (!context) {
        SPDLOG_ERROR("Failed to create context object");
        glfwTerminate();
        return -1;
    }

    // Set user pointer
    glfwSetWindowUserPointer(window, context.get());

    // Register event handlers.
    on_frame_buffer_size_changed(window, WINDOW_WIDTH, WINDOW_HEIGHT);
    glfwSetFramebufferSizeCallback(window, on_frame_buffer_size_changed);
    glfwSetKeyCallback(window, on_key_event);
    glfwSetCursorPosCallback(window, on_cursor_pos);
    glfwSetMouseButtonCallback(window, on_mouse_button);
    glfwSetCharCallback(window, on_char_event);
    glfwSetScrollCallback(window, on_scroll);

    // Enable vsync
    glfwSwapInterval(1);

    SPDLOG_INFO("Start main loop");
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        // Notify starting new frame rendering to ImGui.
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        context->process_input(window);
        context->render();

        ImGui::Render(); // Gether draw data.
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData()); // Render draw data.

        glfwSwapBuffers(window);
    }

    context.reset();

    // Release ImGui resources.
    ImGui_ImplOpenGL3_DestroyFontsTexture();
    ImGui_ImplOpenGL3_DestroyDeviceObjects();
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext(imgui_context);

    glfwTerminate();
    return 0;
}

void on_frame_buffer_size_changed(GLFWwindow *window, const int width, const int height) {
    SPDLOG_INFO("Frame buffer size changed: ({}x{})", width, height);
    // Call Context::reshape
    const auto context = static_cast<Context *>(glfwGetWindowUserPointer(window));
    context->reshape(width, height);
    // Set position and size of OpenGL viewport.
    glViewport(0, 0, width, height);
}

void on_key_event(GLFWwindow *window, const int key, const int scancode, const int action, const int mods) {
    ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);
    auto actionStr = action == GLFW_PRESS     ? "Pressed"
                     : action == GLFW_RELEASE ? "Released"
                     : action == GLFW_REPEAT  ? "Repeated"
                                              : "unknown";
    auto ctrl = mods & GLFW_MOD_CONTROL ? "C" : "";
    auto shift = mods & GLFW_MOD_SHIFT ? "S" : "";
    auto alt = mods & GLFW_MOD_ALT ? "A" : "";
    SPDLOG_INFO(
            "Key pressed: (key: {}, scancode: {}, action: {}, mods: {}{}{})", key, scancode, actionStr, ctrl, shift, alt
    );
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
}

void on_cursor_pos(GLFWwindow *window, const double x, const double y) {
    SPDLOG_TRACE("Mouse cursor moved: ({}, {})", x, y);
    const auto context = static_cast<Context *>(glfwGetWindowUserPointer(window));
    context->mouse_move(x, y);
}

void on_mouse_button(GLFWwindow *window, const int button, const int action, const int mods) {
    ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods); // ImGui mouse handler
    double x, y;
    const auto context = static_cast<Context *>(glfwGetWindowUserPointer(window));
    glfwGetCursorPos(window, &x, &y);
    SPDLOG_INFO("Mouse clicked: button={}, action={}, modifiers={}, pos=({}, {})", button, action, mods, x, y);
    context->mouse_button(button, action, x, y);
}

void on_char_event(GLFWwindow *window, const unsigned int ch) {
    ImGui_ImplGlfw_CharCallback(window, ch);
}

void on_scroll(GLFWwindow *window, const double xoffset, const double yoffset) {
    ImGui_ImplGlfw_ScrollCallback(window, xoffset, yoffset);
}
