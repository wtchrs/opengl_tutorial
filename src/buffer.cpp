#include "glex/buffer.h"
#include <memory>
#include <spdlog/spdlog.h>
#include "glex/common.h"

std::unique_ptr<Buffer> Buffer::create_with_data(
        const uint32_t buffer_type, const uint32_t usage, const void *data, const size_t stride, const size_t count
) {
    uint32_t buffer_id;
    glGenBuffers(1, &buffer_id);
    if (const auto error = glGetError(); error != GL_NO_ERROR) {
        SPDLOG_ERROR("Failed to create buffer: {}", error);
        return nullptr;
    }
    auto buffer = std::unique_ptr<Buffer>{new Buffer{buffer_id, buffer_type, usage, stride, count}};
    buffer->bind();
    glBufferData(buffer_type, stride * count, data, usage);
    if (const auto error = glGetError(); error != GL_NO_ERROR) {
        SPDLOG_ERROR("Failed to set buffer data: {}", error);
        return nullptr;
    }
    SPDLOG_INFO("Buffer has been created: {}", buffer_id);
    return std::move(buffer);
}

Buffer::~Buffer() {
    if (buffer_) {
        glDeleteBuffers(1, &buffer_);
        SPDLOG_INFO("Delete buffer: {}", buffer_);
    }
}

void Buffer::bind() const {
    glBindBuffer(buffer_type_, buffer_);
}

Buffer::Buffer(
        const uint32_t buffer_id, const uint32_t buffer_type, const uint32_t usage, const size_t stride,
        const size_t count
)
    : buffer_{buffer_id}
    , buffer_type_(buffer_type)
    , usage_{usage}
    , stride_{stride}
    , count_{count} {}
