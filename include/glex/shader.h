#ifndef __SHADER_H__
#define __SHADER_H__

#include <cstdint>
#include <memory>
#include "common.h"

/// # Shader
///
/// A class that encapsulates an OpenGL shader.
class Shader {
    const uint32_t shader_;

public:
    /// ## Shader::create_from_file
    ///
    /// Creates a new `Shader` object from a file.
    ///
    /// @param filename: The path to the shader file.
    /// @param shader_type: The type of the shader (e.g., `GL_VERTEX_SHADER`, `GL_FRAGMENT_SHADER`).
    ///
    /// @returns `Shader` object wrapped in `std::unique_ptr` if successful, or `nullptr` if creation fails.
    static std::unique_ptr<Shader> create_from_file(const std::string &filename, GLenum shader_type);

    /// ## Shader::~Shader
    ///
    /// Destructor that deletes the OpenGL shader.
    ~Shader();

    /// ## Shader::get
    ///
    /// @returns OpenGL shader ID.
    [[nodiscard]]
    uint32_t get() const {
        return shader_;
    }

private:
    explicit Shader(uint32_t shader);

    /// ## Shader::load_file
    ///
    /// Loads and compiles a shader from a file.
    ///
    /// @param filename: The path to the shader file.
    ///
    /// @returns `true` if the shader is successfully loaded and compiled, `false` otherwise.
    bool load_file(const std::string &filename) const;
};

#endif // __SHADER_H__
