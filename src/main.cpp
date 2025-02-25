#include <imgui.h>
#include <spdlog/spdlog.h>
// glad/glad.h must be included before including GLFW/glfw3.h.
#include <glad/glad.h>
#include <glfw/glfw3.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include "glex/context.h"

void onFrameBufferSizeChanged(GLFWwindow *window, int width, int height);
void onKeyEvent(GLFWwindow *window, int key, int scancode, int action, int mods);
void onCursorPos(GLFWwindow *window, double x, double y);
void onMouseButton(GLFWwindow *window, int button, int action, int mods);
void onCharEvent(GLFWwindow *window, unsigned int ch);
void onScroll(GLFWwindow *window, double xoffset, double yoffset);

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
    auto window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_NAME, nullptr, nullptr);
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

    auto glVersion = glGetString(GL_VERSION);
    SPDLOG_INFO("OpenGL context version: {}", reinterpret_cast<const char *>(glVersion));

    // Initialize ImGui.
    auto imgui_context = ImGui::CreateContext();
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
    onFrameBufferSizeChanged(window, WINDOW_WIDTH, WINDOW_HEIGHT);
    glfwSetFramebufferSizeCallback(window, onFrameBufferSizeChanged);
    glfwSetKeyCallback(window, onKeyEvent);
    glfwSetCursorPosCallback(window, onCursorPos);
    glfwSetMouseButtonCallback(window, onMouseButton);
    glfwSetCharCallback(window, onCharEvent);
    glfwSetScrollCallback(window, onScroll);

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

void onFrameBufferSizeChanged(GLFWwindow *window, int width, int height) {
    SPDLOG_INFO("Frame buffer size changed: ({}x{})", width, height);
    // Call Context::reshape
    const auto context = static_cast<Context *>(glfwGetWindowUserPointer(window));
    context->reshape(width, height);
    // Set position and size of OpenGL viewport.
    glViewport(0, 0, width, height);
}

void onKeyEvent(GLFWwindow *window, int key, int scancode, int action, int mods) {
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

void onCursorPos(GLFWwindow *window, const double x, const double y) {
    SPDLOG_TRACE("Mouse cursor moved: ({}, {})", x, y);
    const auto context = static_cast<Context *>(glfwGetWindowUserPointer(window));
    context->mouse_move(x, y);
}

void onMouseButton(GLFWwindow *window, const int button, const int action, int mods) {
    ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods); // ImGui mouse handler
    double x, y;
    const auto context = static_cast<Context *>(glfwGetWindowUserPointer(window));
    glfwGetCursorPos(window, &x, &y);
    SPDLOG_INFO("Mouse clicked: button={}, action={}, modifiers={}, pos=({}, {})", button, action, mods, x, y);
    context->mouse_button(button, action, x, y);
}

void onCharEvent(GLFWwindow *window, unsigned int ch) {
    ImGui_ImplGlfw_CharCallback(window, ch);
}

void onScroll(GLFWwindow *window, double xoffset, double yoffset) {
    ImGui_ImplGlfw_ScrollCallback(window, xoffset, yoffset);
}
