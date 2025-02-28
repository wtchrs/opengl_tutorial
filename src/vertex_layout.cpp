#include "glex/vertex_layout.h"
#include <memory>
#include <spdlog/spdlog.h>
#include "glex/common.h"

std::unique_ptr<VertexLayout> VertexLayout::create() {
    uint32_t vertex_array_object_id;
    glGenVertexArrays(1, &vertex_array_object_id);
    if (const auto error = glGetError(); error != GL_NO_ERROR) {
        SPDLOG_ERROR("Failed to create vertex array object: {}", error);
        return nullptr;
    }
    auto vertex_layout = std::unique_ptr<VertexLayout>{new VertexLayout{vertex_array_object_id}};
    vertex_layout->bind();
    SPDLOG_INFO("VertexLayout has been created: {}", vertex_layout->get());
    return std::move(vertex_layout);
}

VertexLayout::VertexLayout(const uint32_t vertex_array_object)
    : vertex_array_object_{vertex_array_object} {}

VertexLayout::~VertexLayout() {
    if (vertex_array_object_) {
        SPDLOG_INFO("Delete vertex array object: {}", vertex_array_object_);
        glDeleteVertexArrays(1, &vertex_array_object_);
    }
}

void VertexLayout::bind() const {
    glBindVertexArray(vertex_array_object_);
}

void VertexLayout::set_attrib(
        const uint32_t attrib_index, const int count, const uint32_t type, const bool normalized, const size_t stride,
        const uint64_t offset
) const {
    glEnableVertexAttribArray(attrib_index);
    glVertexAttribPointer(attrib_index, count, type, normalized, stride, reinterpret_cast<void *>(offset));
}

void VertexLayout::disable_attrib(const int attrib_index) const {
    glDisableVertexAttribArray(attrib_index);
}
