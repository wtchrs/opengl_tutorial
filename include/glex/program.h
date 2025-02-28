#ifndef __PROGRAM_H__
#define __PROGRAM_H__


#include <cstdint>
#include <glm/fwd.hpp>
#include <memory>
#include <vector>
#include "common.h"
#include "glex/texture.h"
#include "shader.h"

/// # Program
///
/// A class that encapsulates an OpenGL shader program.
class Program {
    const uint32_t program_;

public:
    /// ## Program::create
    ///
    /// Creates and links a new `Program` object from the provided shaders.
    /// It links the given shaders into a pipeline program in the given order.
    ///
    /// @param shaders: A vector of shared pointers to `Shader` objects to be linked into the program.
    ///
    /// @returns `Program` object wrapped in `std::unique_ptr` if successful, or `nullptr` if linking fails.
    static std::unique_ptr<Program> create(const std::vector<std::shared_ptr<Shader>> &shaders);

    /// ## Program::create
    ///
    /// Creates and links a new `Program` object from the provided vertex and fragment shader files.
    /// It reads the shader source code from the files, compiles them, and links them into a pipeline program.
    ///
    /// @param vertex_shader_filename: The filename of the vertex shader source code.
    /// @param frag_shader_filename: The filename of the fragment shader source code.
    ///
    /// @returns `Program` object wrapped in `std::unique_ptr` if successful, or `nullptr` if linking fails.
    static std::unique_ptr<Program>
    create(const std::string &vertex_shader_filename, const std::string &frag_shader_filename);

    /// ## Program::~Program
    ///
    /// Destructor that deletes the OpenGL program.
    ~Program();

    /// ## Program::get
    ///
    /// @returns OpenGL program ID.
    [[nodiscard]]
    uint32_t get() const {
        return program_;
    }

    /// ## Program::use
    ///
    /// Uses the program for rendering using OpenGL `glUseProgram` function.
    void use() const;

    /// ## Program::set_uniform
    ///
    /// Sets an integer uniform value in the shader program.
    ///
    /// @param name: The name of the uniform variable in the shader.
    /// @param value: The integer value to set the uniform to.
    void set_uniform(const std::string &name, int value) const;

    /// ## Program::set_uniform
    ///
    /// Sets a float uniform value in the shader program.
    ///
    /// @param name: The name of the uniform variable in the shader.
    /// @param value: The float value to set the uniform to.
    void set_uniform(const std::string &name, float value) const;

    /// ## Program::set_uniform
    ///
    /// Sets a vector uniform value in the shader program.
    ///
    /// @param name: The name of the uniform variable in the shader.
    /// @param value: The `glm::vec2` vector value to set the uniform to.
    void set_uniform(const std::string &name, const glm::vec2 &value) const;

    /// ## Program::set_uniform
    ///
    /// Sets a vector uniform value in the shader program.
    ///
    /// @param name: The name of the uniform variable in the shader.
    /// @param value: The `glm::vec3` vector value to set the uniform to.
    void set_uniform(const std::string &name, const glm::vec3 &value) const;

    /// ## Program::set_uniform
    ///
    /// Sets a vector uniform value in the shader program.
    ///
    /// @param name: The name of the uniform variable in the shader.
    /// @param value: The `glm::vec4` vector value to set the uniform to.
    void set_uniform(const std::string &name, const glm::vec4 &value) const;

    /// ## Program::set_uniform
    ///
    /// Sets a matrix uniform value in the shader program.
    ///
    /// @param name: The name of the uniform variable in the shader.
    /// @param value: The `glm::mat4` matrix value to set the uniform to.
    void set_uniform(const std::string &name, const glm::mat4 &value) const;

    /// ## Program::set_texture
    ///
    /// Assigns a texture to a texture slot in this shader program.
    /// After this, the texture can be bound to a uniform variable by passing the slot ID as the second argument of
    /// `Program::set_uniform(std::string&, int)`.
    ///
    /// @param slot: The texture slot ID to assign the texture to.
    /// @param texture: The `Texture` object to assign.
    void set_texture(int slot, const Texture &texture) const;

private:
    explicit Program(uint32_t program);

    [[nodiscard]]
    bool link(const std::vector<std::shared_ptr<Shader>> &shaders) const;
};


#endif // __PROGRAM_H__
