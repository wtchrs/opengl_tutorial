#include "glex/texture.h"
#include <cstddef>
#include <cstdint>
#include <functional>
#include <glm/gtc/type_ptr.hpp>
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
    auto texture = std::unique_ptr<Texture>{
            new Texture{texture_id, image.get_width(), image.get_height(), format, GL_UNSIGNED_BYTE}
    };
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

std::unique_ptr<Texture> Texture::create(int width, int height, uint32_t format, uint32_t type) {
    uint32_t texture_id;
    glGenTextures(1, &texture_id);
    if (const auto error = glGetError(); error != GL_NO_ERROR) {
        SPDLOG_ERROR("Failed to create texture: {}", error);
        return nullptr;
    }
    auto texture = std::unique_ptr<Texture>{new Texture{texture_id, width, height, format, type}};
    texture->bind();
    texture->set_filter(GL_LINEAR, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, type, nullptr);
    SPDLOG_INFO("Texture has been created: {}", texture_id);
    return texture;
}

Texture::Texture(uint32_t texture_id, int width, int height, uint32_t format, uint32_t type)
    : texture_{texture_id}
    , width_{width}
    , height_{height}
    , format_{format}
    , type_{type} {}

Texture::~Texture() {
    if (texture_) {
        SPDLOG_INFO("Delete texture: {}", texture_);
        glDeleteTextures(1, &texture_);
    }
}

void Texture::bind() const {
    glBindTexture(GL_TEXTURE_2D, texture_);
}

void Texture::bind_to_unit(uint32_t texture_unit) const {
    if (texture_unit < 0 || texture_unit > 31) {
        SPDLOG_ERROR("Texture unit id to bind must be between 0 and 31, got: {}", texture_unit);
        return;
    }
    glActiveTexture(GL_TEXTURE0 + texture_unit);
    bind();
}

void Texture::set_filter(const int32_t min_filter, const int32_t mag_filter) const {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag_filter);
}

void Texture::set_wrap(const int32_t s_wrap, const int32_t t_wrap) const {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, s_wrap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, t_wrap);
}

void Texture::set_border_color(const glm::vec4 &color) const {
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, glm::value_ptr(color));
}

std::unique_ptr<CubeTexture>
CubeTexture::create_from_images(const std::vector<std::reference_wrapper<const Image>> &images) {
    uint32_t texture_id;
    glGenTextures(1, &texture_id);
    if (const auto error = glGetError(); error != GL_NO_ERROR) {
        SPDLOG_ERROR("Failed to create texture: {}", error);
        return nullptr;
    }
    auto cube_texture = std::unique_ptr<CubeTexture>{new CubeTexture{texture_id}};
    cube_texture->bind();
    // Set filter and wrap.
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    // Apply each images to cube map texture.
    for (size_t i = 0; i < images.size(); ++i) {
        auto &image = images[i].get();
        auto format = channels_to_format(image.get_channels());
        glTexImage2D(
                GL_TEXTURE_CUBE_MAP_POSITIVE_X + static_cast<uint32_t>(i), 0, GL_RGB, image.get_width(),
                image.get_height(), 0, format, GL_UNSIGNED_BYTE, image.get_data()
        );
    }
    SPDLOG_INFO("Cube texture has been created: {}", texture_id);
    return std::move(cube_texture);
}

CubeTexture::~CubeTexture() {
    if (cube_texture_) {
        glDeleteTextures(1, &cube_texture_);
    }
}

void CubeTexture::bind() const {
    glBindTexture(GL_TEXTURE_CUBE_MAP, cube_texture_);
}
