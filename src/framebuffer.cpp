#include "glex/framebuffer.h"
#include <cstddef>
#include <cstdint>
#include <memory>
#include <spdlog/spdlog.h>
#include "glex/common.h"

std::unique_ptr<FrameBuffer> FrameBuffer::create(const std::vector<std::shared_ptr<Texture>> &color_attachments) {
    // Generate framebuffer and renderbuffer.
    uint32_t framebuffer_id;
    glGenFramebuffers(1, &framebuffer_id);
    uint32_t renderbuffer_id;
    glGenRenderbuffers(1, &renderbuffer_id);
    // Create and initialize framebuffer.
    auto framebuffer =
            std::unique_ptr<FrameBuffer>{new FrameBuffer{framebuffer_id, renderbuffer_id, color_attachments}};
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

    for (size_t i = 0; i < get_color_attachments_size(); ++i) {
        glFramebufferTexture2D(
                GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, color_attachments_[i]->get(), 0
        );
    }

    if (auto size = get_color_attachments_size(); size > 0) {
        std::vector<GLenum> attachments(size);
        for (size_t i = 0; i < size; ++i) {
            attachments[i] = GL_COLOR_ATTACHMENT0 + i;
        }
        // The draw buffers setting is specific to each framebuffer. So, when switching between framebuffers,
        // the draw buffers setting is automatically restored to the previously set value for that framebuffer.
        // (It isn't necessary to call `glDrawBuffers` if only one color attachments(GL_COLOR_ATTACHMENT0) is needed
        // because `GL_COLOR_ATTACHMENT0` is the default draw buffer.)
        glDrawBuffers(size, attachments.data());
    }

    int width = color_attachments_[0]->get_width();
    int height = color_attachments_[0]->get_height();

    glBindRenderbuffer(GL_RENDERBUFFER, depth_stencil_buffer_);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
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

std::unique_ptr<CubeFrameBuffer>
CubeFrameBuffer::create(const std::shared_ptr<CubeTexture> &color_attachment, uint32_t mip_level) {
    uint32_t framebuffer_id;
    glGenFramebuffers(1, &framebuffer_id);
    uint32_t renderbuffer_id;
    glGenRenderbuffers(1, &renderbuffer_id);
    auto framebuffer = std::unique_ptr<CubeFrameBuffer>{
            new CubeFrameBuffer{framebuffer_id, renderbuffer_id, color_attachment, mip_level}
    };
    if (!framebuffer->init()) {
        SPDLOG_ERROR("Failed to create cube framebuffer");
        return nullptr;
    }
    SPDLOG_INFO("FrameBuffer created: framebuffer: {}, renderbuffer: {}", framebuffer_id, renderbuffer_id);
    return std::move(framebuffer);
}

CubeFrameBuffer::~CubeFrameBuffer() {
    if (depth_stencil_buffer_id_) {
        glDeleteRenderbuffers(1, &depth_stencil_buffer_id_);
    }
    if (framebuffer_id_) {
        glDeleteFramebuffers(1, &framebuffer_id_);
    }
}

void CubeFrameBuffer::bind(int cube_index) const {
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_id_);
    glFramebufferTexture2D(
            GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + cube_index, color_attachment_->get(),
            mip_level_
    );
}

bool CubeFrameBuffer::init() {
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_id_);
    glFramebufferTexture2D(
            GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X, color_attachment_->get(), mip_level_
    );

    size_t width = color_attachment_->get_width() >> mip_level_;
    size_t height = color_attachment_->get_height() >> mip_level_;
    glBindRenderbuffer(GL_RENDERBUFFER, depth_stencil_buffer_id_);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depth_stencil_buffer_id_);

    if (auto result = glCheckFramebufferStatus(GL_FRAMEBUFFER); result != GL_FRAMEBUFFER_COMPLETE) {
        SPDLOG_ERROR("Failed to initialize cube framebuffer: 0x{:04x}", result);
        FrameBuffer::bind_to_default();
        return false;
    }

    FrameBuffer::bind_to_default();
    return true;
}
