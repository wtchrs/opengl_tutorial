#ifndef __SHADOW_MAP_H__
#define __SHADOW_MAP_H__


#include <cstdint>
#include <memory>
#include "glex/texture.h"

class ShadowMap {
    /// For rendering to depth map.
    const uint32_t framebuffer_;
    /// For storing depth map.
    const std::shared_ptr<Texture> shadow_map_;

public:
    static std::unique_ptr<ShadowMap> create(int width, int height);

    ~ShadowMap();

    uint32_t get() const {
        return framebuffer_;
    }

    void bind() const;

    const std::shared_ptr<Texture> get_shadow_map() const {
        return shadow_map_;
    }

private:
    ShadowMap(const uint32_t framebuffer_id, const std::shared_ptr<Texture> shadow_map);
};


#endif // !__SHADOW_MAP_H__
