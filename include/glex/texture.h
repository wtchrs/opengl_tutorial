#ifndef __TEXTURE_H__
#define __TEXTURE_H__


#include <cstdint>
#include <memory>
#include "glex/image.h"

/// # Texture
///
/// A class that encapsulates an OpenGL texture object.
///
/// ## Examples
/// ```cpp
/// auto image = Image::load("image.jpg");
/// auto texture = Texture::create();
/// texture->set_texture_image(0, *image);
///
/// // ...do something
/// ```
///
/// The above example is equivalent to:
///
/// ```cpp
/// auto image = Image::load("image.jpg");
/// uint32_t texture_id;
/// glGenTextures(1, &texture_id);
/// glBindTexture(GL_TEXTURE_2D, texture_id);
/// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
/// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
/// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
/// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
///
/// glTexImage2D(
///         GL_TEXTURE_2D, 0, GL_RGBA, image->get_width(), image->get_height(), 0,
///         GL_RGB, // channel of image
///         GL_UNSIGNED_BYTE, image->get_data()
/// );
///
/// // ...do something
///
/// glDeleteTextures(1, &texture_);
/// ```
class Texture {
    const uint32_t texture_;

public:
    /// # Texture::create
    ///
    /// Creates and initializes a new `Texture` object.
    /// This Texture object has `GL_LINEAR` min and mag filters, and `GL_CLAMP_TO_EDGE` wrap modes as default.
    ///
    /// ## Returns
    /// A `Texture` object wrapped in `std::unique_ptr` if successful, or `nullptr` if initialization fails.
    static std::unique_ptr<Texture> create();

    /// # Texture::~Texture
    ///
    /// Destructor that deletes the OpenGL texture.
    ~Texture();

    /// # Texture::get
    ///
    /// Returns the OpenGL texture ID.
    ///
    /// ## Returns
    /// The OpenGL texture ID.
    [[nodiscard]]
    uint32_t get() const {
        return texture_;
    }

    /// # Texture::bind
    ///
    /// Binds the OpenGL texture.
    void bind() const;

    /// # Texture::set_filter
    ///
    /// Sets the texture filtering parameters.
    ///
    /// ## Parameters
    /// - `min_filter`: The minifying function used whenever the pixel being textured maps to an area
    ///                 greater than one texture element.
    /// - `mag_filter`: The magnification function used whenever the pixel being textured maps to an area
    ///                 less than or equal to one texture element.
    void set_filter(int32_t min_filter, int32_t mag_filter) const;

    /// # Texture::set_wrap
    ///
    /// Sets the texture wrapping parameters.
    ///
    /// ## Parameters
    /// - `s_wrap`: The wrapping mode for texture coordinate `s`.
    /// - `t_wrap`: The wrapping mode for texture coordinate `t`.
    void set_wrap(int32_t s_wrap, int32_t t_wrap) const;

    /// # Texture::set_texture_image
    ///
    /// Sets the texture image data.
    ///
    /// ## Parameters
    /// - `level`: The level-of-detail number.
    ///            Level 0 is the base image level. Level n is the nth mipmap reduction image.
    /// - `image`: The `Image` object containing the texture data.
    void set_texture_image(int32_t level, const Image &image);

private:
    explicit Texture(uint32_t texture_id);
};


#endif // __TEXTURE_H__
