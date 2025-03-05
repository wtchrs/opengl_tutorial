#include "glex/framebuffer.h"
#include <cstdint>
#include <spdlog/spdlog.h>
#include "glex/common.h"

std::unique_ptr<FrameBuffer> FrameBuffer::create(const std::shared_ptr<Texture> color_attachment) {
    // Generate framebuffer and renderbuffer.
    uint32_t framebuffer_id;
    glGenFramebuffers(1, &framebuffer_id);
    uint32_t renderbuffer_id;
    glGenRenderbuffers(1, &renderbuffer_id);
    // Create and initialize framebuffer.
    auto framebuffer = std::unique_ptr<FrameBuffer>{new FrameBuffer{framebuffer_id, renderbuffer_id, color_attachment}};
    if (!framebuffer->init()) {
        SPDLOG_ERROR("Failed to create framebuffer");
        return nullptr;
    }
    SPDLOG_INFO("FrameBuffer created: framebuffer: {}, renderbuffer: {}", framebuffer_id, renderbuffer_id);
    return std::move(framebuffer);
}
void FrameBuffer::bind_to_default() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

FrameBuffer::~FrameBuffer() {
    if (depth_stencil_buffer_) {
        SPDLOG_INFO("Delete renderbuffer: {}", depth_stencil_buffer_);
        glDeleteRenderbuffers(1, &depth_stencil_buffer_);
    }
    if (framebuffer_) {
        SPDLOG_INFO("Delete framebuffer: {}", framebuffer_);
        glDeleteFramebuffers(1, &framebuffer_);
    }
}

void FrameBuffer::bind() const {
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_);
}

bool FrameBuffer::init() const {
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color_attachment_->get(), 0);

    glBindRenderbuffer(GL_RENDERBUFFER, depth_stencil_buffer_);
    glRenderbufferStorage(
            GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, color_attachment_->get_width(), color_attachment_->get_height()
    );
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depth_stencil_buffer_);

    auto result = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (result != GL_FRAMEBUFFER_COMPLETE) {
        SPDLOG_ERROR("Failed to initialize framebuffer: {}", result);
        bind_to_default();
        return false;
    }

    bind_to_default();
    return true;
}
