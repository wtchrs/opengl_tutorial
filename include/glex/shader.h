#ifndef __SHADER_H__
#define __SHADER_H__

#include <cstdint>
#include <memory>
#include "common.h"

class Shader {
private:
    uint32_t shader_{0};

public:
    static std::unique_ptr<Shader> create_from_file(const std::string &filename, GLenum shader_type);

    ~Shader();

    uint32_t get() const {
        return shader_;
    }

private:
    Shader() {}
    bool load_file(const std::string &filename, GLenum shader_type);
};

#endif // __SHADER_H__
