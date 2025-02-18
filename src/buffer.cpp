#include "glex/buffer.h"
#include <memory>
#include <spdlog/spdlog.h>
#include "glex/common.h"

std::unique_ptr<Buffer>
Buffer::create_with_data(uint32_t buffer_type, uint32_t usage, const void *data, size_t data_size) {
    auto buffer = std::unique_ptr<Buffer>{new Buffer{}};
    if (!buffer->init(buffer_type, usage, data, data_size)) {
        SPDLOG_ERROR("Failed to create buffer.");
        return nullptr;
    }
    SPDLOG_INFO("Buffer has been created: {}", buffer->get());
    return std::move(buffer);
}

Buffer::~Buffer() {
    if (buffer_) {
        glDeleteBuffers(1, &buffer_);
    }
}

void Buffer::bind() const {
    glBindBuffer(buffer_type_, buffer_);
}

bool Buffer::init(uint32_t buffer_type, uint32_t usage, const void *data, size_t data_size) {
    buffer_type_ = buffer_type;
    usage_ = usage;
    glGenBuffers(1, &buffer_);
    bind();
    glBufferData(buffer_type_, data_size, data, usage_);
    return true;
}
