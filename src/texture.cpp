#include "glex/texture.h"
#include <cstdint>
#include <memory>
#include <spdlog/spdlog.h>
#include "glex/common.h"

static GLenum channels_to_format(int channels) {
    switch (channels) {
    case 1: return GL_RED;
    case 2: return GL_RG;
    case 3: return GL_RGB;
    default: return GL_RGBA;
    }
}

std::unique_ptr<Texture> Texture::create(const Image &image) {
    uint32_t texture_id;
    glGenTextures(1, &texture_id);
    if (const auto error = glGetError(); error != GL_NO_ERROR) {
        SPDLOG_ERROR("Failed to create texture: {}", error);
        return nullptr;
    }
    const auto format = channels_to_format(image.get_channels());
    auto texture = std::unique_ptr<Texture>{new Texture{texture_id, image.get_width(), image.get_height(), format}};
    texture->bind();
    // Set default filter and wrap.
    // GL_LINEAR_MIPMAP_LINEAR: Trilinear interpolation
    texture->set_filter(GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);
    texture->set_wrap(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
    glTexImage2D(
            GL_TEXTURE_2D, 0, GL_RGBA, image.get_width(), image.get_height(), 0, format, GL_UNSIGNED_BYTE,
            image.get_data()
    );
    glGenerateMipmap(GL_TEXTURE_2D);
    SPDLOG_INFO(
            "Texture image has been set: {}x{}, {} channels", image.get_width(), image.get_height(),
            image.get_channels()
    );
    SPDLOG_INFO("Texture has been created: {}", texture_id);
    return std::move(texture);
}

std::unique_ptr<Texture> Texture::create(int width, int height, uint32_t format) {
    uint32_t texture_id;
    glGenTextures(1, &texture_id);
    if (const auto error = glGetError(); error != GL_NO_ERROR) {
        SPDLOG_ERROR("Failed to create texture: {}", error);
        return nullptr;
    }
    auto texture = std::unique_ptr<Texture>{new Texture{texture_id, width, height, format}};
    texture->bind();
    texture->set_filter(GL_LINEAR, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, format, GL_UNSIGNED_BYTE, nullptr);
    SPDLOG_INFO("Texture has been created: {}", texture_id);
    return texture;
}

Texture::Texture(uint32_t texture_id, int width, int height, uint32_t format)
    : texture_{texture_id}
    , width_{width}
    , height_{height}
    , format_{format} {}

Texture::~Texture() {
    if (texture_) {
        SPDLOG_INFO("Delete texture: {}", texture_);
        glDeleteTextures(1, &texture_);
    }
}

void Texture::bind() const {
    glBindTexture(GL_TEXTURE_2D, texture_);
}

void Texture::set_filter(const int32_t min_filter, const int32_t mag_filter) const {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag_filter);
}

void Texture::set_wrap(const int32_t s_wrap, const int32_t t_wrap) const {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, s_wrap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, t_wrap);
}
