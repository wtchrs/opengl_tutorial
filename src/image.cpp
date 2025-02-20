#include "glex/image.h"
#include <cstdint>
#include <memory>
#include <spdlog/spdlog.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

std::unique_ptr<Image> Image::load(const std::string &filepath) {
    int width, height, channels;
    unsigned char *image = stbi_load(filepath.c_str(), &width, &height, &channels, 0);
    if (!image) {
        SPDLOG_ERROR("Failed to load image \"{}\"", filepath);
        return nullptr;
    }
    SPDLOG_INFO("Image has been loaded: \"{}\" ({}x{}, {} channels)", filepath, width, height, channels);
    return std::unique_ptr<Image>{new Image{width, height, channels, image}};
}

Image::~Image() {
    if (data_) {
        stbi_image_free(reinterpret_cast<void *>(data_));
    }
}

Image::Image(int width, int height, int channels, uint8_t *data)
    : width_{width}
    , height_{height}
    , channels_{channels}
    , data_{data} {}
