#include "glex/context.h"
#include <cstdint>
#include <memory>
#include <spdlog/spdlog.h>
#include "glex/common.h"

std::unique_ptr<Context> Context::create() {
    auto context = std::unique_ptr<Context>{new Context{}};
    if (!context->init()) {
        return nullptr;
    }
    return std::move(context);
}

bool Context::init() {
    // Load and compile shaders.
    std::shared_ptr<Shader> vertex_shader = Shader::create_from_file("./shader/simple.vs", GL_VERTEX_SHADER);
    std::shared_ptr<Shader> fragment_shader = Shader::create_from_file("./shader/simple.fs", GL_FRAGMENT_SHADER);
    if (!vertex_shader || !fragment_shader) {
        return false;
    }
    SPDLOG_INFO("Vertex shader id: {}", vertex_shader->get());
    SPDLOG_INFO("Fragment shader id: {}", fragment_shader->get());

    // Link program.
    program_ = Program::create({vertex_shader, fragment_shader});
    if (!program_) {
        return false;
    }
    SPDLOG_INFO("Program id: {}", program_->get());

    // Set clear color.
    glClearColor(0.0f, 0.1f, 0.2f, 0.0f);

    uint32_t vao = 0;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    return true;
}

void Context::render() {
    // Clear with color that has been defined with `glClearColor`.
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(program_->get());
    glDrawArrays(GL_POINTS, 0, 1);
}
