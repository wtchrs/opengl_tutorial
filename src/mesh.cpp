#include "glex/mesh.h"
#include <cstddef>
#include <cstdint>
#include <spdlog/spdlog.h>
#include "glex/common.h"

static const glm::vec2 *to_vec2(const float *p) {
    return reinterpret_cast<const glm::vec2 *>(p);
}

static const glm::vec3 *to_vec3(const float *p) {
    return reinterpret_cast<const glm::vec3 *>(p);
}

static const std::vector<Vertex>
data_to_vertices(const float *vert_data, const size_t vert_len, const uint32_t *idx_data, const size_t idx_len) {
    constexpr size_t STRIDE = 8;
    std::vector<Vertex> vertices;
    vertices.reserve(vert_len);
    for (size_t i = 0; i < vert_len; i += STRIDE) {
        const auto coord = to_vec3(&vert_data[i]);
        const auto norm = to_vec3(&vert_data[i + 3]);
        const auto uv = to_vec2(&vert_data[i + 6]);
        vertices.emplace_back(*coord, *norm, *uv);
    }
    for (size_t i = 0; i < idx_len; i += 3) {
        auto &[pos1, norm1, uv1, tan1] = vertices[idx_data[i]];
        auto &[pos2, norm2, uv2, tan2] = vertices[idx_data[i + 1]];
        auto &[pos3, norm3, uv3, tan3] = vertices[idx_data[i + 2]];
        tan1 += Vertex::compute_tangent(pos1, pos2, pos3, uv1, uv2, uv3);
        tan2 += Vertex::compute_tangent(pos2, pos1, pos3, uv2, uv1, uv3);
        tan3 += Vertex::compute_tangent(pos3, pos1, pos2, uv3, uv1, uv2);
    }
    for (auto &v : vertices) {
        v.tangent = glm::normalize(v.tangent);
    }
    return std::move(vertices);
}

glm::vec3 Vertex::compute_tangent(
        const glm::vec3 &coord1, const glm::vec3 &coord2, const glm::vec3 &coord3, const glm::vec2 &uv1,
        const glm::vec2 &uv2, const glm::vec2 &uv3
) {
    const auto edge1 = coord2 - coord1;
    const auto edge2 = coord3 - coord1;
    const auto delta_uv1 = uv2 - uv1;
    const auto delta_uv2 = uv3 - uv1;
    const float det = delta_uv1.x * delta_uv2.y - delta_uv1.y * delta_uv2.x;
    if (det == 0.0f) {
        return glm::vec3{0.0f};
    }
    float inv_det = 1.0f / det;
    return inv_det * (delta_uv2.y * edge1 + delta_uv1.y * edge2);
}

std::unique_ptr<Mesh> Mesh::create(
        Vertex *vertices, const size_t vertices_size, const uint32_t *indices, const size_t indices_size,
        const uint32_t primitive_type
) {
    if (primitive_type == GL_TRIANGLES) {
        // Set tangents.
        for (size_t i = 0; i < indices_size; i += 3) {
            auto &[pos1, norm1, uv1, tan1] = vertices[indices[i]];
            auto &[pos2, norm2, uv2, tan2] = vertices[indices[i + 1]];
            auto &[pos3, norm3, uv3, tan3] = vertices[indices[i + 2]];
            tan1 += Vertex::compute_tangent(pos1, pos2, pos3, uv1, uv2, uv3);
            tan2 += Vertex::compute_tangent(pos2, pos1, pos3, uv2, uv1, uv3);
            tan3 += Vertex::compute_tangent(pos3, pos1, pos2, uv3, uv1, uv2);
        }
        for (size_t i = 0; i < vertices_size; ++i) {
            vertices[i].tangent = glm::normalize(vertices[i].tangent);
        }
    }
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
    // Enable VAO attribute. (position, normal, texCoord, tangent)
    vertex_layout->set_attrib(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
    vertex_layout->set_attrib(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), offsetof(Vertex, normal));
    vertex_layout->set_attrib(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), offsetof(Vertex, tex_coord));
    vertex_layout->set_attrib(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), offsetof(Vertex, tangent));
    SPDLOG_INFO("Mesh has been created");
    return std::unique_ptr<Mesh>{new Mesh{primitive_type, std::move(vertex_layout), vertex_buffer, index_buffer}};
}

std::unique_ptr<Mesh>
Mesh::create(std::vector<Vertex> &vertices, const std::vector<uint32_t> &indices, const uint32_t primitive_type) {
    return create(vertices.data(), vertices.size(), indices.data(), indices.size(), primitive_type);
}

std::unique_ptr<Mesh> Mesh::create_cube() {
    // Declare vertices and indices for drawing a cube.
    static Vertex VERTICES[] = {
            {{0.5f, -0.5f, -0.5f}, {0.0f, -1.0f, 0.0f}, {0.0f, 1.0f}}, // bottom
            {{0.5f, -0.5f, 0.5f}, {0.0f, -1.0f, 0.0f}, {0.0f, 0.0f}},
            {{-0.5f, -0.5f, 0.5f}, {0.0f, -1.0f, 0.0f}, {1.0f, 0.0f}},
            {{-0.5f, -0.5f, -0.5f}, {0.0f, -1.0f, 0.0f}, {1.0f, 1.0f}},
            {{0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f}}, // back
            {{0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f}},
            {{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {1.0f, 0.0f}},
            {{-0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {1.0f, 1.0f}},
            {{0.5f, 0.5f, 0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}}, // right
            {{0.5f, -0.5f, 0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
            {{0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
            {{0.5f, 0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}},
            {{-0.5f, 0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}}, // front
            {{-0.5f, -0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
            {{0.5f, -0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
            {{0.5f, 0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
            {{-0.5f, 0.5f, -0.5f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}}, // left
            {{-0.5f, -0.5f, -0.5f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
            {{-0.5f, -0.5f, 0.5f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
            {{-0.5f, 0.5f, 0.5f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}},
            {{-0.5f, 0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f}}, // top
            {{-0.5f, 0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
            {{0.5f, 0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
            {{0.5f, 0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}},
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

std::unique_ptr<Mesh> Mesh::create_plain() {
    static Vertex VERTICES[] = {
            {{-0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
            {{-0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
            {{0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
            {{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
    };
    constexpr uint32_t INDICES[] = {0, 1, 2, 0, 2, 3};
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
        diffuse_->bind_to_unit(texture_count);
        program.set_uniform("material.diffuse", texture_count);
        texture_count++;
    }
    if (specular_) {
        specular_->bind_to_unit(texture_count);
        program.set_uniform("material.specular", texture_count);
    }
    program.set_uniform("material.shininess", shininess_);
}
