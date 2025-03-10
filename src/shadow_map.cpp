#include "glex/shadow_map.h"
#include <cstdint>
#include <memory>
#include <spdlog/spdlog.h>
#include "glex/common.h"

std::unique_ptr<ShadowMap> ShadowMap::create(int width, int height) {
    uint32_t framebuffer_id;
    glGenFramebuffers(1, &framebuffer_id);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_id);

    const std::shared_ptr shadow_map = Texture::create(width, height, GL_DEPTH_COMPONENT, GL_FLOAT);
    if (!shadow_map) {
        SPDLOG_ERROR("Failed to complete creating shadow map framebuffer, shadow_map creation failed");
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        return nullptr;
    }
    shadow_map->set_filter(GL_LINEAR, GL_LINEAR);
    shadow_map->set_wrap(GL_CLAMP_TO_BORDER, GL_CLAMP_TO_BORDER);
    shadow_map->set_border_color(glm::vec4{1.0f});

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadow_map->get(), 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    if (auto status = glCheckFramebufferStatus(GL_FRAMEBUFFER); status != GL_FRAMEBUFFER_COMPLETE) {
        SPDLOG_ERROR("Failed to complete creating shadow map framebuffer: {}", status);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        return nullptr;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    SPDLOG_INFO("Shadow map has been created, framebuffer id: {}", framebuffer_id);
    return std::unique_ptr<ShadowMap>{new ShadowMap{framebuffer_id, shadow_map}};
}

ShadowMap::~ShadowMap() {
    if (framebuffer_) {
        SPDLOG_INFO("Delete shadow map, framebuffer id: {}", framebuffer_);
        glDeleteFramebuffers(1, &framebuffer_);
    }
}

void ShadowMap::bind() const {
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_);
}

ShadowMap::ShadowMap(const uint32_t framebuffer_id, const std::shared_ptr<Texture> shadow_map)
    : framebuffer_{framebuffer_id}
    , shadow_map_{shadow_map} {}
