#include "glex/context.h"
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
    // Declare vertices for drawing a triangle.
    float vertices[] = {
            -0.5f, -0.5f, 0.0f, //
            0.5f,  -0.5f, 0.0f, //
            0.0f,  0.5f,  0.0f, //
    };

    // Generate vertex array object (VAO).
    // VAO must be generated before VBO generated.
    glGenVertexArrays(1, &vertex_array_object_);
    glBindVertexArray(vertex_array_object_);

    // Generate vertex buffer object (VBO).
    glGenBuffers(1, &vertex_buffer_);
    // GL_ARRAY_BUFFER means VBO.
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_);
    // usage can be "GL_(STATIC|DYNAMIC|STREAM)_(DRAW|COPY|READ)"
    // GL_STATIC_DRAW means that this vertices will not be changed.
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glad_glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, 0);

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

    return true;
}

void Context::render() {
    // Clear with color that has been defined with `glClearColor`.
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(program_->get());
    glDrawArrays(GL_TRIANGLES, 0, 3);
}
