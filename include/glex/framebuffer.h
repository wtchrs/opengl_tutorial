#ifndef __FRAMEBUFFER_H__
#define __FRAMEBUFFER_H__


#include <cstdint>
#include <memory>
#include "glex/texture.h"

class FrameBuffer {
    const uint32_t framebuffer_;
    const uint32_t depth_stencil_buffer_;
    const std::shared_ptr<Texture> color_attachment_;

public:
    static std::unique_ptr<FrameBuffer> create(const std::shared_ptr<Texture> color_attachment);
    static void bind_to_default();

    ~FrameBuffer();

    const uint32_t get() const {
        return framebuffer_;
    }

    void bind() const;

    const std::shared_ptr<Texture> get_color_attachment() const {
        return color_attachment_;
    }

private:
    FrameBuffer(uint32_t framebuffer_id, uint32_t depth_stencil_buffer, const std::shared_ptr<Texture> color_attachment)
        : framebuffer_{framebuffer_id}
        , depth_stencil_buffer_{depth_stencil_buffer}
        , color_attachment_{color_attachment} {}

    bool init() const;
};


#endif // __FRAMEBUFFER_H__
