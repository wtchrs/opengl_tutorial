#ifndef __PROGRAM_H__
#define __PROGRAM_H__


#include <cstdint>
#include <memory>
#include <vector>
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

private:
    Program() {}
    bool link(const std::vector<std::shared_ptr<Shader>> &shaders);
};


#endif // __PROGRAM_H__
