#include "glex/image.h"
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <glm/gtc/type_ptr.hpp>
#include <memory>
#include <spdlog/spdlog.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

std::unique_ptr<Image> Image::load(const std::string &filepath, const bool flip_vertical) {
    int width, height, channels;
    stbi_set_flip_vertically_on_load(flip_vertical);
    unsigned char *image = stbi_load(filepath.c_str(), &width, &height, &channels, 0);
    if (!image) {
        SPDLOG_ERROR("Failed to load image \"{}\"", filepath);
        return nullptr;
    }
    SPDLOG_INFO("Image has been loaded: \"{}\" {}x{}, {} channels", filepath, width, height, channels);
    return std::unique_ptr<Image>{new Image{width, height, channels, image, filepath}};
}

std::unique_ptr<Image> Image::create(int width, int height, int channels) {
    auto *data = static_cast<uint8_t *>(malloc(width * height * channels));
    if (!data) {
        SPDLOG_ERROR("Failed to allocate memory for new image");
        return nullptr;
    }
    auto image = std::unique_ptr<Image>{new Image{width, height, channels, data, "CHECK_IMAGE"}};
    SPDLOG_INFO("Empty image has been created: {}x{}, {} channels", width, height, channels);
    return image;
}

void Image::set_check_image(const int grid_x, const int grid_y) const {
    for (size_t i = 0; i < height_; ++i) {
        for (size_t j = 0; j < width_; ++j) {
            const size_t pos = (i * width_ + j) * channels_;
            const bool even = (i / grid_y + j / grid_x) % 2 == 0;
            uint8_t value = even ? 255 : 0;
            std::fill_n(&data_[pos], static_cast<size_t>(channels_), value);
            if (channels_ > 3) {
                data_[pos + 3] = 255;
            }
        }
    }
}

void Image::set_single_color_image(const glm::vec4 &color) const {
    const auto clamped = glm::clamp(color * 255.0f, 0.0f, 255.0f);
    const uint8_t rgb[] = {
            static_cast<uint8_t>(clamped.r), static_cast<uint8_t>(clamped.g), static_cast<uint8_t>(clamped.b),
            static_cast<uint8_t>(clamped.a)
    };
    for (size_t i = 0; i < height_; ++i) {
        for (size_t j = 0; j < width_; ++j) {
            const size_t pos = (i * width_ + j) * channels_;
            std::copy_n(rgb, channels_, &data_[pos]);
        }
    }
}

Image::~Image() {
    if (data_) {
        SPDLOG_INFO("Unload image: \"{}\"", filepath_);
        stbi_image_free(data_);
    }
}

Image::Image(const int width, const int height, const int channels, uint8_t *data, const std::string &filepath)
    : width_{width}
    , height_{height}
    , channels_{channels}
    , data_{data}
    , filepath_{filepath} {}
