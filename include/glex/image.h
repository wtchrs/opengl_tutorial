#ifndef __IMAGE_H__
#define __IMAGE_H__


#include <cstdint>
#include <memory>
#include <string>

/// # Image
///
/// A class that encapsulates an image loaded from a file.
class Image {
    const int width_;
    const int height_;
    const int channels_;
    uint8_t *const data_;

    const std::string filepath_;

public:
    /// ## Image::load
    ///
    /// Loads an image from the specified file path.
    ///
    /// @param filepath: The path to the image file.
    ///
    /// @returns `std::unique_ptr` to an `Image` object if successful, or `nullptr` if loading fails.
    [[nodiscard]]
    static std::unique_ptr<Image> load(const std::string &filepath);

    static std::unique_ptr<Image> create(int width, int height, int channels = 4);

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
    int get_width() const {
        return width_;
    }

    /// ## Image::get_height
    ///
    /// @returns height of the image.
    [[nodiscard]]
    int get_height() const {
        return height_;
    }

    /// ## Image::get_channels
    ///
    /// @returns number of color channels in the image.
    [[nodiscard]]
    int get_channels() const {
        return channels_;
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

private:
    /// ## Image::Image
    ///
    /// Private constructor to initialize an `Image` object.
    ///
    /// @param width: The width of the image.
    /// @param height: The height of the image.
    /// @param channels: The number of color channels in the image.
    /// @param data: A pointer to the raw pixel data.
    Image(int width, int height, int channels, uint8_t *data, const std::string &filepath);
};


#endif // __IMAGE_H__
