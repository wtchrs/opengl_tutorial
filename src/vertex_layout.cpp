#include "glex/vertex_layout.h"
#include <memory>
#include <spdlog/spdlog.h>
#include "glex/common.h"

std::unique_ptr<VertexLayout> VertexLayout::create() {
    auto vertex_layout = std::unique_ptr<VertexLayout>{new VertexLayout{}};
    vertex_layout->init();
    SPDLOG_INFO("VertexLayout has been created: {}", vertex_layout->get());
    return std::move(vertex_layout);
}

VertexLayout::~VertexLayout() {
    if (vertex_array_object_) {
        glDeleteVertexArrays(1, &vertex_array_object_);
    }
}

void VertexLayout::bind() const {
    glBindVertexArray(vertex_array_object_);
}

void VertexLayout::set_attrib(
        uint32_t attrib_index, int count, uint32_t type, bool normalized, size_t stride, uint64_t offset
) const {
    glEnableVertexAttribArray(attrib_index);
    glVertexAttribPointer(attrib_index, count, type, normalized, stride, reinterpret_cast<void *>(offset));
}

void VertexLayout::disable_attrib(int attrib_index) const {
    glDisableVertexAttribArray(attrib_index);
}

void VertexLayout::init() {
    glGenVertexArrays(1, &vertex_array_object_);
    bind();
}
