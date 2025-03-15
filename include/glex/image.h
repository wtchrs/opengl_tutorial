#ifndef __IMAGE_H__
#define __IMAGE_H__


#include <cstddef>
#include <cstdint>
#include <glm/vec4.hpp>
#include <memory>
#include <string>

/// # Image
///
/// A class that encapsulates an image loaded from a file.
class Image {
    const size_t width_;
    const size_t height_;
    const size_t channels_;
    const size_t bytes_per_channel_;
    uint8_t *const data_;

    const std::string filepath_;

public:
    /// ## Image::load
    ///
    /// Loads an image from the specified file path.
    ///
    /// @param filepath: The path to the image file.
    /// @param flip_vertical: Whether to load the image with its vertical flipped or load it as is.
    ///
    /// @returns `std::unique_ptr` to an `Image` object if successful, or `nullptr` if loading fails.
    [[nodiscard]]
    static std::unique_ptr<Image> load(const std::string &filepath, bool flip_vertical = true);

    /// ## Image::create
    ///
    /// Creates a new empty image with the specified dimensions and number of color channels.
    ///
    /// @param width: The width of the image.
    /// @param height: The height of the image.
    /// @param channels: The number of color channels in the image (default is 4).
    /// @param bytes_per_channel: The number of bytes per channel.
    ///
    /// @returns `std::unique_ptr` to an `Image` object if successful, or `nullptr` if creation fails.
    static std::unique_ptr<Image>
    create(size_t width, size_t height, size_t channels = 4, size_t bytes_per_channel = 1);

    /// ## Image::~Image
    ///
    /// Destructor that frees the image data.
    ~Image();

    /// ## Image::get_data
    ///
    /// @returns pointer to the raw pixel data of the image.
    [[nodiscard]]
    const uint8_t *get_data() const {
        return data_;
    }

    /// ## Image::get_width
    ///
    /// @returns width of the image.
    [[nodiscard]]
    size_t get_width() const {
        return width_;
    }

    /// ## Image::get_height
    ///
    /// @returns height of the image.
    [[nodiscard]]
    size_t get_height() const {
        return height_;
    }

    /// ## Image::get_channels
    ///
    /// @returns number of color channels in the image.
    [[nodiscard]]
    size_t get_channels() const {
        return channels_;
    }

    /// ## Image::get_bytes_per_channel
    ///
    /// @returns number of bytes per channels in the image.
    [[nodiscard]]
    size_t get_bytes_per_channel() const {
        return bytes_per_channel_;
    }

    /// ## Image::set_check_image
    ///
    /// Sets the image data to a checkerboard pattern.
    ///
    /// @param grid_x: The number of horizontal squares in the checkerboard.
    /// @param grid_y: The number of vertical squares in the checkerboard.
    ///
    /// #### Details
    /// This function modifies the image data to create a checkerboard pattern with the specified number of horizontal
    /// and vertical squares.
    void set_check_image(int grid_x, int grid_y) const;

    /// ## Image::set_single_color_image
    ///
    /// Sets the image data to a single color.
    ///
    /// @param color: The color to set the image to, represented as a `glm::vec4`.
    ///
    /// #### Details
    /// This function modifies the image data to fill the entire image with the specified color.
    void set_single_color_image(const glm::vec4 &color) const;

private:
    /// ## Image::Image
    ///
    /// Private constructor to initialize an `Image` object.
    ///
    /// @param width: The width of the image.
    /// @param height: The height of the image.
    /// @param channels: The number of color channels in the image.
    /// @param bytes_per_channel: The number of bytes per channel.
    /// @param data: A pointer to the raw pixel data.
    Image(size_t width, size_t height, size_t channels, size_t bytes_per_channel, uint8_t *data,
          const std::string &filepath);
};


#endif // __IMAGE_H__
