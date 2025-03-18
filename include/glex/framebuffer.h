#ifndef __FRAMEBUFFER_H__
#define __FRAMEBUFFER_H__


#include <cstddef>
#include <cstdint>
#include <memory>
#include "glex/texture.h"

/// # FrameBuffer
///
/// A class that encapsulates an OpenGL framebuffer object.
class FrameBuffer {
    /// OpenGL framebuffer ID
    const uint32_t framebuffer_;
    /// OpenGL renderbuffer ID for depth and stencil buffer
    const uint32_t depth_stencil_buffer_;
    /// Color attachment texture
    const std::vector<std::shared_ptr<Texture>> color_attachments_;

public:
    /// ## FrameBuffer::create
    ///
    /// Creates and initializes a new `FrameBuffer` object with the given color attachment texture.
    ///
    /// @param color_attachments: Vector of shared pointers to the color attachment textures.
    ///
    /// @returns `FrameBuffer` object wrapped in `std::unique_ptr` if successful, or `nullptr` if initialization fails.
    static std::unique_ptr<FrameBuffer> create(const std::vector<std::shared_ptr<Texture>> &color_attachments);

    /// ## FrameBuffer::bind_to_default
    ///
    /// Binds the default framebuffer (0) to the OpenGL context.
    static void bind_to_default();

    /// ## FrameBuffer::~FrameBuffer
    ///
    /// Destructor that deletes the OpenGL framebuffer and renderbuffer.
    ~FrameBuffer();

    /// ## FrameBuffer::get
    ///
    /// @returns OpenGL framebuffer ID.
    [[nodiscard]]
    uint32_t get() const {
        return framebuffer_;
    }

    /// ## FrameBuffer::bind
    ///
    /// Binds the framebuffer to the OpenGL context.
    void bind() const;

    /// ## FrameBuffer::get_color_attachments_size
    ///
    /// @returns The number of color attachment textures.
    [[nodiscard]]
    size_t get_color_attachments_size() const {
        return color_attachments_.size();
    }

    /// ## FrameBuffer::get_color_attachment
    ///
    /// @param index: Index to get.
    ///
    /// @returns Shared pointer to the color attachment texture from color attachment vector.
    [[nodiscard]]
    std::shared_ptr<Texture> get_color_attachment(int index = 0) const {
        return color_attachments_[index];
    }

private:
    FrameBuffer(
            const uint32_t framebuffer_id, const uint32_t depth_stencil_buffer,
            const std::vector<std::shared_ptr<Texture>> &color_attachments
    )
        : framebuffer_{framebuffer_id}
        , depth_stencil_buffer_{depth_stencil_buffer}
        , color_attachments_{color_attachments} {}

    /// ## FrameBuffer::init
    ///
    /// Initializes the framebuffer by attaching the color attachment texture and depth-stencil renderbuffer.
    ///
    /// @returns `true` if the framebuffer is initialized successfully, `false` otherwise.
    bool init() const;
};

class CubeFrameBuffer {
    const uint32_t framebuffer_id_;
    const uint32_t depth_stencil_buffer_id_;
    const uint32_t mip_level_;
    const std::shared_ptr<CubeTexture> color_attachment_;

public:
    static std::unique_ptr<CubeFrameBuffer>
    create(const std::shared_ptr<CubeTexture> &color_attachment, uint32_t mip_level = 0);

    ~CubeFrameBuffer();

    [[nodiscard]]
    const uint32_t get() const {
        return framebuffer_id_;
    }

    [[nodiscard]]
    const std::shared_ptr<CubeTexture> get_color_attachment() const {
        return color_attachment_;
    }

    void bind(int cube_index = 0) const;

private:
    CubeFrameBuffer(
            uint32_t framebuffer_id, uint32_t depth_stencil_buffer_id, std::shared_ptr<CubeTexture> color_attachment,
            uint32_t mip_level
    )
        : framebuffer_id_{framebuffer_id}
        , depth_stencil_buffer_id_{depth_stencil_buffer_id}
        , color_attachment_{color_attachment}
        , mip_level_{mip_level} {}

    bool init();
};


#endif // __FRAMEBUFFER_H__
