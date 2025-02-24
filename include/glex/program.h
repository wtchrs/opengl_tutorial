#ifndef __PROGRAM_H__
#define __PROGRAM_H__


#include <cstdint>
#include <glm/fwd.hpp>
#include <memory>
#include <vector>
#include "glex/texture.h"
#include "shader.h"

/// # Program
///
/// A class that encapsulates an OpenGL shader program.
class Program {
    uint32_t program_{0};

public:
    /// # Program::create
    ///
    /// Creates and links a new `Program` object from the provided shaders.
    ///
    /// ## Parameters
    /// - `shaders`: A vector of shared pointers to `Shader` objects to be linked into the program.
    ///
    /// ## Returns
    /// A `Program` object wrapped in `std::unique_ptr` if successful, or `nullptr` if linking fails.
    ///
    /// ## Details
    /// This function creates and links a new `Program` object from the provided shaders.
    /// It links the given shaders into a pipeline program in the given order.
    static std::unique_ptr<Program> create(const std::vector<std::shared_ptr<Shader>> &shaders);

    /// # Program::~Program
    ///
    /// Destructor that deletes the OpenGL program.
    ~Program();

    /// # Program::get
    ///
    /// Returns the OpenGL program ID.
    ///
    /// ## Returns
    /// The OpenGL program ID.
    [[nodiscard]]
    uint32_t get() const {
        return program_;
    }

    /// # Program::use
    ///
    /// Uses the program for rendering using OpenGL `glUseProgram` function.
    void use() const;

    /// # Program::set_uniform
    ///
    /// Sets an integer uniform value in the shader program.
    ///
    /// ## Parameters
    /// - `name`: The name of the uniform variable in the shader.
    /// - `value`: The integer value to set the uniform to.
    void set_uniform(const std::string &name, int value) const;

    /// # Program::set_uniform
    ///
    /// Sets a matrix uniform value in the shader program.
    ///
    /// ## Parameters
    /// - `name`: The name of the uniform variable in the shader.
    /// - `value`: The `glm::mat4` matrix value to set the uniform to.
    void set_uniform(const std::string &name, const glm::mat4 &value) const;

    /// # Program::set_texture
    ///
    /// Sets a texture uniform value in the shader program and binds the texture to a texture unit.
    ///
    /// ## Parameters
    /// - `name`: The name of the uniform variable in the shader.
    /// - `slot`: The texture slot to bind the texture to.
    /// - `texture`: The `Texture` object to bind.
    void set_texture(const std::string &name, int slot, const Texture &texture) const;

private:
    Program() {}
    bool link(const std::vector<std::shared_ptr<Shader>> &shaders);
};


#endif // __PROGRAM_H__
