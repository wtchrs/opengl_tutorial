#ifndef __TEXTURE_H__
#define __TEXTURE_H__


#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <vector>
#include "glex/common.h"
#include "glex/image.h"

/// # Texture
///
/// A class that encapsulates an OpenGL texture object.
///
/// ## Examples
/// ```cpp
/// auto image = Image::load("image.jpg");
/// auto texture = Texture::create(*image);
/// texture->bind_to_unit(i);
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
///         GL_RGB, GL_UNSIGNED_BYTE, image->get_data()
/// );
///
/// glActiveTexture(GL_TEXTURE0 + i);
/// glBindTexture(GL_TEXTURE_2D, texture_id);
///
/// // ...do something
///
/// glDeleteTextures(1, &texture_);
/// ```
class Texture {
    const uint32_t texture_;

    const size_t width_, height_;
    const uint32_t format_;
    const uint32_t type_;

public:
    /// ## Texture::create
    ///
    /// Creates and initializes a new `Texture` object with **given image**.
    /// This Texture object has `GL_LINEAR_MIPMAP_LINEAR` min filter and `GL_LINEAR` mag filter,
    /// and `GL_CLAMP_TO_EDGE` wrap modes as default.
    ///
    /// @param image: The `Image` object containing the texture data.
    ///
    /// @returns `Texture` object wrapped in `std::unique_ptr` if successful, or `nullptr` if initialization fails.
    static std::unique_ptr<Texture> create(const Image &image);

    /// ## Texture::create
    ///
    /// Creates and initializes a new **empty** `Texture` object.
    /// This Texture object has `GL_LINEAR` min and mag filters, and `GL_CLAMP_TO_EDGE` wrap modes as default.
    ///
    /// @returns `Texture` object wrapped in `std::unique_ptr` if successful, or `nullptr` if initialization fails.
    static std::unique_ptr<Texture>
    create(size_t width, size_t height, uint32_t format, uint32_t type = GL_UNSIGNED_BYTE);

    /// ## Texture::~Texture
    ///
    /// Destructor that deletes the OpenGL texture.
    ~Texture();

    /// ## Texture::get
    ///
    /// @returns OpenGL texture ID.
    [[nodiscard]]
    uint32_t get() const {
        return texture_;
    }

    /// ## Texture::get_width
    ///
    /// @returns width of texture.
    [[nodiscard]]
    size_t get_width() const {
        return width_;
    }

    /// ## Texture::get_height
    ///
    /// @returns height of texture.
    [[nodiscard]]
    size_t get_height() const {
        return height_;
    }

    /// ## Texture::get_format
    ///
    /// @returns format of texture.
    [[nodiscard]]
    uint32_t get_format() const {
        return format_;
    }

    /// ## Texture::get_type
    ///
    /// @returns pixel type of texture.
    [[nodiscard]]
    uint32_t get_type() const {
        return type_;
    }

    /// ## Texture::bind
    ///
    /// Binds the OpenGL texture.
    void bind() const;

    /// ## Texture::bind_to_unit
    ///
    /// Assigns a texture to a texture unit. After this, the texture can be bound to a uniform variable by passing the
    /// unit ID as the second argument of `Program::set_uniform(std::string&, int)`.
    ///
    /// @param texture_unit: The texture unit ID to assign the texture to. It must be betwwen 0 and 31 because OpenGL
    /// provides only 32 texture units.
    void bind_to_unit(uint32_t texture_unit) const;

    /// ## Texture::set_filter
    ///
    /// Sets the texture filtering parameters.
    ///
    /// @param min_filter: The minifying function used whenever the pixel being textured maps to an area
    ///                    greater than one texture element.
    /// @param mag_filter: The magnification function used whenever the pixel being textured maps to an area
    ///                    less than or equal to one texture element.
    void set_filter(int32_t min_filter, int32_t mag_filter) const;

    /// # Texture::set_wrap
    ///
    /// Sets the texture wrapping parameters.
    ///
    /// @param s_wrap: The wrapping mode for texture coordinate `s`.
    /// @param t_wrap: The wrapping mode for texture coordinate `t`.
    void set_wrap(int32_t s_wrap, int32_t t_wrap) const;

    void set_border_color(const glm::vec4 &color) const;

private:
    Texture(uint32_t texture_id, size_t width, size_t height, uint32_t format, uint32_t type);
};


class CubeTexture {
    const uint32_t cube_texture_;

public:
    static std::unique_ptr<CubeTexture>
    create_from_images(const std::vector<std::reference_wrapper<const Image>> &images);

    ~CubeTexture();

    [[nodiscard]]
    uint32_t get() const {
        return cube_texture_;
    }

    void bind() const;

private:
    explicit CubeTexture(uint32_t texture_id)
        : cube_texture_{texture_id} {}
};


#endif // __TEXTURE_H__
