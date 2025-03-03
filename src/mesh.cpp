#include "glex/mesh.h"
#include <spdlog/spdlog.h>

std::unique_ptr<Mesh> Mesh::create(
        const Vertex *vertices, const size_t vertices_size, const uint32_t *indices, const size_t indices_size,
        const uint32_t primitive_type
) {
    // Generate VAO before generating VBO and EBO.
    auto vertex_layout = VertexLayout::create();
    if (!vertex_layout) {
        SPDLOG_ERROR("Failed to create mesh");
        return nullptr;
    }
    // Generate VBO from vertices.
    const std::shared_ptr vertex_buffer =
            Buffer::create_with_data(GL_ARRAY_BUFFER, GL_STATIC_DRAW, vertices, sizeof(Vertex), vertices_size);
    if (!vertex_buffer) {
        SPDLOG_ERROR("Failed to create mesh");
        return nullptr;
    }
    // Generate EBO from indices.
    const std::shared_ptr index_buffer =
            Buffer::create_with_data(GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW, indices, sizeof(uint32_t), indices_size);
    if (!index_buffer) {
        SPDLOG_ERROR("Failed to create mesh");
        return nullptr;
    }
    // Enable VAO attribute. (position, normal, texCoord)
    vertex_layout->set_attrib(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
    vertex_layout->set_attrib(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), offsetof(Vertex, normal));
    vertex_layout->set_attrib(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), offsetof(Vertex, tex_coord));
    SPDLOG_INFO("Mesh has been created");
    return std::unique_ptr<Mesh>{new Mesh{primitive_type, std::move(vertex_layout), vertex_buffer, index_buffer}};
}

std::unique_ptr<Mesh>
Mesh::create(const std::vector<Vertex> &vertices, const std::vector<uint32_t> &indices, const uint32_t primitive_type) {
    return create(vertices.data(), vertices.size(), indices.data(), indices.size(), primitive_type);
}

std::unique_ptr<Mesh> Mesh::create_cube() {
    // Declare vertices and indices for drawing a cube.
    constexpr Vertex VERTICES[] = {
            {{-0.5f, -0.5f, -0.5f}, {0.0f, -1.0f, 0.0f}, {0.0f, 1.0f}}, // bottom
            {{-0.5f, -0.5f, 0.5f}, {0.0f, -1.0f, 0.0f}, {0.0f, 0.0f}},
            {{0.5f, -0.5f, 0.5f}, {0.0f, -1.0f, 0.0f}, {1.0f, 0.0f}},
            {{0.5f, -0.5f, -0.5f}, {0.0f, -1.0f, 0.0f}, {1.0f, 1.0f}},
            {{-0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f}}, // back
            {{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f}},
            {{0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {1.0f, 0.0f}},
            {{0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {1.0f, 1.0f}},
            {{0.5f, 0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}}, // right
            {{0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
            {{0.5f, -0.5f, 0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
            {{0.5f, 0.5f, 0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}},
            {{0.5f, 0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}}, // front
            {{0.5f, -0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
            {{-0.5f, -0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
            {{-0.5f, 0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
            {{-0.5f, 0.5f, 0.5f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}}, // left
            {{-0.5f, -0.5f, 0.5f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
            {{-0.5f, -0.5f, -0.5f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
            {{-0.5f, 0.5f, -0.5f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}},
            {{0.5f, 0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f}}, // top
            {{0.5f, 0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
            {{-0.5f, 0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
            {{-0.5f, 0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}},
    };
    constexpr uint32_t INDICES[] = {
            0,  1,  2,  0,  2,  3, // bottom
            4,  5,  6,  4,  6,  7, // back
            8,  9,  10, 8,  10, 11, // right
            12, 13, 14, 12, 14, 15, // front
            16, 17, 18, 16, 18, 19, // left
            20, 21, 22, 20, 22, 23, // top
    };
    return create(
            VERTICES, sizeof(VERTICES) / sizeof(Vertex), INDICES, sizeof(INDICES) / sizeof(uint32_t), GL_TRIANGLES
    );
}

void Mesh::draw(const Program &program) const {
    vertex_layout_->bind();
    if (material_) {
        material_->set_to_program(program);
    }
    glDrawElements(primitive_type_, index_buffer_->get_count(), GL_UNSIGNED_INT, nullptr);
}

Mesh::Mesh(
        const uint32_t primitive_type, std::unique_ptr<VertexLayout> &&vertex_layout,
        const std::shared_ptr<Buffer> &vertex_buffer, const std::shared_ptr<Buffer> &index_buffer
)
    : primitive_type_{primitive_type}
    , vertex_layout_{std::move(vertex_layout)}
    , vertex_buffer_{vertex_buffer}
    , index_buffer_{index_buffer} {}

void Material::set_to_program(const Program &program) const {
    int texture_count = 0;
    if (diffuse_) {
        program.set_texture(texture_count, *diffuse_);
        program.set_uniform("material.diffuse", texture_count);
        texture_count++;
    }
    if (specular_) {
        program.set_texture(texture_count, *specular_);
        program.set_uniform("material.specular", texture_count);
    }
    program.set_uniform("material.shininess", shininess_);
}
