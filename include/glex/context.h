#ifndef __CONTEXT_H__
#define __CONTEXT_H__


#include <memory>
#include "glex/program.h"

/// # Context
///
/// A class that manages the OpenGL context, including shaders and buffer objects.
class Context {
public:
    /// ## Context::create
    ///
    /// Creates and initializes a new `Context` object.
    ///
    /// @returns `Context` object wrapped in `std::unique_ptr` if successful, or `nullptr` if initialization fails.
    static std::unique_ptr<Context> create();

    ~Context();

    /// ## Context::render
    ///
    /// Renders the scene using the current OpenGL context.
    void render();

    void draw_ui();

    void draw_scene(const glm::mat4 &view, const glm::mat4 &projection, const Program &program);

    /// ## Context::process_input
    ///
    /// Processes input from the specified GLFW window to update the camera position.
    ///
    /// @param window: A pointer to the GLFW window from which to process input.
    void process_input(GLFWwindow *window);

    /// ## Context::reshape
    ///
    /// Updates the aspect ratio based on the new window dimensions.
    ///
    /// @param width: The new width of the window.
    /// @param height: The new height of the window.
    void reshape(int width, int height);

    /// ## Context::mouse_move
    ///
    /// Handles mouse movement events to update the camera's orientation.
    ///
    /// @param x: The current x-coordinate of the mouse cursor.
    /// @param y: The current y-coordinate of the mouse cursor.
    void mouse_move(double x, double y);

    /// ## Context::mouse_button
    ///
    /// Handles mouse button events to enable or disable camera rotation control.
    ///
    /// @param button: The mouse button that was pressed or released.
    /// @param action: The action performed (press or release).
    /// @param x: The x-coordinate of the mouse cursor at the time of the event.
    /// @param y: The y-coordinate of the mouse cursor at the time of the event.
    void mouse_button(int button, int action, double x, double y);

private:
    Context() {}
    bool init();
};


#endif // __CONTEXT_H__
