#include "glex/texture.h"
#include <cstdint>
#include <memory>
#include <spdlog/spdlog.h>
#include "glex/common.h"

std::unique_ptr<Texture> Texture::create() {
    uint32_t texture_id;
    glGenTextures(1, &texture_id);
    auto texture = std::unique_ptr<Texture>{new Texture{texture_id}};
    texture->bind();
    // Set default filter and wrap.
    // GL_LINEAR_MIPMAP_LINEAR: Trilinear interpolation
    texture->set_filter(GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);
    texture->set_wrap(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
    SPDLOG_INFO("Texture has been created: {}", texture_id);
    return texture;
}

Texture::Texture(const uint32_t texture_id)
    : texture_{texture_id} {}

Texture::~Texture() {
    if (texture_) {
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

void Texture::set_texture_image(const int32_t level, const Image &image) {
    GLenum format;
    switch (image.get_channels()) {
    case 1: format = GL_RED; break;
    case 2: format = GL_RG; break;
    case 3: format = GL_RGB; break;
    default: format = GL_RGBA; break;
    }
    glTexImage2D(
            GL_TEXTURE_2D, level, //
            GL_RGBA, image.get_width(), image.get_height(), // OpenGL texture settings
            0, // border
            format, GL_UNSIGNED_BYTE, image.get_data() // original image information
    );
    glGenerateMipmap(GL_TEXTURE_2D);
    SPDLOG_INFO(
            "Texture image has been set: {}x{}, {} channels", image.get_width(), image.get_height(),
            image.get_channels()
    );
}
