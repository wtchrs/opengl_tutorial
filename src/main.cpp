#include <spdlog/spdlog.h>
// glad/glad.h must be included before including GLFW/glfw3.h.
#include <glad/glad.h>
#include <glfw/glfw3.h>
#include "glex/context.h"

void onFrameBufferSizeChanged(GLFWwindow *window, int width, int height);
void onKeyEvent(GLFWwindow *window, int key, int scancode, int action, int mods);

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

    // `Context::create()` will load shaders, compile shaders, and link a pipeline program.
    auto context = Context::create();
    if (!context) {
        SPDLOG_ERROR("Failed to create context object");
        glfwTerminate();
        return -1;
    }

    // Register event handlers.
    onFrameBufferSizeChanged(window, WINDOW_WIDTH, WINDOW_HEIGHT);
    glfwSetFramebufferSizeCallback(window, onFrameBufferSizeChanged);
    glfwSetKeyCallback(window, onKeyEvent);

    // Enable vsync
    glfwSwapInterval(1);

    SPDLOG_INFO("Start main loop");
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        // glfwWaitEvents();
        context->render();
        glfwSwapBuffers(window);
    }

    context.reset();

    glfwTerminate();
    return 0;
}

void onFrameBufferSizeChanged(GLFWwindow *window, int width, int height) {
    SPDLOG_INFO("Frame buffer size changed: ({}x{})", width, height);
    // Set position and size of OpenGL viewport.
    glViewport(0, 0, width, height);
}

void onKeyEvent(GLFWwindow *window, int key, int scancode, int action, int mods) {
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
