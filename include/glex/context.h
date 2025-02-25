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

    float camera_pitch_{0.0f}; ///< Camera pitch
    float camera_yaw_{0.0f}; ///< Camera yaw

    glm::vec3 camera_pos_{0.0f, 0.0f, 3.0f}; ///< Camera position
    glm::vec3 camera_front_{0.0f, 0.0f, -1.0f}; ///< Direction that camera is looking
    glm::vec3 camera_up_{0.0f, 1.0f, 0.0f}; ///< Camera up vector

    bool camera_rot_control_{false}; ///< Camera control flag
    glm::vec2 prev_mouse_pos_{0.0f}; ///< Previous mouse position

    /// @}

    /// Aspect ratio of window
    float aspect_ratio_{static_cast<float>(WINDOW_WIDTH) / static_cast<float>(WINDOW_HEIGHT)};

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

    /// # Context::process_input
    ///
    /// Processes input from the specified GLFW window to update the camera position.
    ///
    /// ## Parameters
    /// - `window`: A pointer to the GLFW window from which to process input.
    void process_input(GLFWwindow *window);

    /// # Context::reshape
    ///
    /// Updates the aspect ratio based on the new window dimensions.
    ///
    /// ## Parameters
    /// - `width`: The new width of the window.
    /// - `height`: The new height of the window.
    void reshape(int width, int height);

    /// # Context::mouse_move
    ///
    /// Handles mouse movement events to update the camera's orientation.
    ///
    /// ## Parameters
    /// - `x`: The current x-coordinate of the mouse cursor.
    /// - `y`: The current y-coordinate of the mouse cursor.
    void mouse_move(double x, double y);

    /// # Context::mouse_button
    ///
    /// Handles mouse button events to enable or disable camera rotation control.
    ///
    /// ## Parameters
    /// - `button`: The mouse button that was pressed or released.
    /// - `action`: The action performed (press or release).
    /// - `x`: The x-coordinate of the mouse cursor at the time of the event.
    /// - `y`: The y-coordinate of the mouse cursor at the time of the event.
    void mouse_button(int button, int action, double x, double y);

private:
    Context() {}
    bool init();
};


#endif // __CONTEXT_H__
