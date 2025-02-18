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
    // Declare vertices and indices for drawing a Rectangle.
    float vertices[] = {
            0.5f,  0.5f,  0.0f, //
            0.5f,  -0.5f, 0.0f, //
            -0.5f, -0.5f, 0.0f, //
            -0.5f, 0.5f,  0.0f, //
    };
    uint32_t indices[] = {
            0, 1, 3, // first triangle
            1, 2, 3, // second triangle
    };

    // Generate VAO, Vertex Array Object.
    // VAO must be generated before VBO generated.
    glGenVertexArrays(1, &vertex_array_object_);
    glBindVertexArray(vertex_array_object_);

    // Generate VBO, Vertex Buffer Object.
    // GL_ARRAY_BUFFER means VBO.
    // usage in `glBufferData` can be "GL_(STATIC|DYNAMIC|STREAM)_(DRAW|COPY|READ)"
    // GL_STATIC_DRAW means that this vertices will not be changed.
    vertex_buffer_ = Buffer::create_with_data(GL_ARRAY_BUFFER, GL_STATIC_DRAW, vertices, sizeof(vertices));

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, 0);

    // Generate EBO, Element Buffer Object.
    // GL_ELEMENT_ARRAY_BUFFER means EBO.
    /*
    glGenBuffers(1, &index_buffer_);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer_);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    */
    index_buffer_ = Buffer::create_with_data(GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW, indices, sizeof(indices));

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
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
}
