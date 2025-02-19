#include "glex/context.h"
#include <cstdint>
#include <memory>
#include <spdlog/spdlog.h>
#include "glex/common.h"
#include "glex/vertex_layout.h"

std::unique_ptr<Context> Context::create() {
    auto context = std::unique_ptr<Context>{new Context{}};
    if (!context->init()) {
        SPDLOG_ERROR("Failed to create context.");
        return nullptr;
    }
    SPDLOG_INFO("Context has been created.");
    return std::move(context);
}

bool Context::init() {
    // Declare vertices and indices for drawing a Rectangle.
    float vertices[] = {
            0.5f,  0.5f,  0.0f, 1.0f, 0.0f, 0.0f, //
            0.5f,  -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, //
            -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, //
            -0.5f, 0.5f,  0.0f, 1.0f, 1.0f, 0.0f, //
    };
    uint32_t indices[] = {
            0, 1, 3, // first triangle
            1, 2, 3, // second triangle
    };

    // Generate VAO, Vertex Array Object.
    // VAO must be generated before VBO generated.
    vertex_layout_ = VertexLayout::create();

    // Generate VBO, Vertex Buffer Object.
    // GL_ARRAY_BUFFER means VBO.
    // usage in `glBufferData` can be "GL_(STATIC|DYNAMIC|STREAM)_(DRAW|COPY|READ)"
    // GL_STATIC_DRAW means that this vertices will not be changed.
    vertex_buffer_ = Buffer::create_with_data(GL_ARRAY_BUFFER, GL_STATIC_DRAW, vertices, sizeof(vertices));

    // Set and enable VAO attribute.
    vertex_layout_->set_attrib(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 6, 0);
    vertex_layout_->set_attrib(1, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 6, sizeof(float) * 3);

    // Generate EBO, Element Buffer Object.
    // GL_ELEMENT_ARRAY_BUFFER means EBO.
    index_buffer_ = Buffer::create_with_data(GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW, indices, sizeof(indices));

    // Load and compile shaders.
    std::shared_ptr vertex_shader = Shader::create_from_file("./shader/per_vertex_color.vs", GL_VERTEX_SHADER);
    std::shared_ptr fragment_shader = Shader::create_from_file("./shader/per_vertex_color.fs", GL_FRAGMENT_SHADER);
    if (!vertex_shader || !fragment_shader) {
        return false;
    }

    // Link program.
    program_ = Program::create({vertex_shader, fragment_shader});
    if (!program_) {
        return false;
    }

    // Set clear color.
    glClearColor(0.0f, 0.1f, 0.2f, 0.0f);

    return true;
}

void Context::render() {
    // Clear with color that has been defined with `glClearColor`.
    glClear(GL_COLOR_BUFFER_BIT);

    program_->use();
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
}
