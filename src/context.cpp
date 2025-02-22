#include "glex/context.h"
#include <cstddef>
#include <cstdint>
#include <glm/gtc/type_ptr.hpp>
#include <glm/trigonometric.hpp>
#include <memory>
#include <spdlog/spdlog.h>
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
    // Declare vertices and indices for drawing a Rectangle.
    float vertices[] = {
            0.5f,  0.5f,  0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, // 3: position, 3: color, 2: texture coordinate
            0.5f,  -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, //
            -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, //
            -0.5f, 0.5f,  0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, //
    };
    uint32_t indices[] = {
            0, 1, 3, // first triangle
            1, 2, 3, // second triangle
    };
    size_t stride = 8; // number of floats in one vertex

    // Generate VAO, Vertex Array Object.
    // VAO must be generated before VBO generated.
    vertex_layout_ = VertexLayout::create();

    // Generate VBO, Vertex Buffer Object.
    // GL_ARRAY_BUFFER means VBO.
    // usage in `glBufferData` can be "GL_(STATIC|DYNAMIC|STREAM)_(DRAW|COPY|READ)"
    // GL_STATIC_DRAW means that this vertices will not be changed.
    vertex_buffer_ = Buffer::create_with_data(GL_ARRAY_BUFFER, GL_STATIC_DRAW, vertices, sizeof(vertices));

    // Set and enable VAO attribute.
    vertex_layout_->set_attrib(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * stride, 0);
    vertex_layout_->set_attrib(1, 3, GL_FLOAT, GL_FALSE, sizeof(float) * stride, sizeof(float) * 3);
    vertex_layout_->set_attrib(2, 2, GL_FLOAT, GL_FALSE, sizeof(float) * stride, sizeof(float) * 6);

    // Generate EBO, Element Buffer Object.
    // GL_ELEMENT_ARRAY_BUFFER means EBO.
    index_buffer_ = Buffer::create_with_data(GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW, indices, sizeof(indices));

    // Load and compile shaders.
    std::shared_ptr vertex_shader = Shader::create_from_file("./shader/transform.vs", GL_VERTEX_SHADER);
    std::shared_ptr fragment_shader = Shader::create_from_file("./shader/transform.fs", GL_FRAGMENT_SHADER);
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
    glActiveTexture(GL_TEXTURE0);
    texture1_->bind();
    glActiveTexture(GL_TEXTURE1);
    texture2_->bind();

    // Provide texture slot numbers to uniform locations.
    program_->use();
    glUniform1i(glGetUniformLocation(program_->get(), "tex1"), 0);
    glUniform1i(glGetUniformLocation(program_->get(), "tex2"), 1);

    // # Transform matrix
    // - Model matrix: Local space -> World space
    // - View matrix: World space -> View space
    // - Projection matrix: View space -> Canonical space
    // - MVP matrix: Model-View-Projection matrix

    // Create transform matrix that apply rotation, scaling, and translation.
    // Each glm transform funtions multiply corresponding transform matrix in left-side.
    /*
    auto transform = glm::rotate(
            // scaling and translation transform after rotation
            glm::scale(glm::translate(glm::mat4{1.0f}, glm::vec3{0.8f, 0.5f, 0.0f}), glm::vec3{0.5f}),
            glm::radians(60.0f), // 45 degrees for rotation
            glm::vec3{0.0f, 0.0f, 0.1f} // rotation axis
    );
    */

    // Object is rotated -55 degrees in the x-axis.
    auto model = glm::rotate(glm::mat4{1.0f}, glm::radians(-55.0f), glm::vec3{1.0f, 0.0f, 0.0f});
    // Camera is -3 units away from the origin in the z-axis.
    auto view = glm::translate(glm::mat4{1.0f}, glm::vec3{0.0f, 0.0f, -3.0f});
    // Perspective projection in aspect ratio 4:3 and fov 45 degrees.
    auto projection = glm::perspective(
            glm::radians(45.0f), // fov
            static_cast<float>(WINDOW_WIDTH) / static_cast<float>(WINDOW_HEIGHT), // aspect ratio
            0.01f, 10.0f // near and far
    );
    // MVP matrix
    auto transform = projection * view * model;

    // Pass transform matrix as OpenGL uniform value.
    auto transform_loc = glGetUniformLocation(program_->get(), "transform");
    glUniformMatrix4fv(transform_loc, 1, GL_FALSE, glm::value_ptr(transform));

    return true;
}

void Context::render() const {
    // Clear with color that has been defined with `glClearColor`.
    glClear(GL_COLOR_BUFFER_BIT);

    program_->use();
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
}
