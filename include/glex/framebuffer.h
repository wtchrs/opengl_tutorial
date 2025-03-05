#ifndef __FRAMEBUFFER_H__
#define __FRAMEBUFFER_H__


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
    const std::shared_ptr<Texture> color_attachment_;

public:
    /// ## FrameBuffer::create
    ///
    /// Creates and initializes a new `FrameBuffer` object with the given color attachment texture.
    ///
    /// @param color_attachment: Shared pointer to the color attachment texture.
    ///
    /// @returns `FrameBuffer` object wrapped in `std::unique_ptr` if successful, or `nullptr` if initialization fails.
    static std::unique_ptr<FrameBuffer> create(std::shared_ptr<Texture> color_attachment);

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

    /// ## FrameBuffer::get_color_attachment
    ///
    /// @returns Shared pointer to the color attachment texture.
    [[nodiscard]]
    std::shared_ptr<Texture> get_color_attachment() const {
        return color_attachment_;
    }

private:
    FrameBuffer(
            const uint32_t framebuffer_id, const uint32_t depth_stencil_buffer,
            const std::shared_ptr<Texture> &color_attachment
    )
        : framebuffer_{framebuffer_id}
        , depth_stencil_buffer_{depth_stencil_buffer}
        , color_attachment_{color_attachment} {}

    /// ## FrameBuffer::init
    ///
    /// Initializes the framebuffer by attaching the color attachment texture and depth-stencil renderbuffer.
    ///
    /// @returns `true` if the framebuffer is initialized successfully, `false` otherwise.
    bool init() const;
};


#endif // __FRAMEBUFFER_H__
