#ifndef __CONTEXT_H__
#define __CONTEXT_H__


#include <memory>
#include "glex/buffer.h"
#include "glex/program.h"
#include "glex/texture.h"
#include "glex/vertex_layout.h"

/// # Context
///
/// A class that manages the OpenGL context, including shaders and buffer objects.
class Context {
    /// The shader program used for rendering.
    std::unique_ptr<Program> program_;
    /// VAO, Vertex Array Object
    std::unique_ptr<VertexLayout> vertex_layout_;
    /// VBO, Vertex Buffer Object
    std::unique_ptr<Buffer> vertex_buffer_;
    /// EBO, Element Buffer Object
    std::unique_ptr<Buffer> index_buffer_;
    /// Texture
    std::unique_ptr<Texture> texture1_, texture2_;

    /// @{
    /// Camera parameters
    glm::vec3 camera_pos_{0.0f, 0.0f, 3.0f}; ///< Camera position.
    glm::vec3 camera_front_{0.0f, 0.0f, -1.0f}; ///< Direction that camera is looking.
    glm::vec3 camera_up_{0.0f, 1.0f, 0.0f}; ///< Camera upvector.
    /// @}

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
    void render() const;

    void process_input(GLFWwindow *window);

private:
    Context() {}
    bool init();
};


#endif // __CONTEXT_H__
