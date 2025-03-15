#include "glex/image.h"
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <glm/gtc/type_ptr.hpp>
#include <memory>
#include <ranges>
#include <spdlog/spdlog.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace sr = std::ranges;
namespace srv = std::ranges::views;

namespace {

    char ascii_to_lower(char ch) {
        if (ch >= 'A' && ch <= 'Z')
            return ch - ('Z' - 'z');
        return ch;
    }

} // namespace

std::unique_ptr<Image> Image::load(const std::string &filepath, const bool flip_vertical) {
    int width, height, channels;
    size_t bytes_per_channel;
    stbi_set_flip_vertically_on_load(flip_vertical);
    const auto ext =
            filepath.substr(filepath.find_last_of('.')) | srv::transform(ascii_to_lower) | sr::to<std::string>();
    unsigned char *image;
    if (ext == ".hdr") {
        image = reinterpret_cast<unsigned char *>(stbi_loadf(filepath.c_str(), &width, &height, &channels, 0));
        bytes_per_channel = 4;
    } else {
        image = stbi_load(filepath.c_str(), &width, &height, &channels, 0);
        bytes_per_channel = 1;
    }
    if (!image) {
        SPDLOG_ERROR("Failed to load image \"{}\"", filepath);
        return nullptr;
    }
    SPDLOG_INFO("Image has been loaded: \"{}\", {}x{}, {} channels", filepath, width, height, channels);
    return std::unique_ptr<Image>{new Image{
            static_cast<size_t>(width), static_cast<size_t>(height), static_cast<size_t>(channels), bytes_per_channel,
            image, filepath
    }};
}

std::unique_ptr<Image> Image::create(size_t width, size_t height, size_t channels, size_t bytes_per_channel) {
    auto *data = static_cast<uint8_t *>(malloc(width * height * channels * bytes_per_channel));
    if (!data) {
        SPDLOG_ERROR("Failed to allocate memory for new image");
        return nullptr;
    }
    auto image = std::unique_ptr<Image>{new Image{width, height, channels, bytes_per_channel, data, "TEMP_IMAGE"}};
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

Image::Image(
        const size_t width, const size_t height, const size_t channels, const size_t bytes_per_channel, uint8_t *data,
        const std::string &filepath
)
    : width_{width}
    , height_{height}
    , channels_{channels}
    , bytes_per_channel_{bytes_per_channel}
    , data_{data}
    , filepath_{filepath} {}
