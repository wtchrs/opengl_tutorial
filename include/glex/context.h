#ifndef __CONTEXT_H__
#define __CONTEXT_H__


#include <cstdint>
#include <memory>
#include "glex/buffer.h"
#include "program.h"

/// # Context
///
/// A class that manages the OpenGL context, including shaders and buffer objects.
class Context {
    /// The shader program used for rendering.
    std::unique_ptr<Program> program_;
    /// VBO, Vertex Buffer Object
    std::unique_ptr<Buffer> vertex_buffer_;
    /// VAO, Vertex Array Object
    uint32_t vertex_array_object_{0};
    /// EBO, Element Buffer Object
    std::unique_ptr<Buffer> index_buffer_;

public:
    /// # Context::create
    ///
    /// Creates and initializes a new `Context` object.
    ///
    /// ## Returns
    /// A `Context` object wrapped in `std::unique_ptr` if successful, or `nullptr` if initialization fails.
    static std::unique_ptr<Context> create();

    /// # Context::render
    ///
    /// Renders the scene using the current OpenGL context.
    void render();

private:
    Context() {}
    bool init();
};


#endif // __CONTEXT_H__
