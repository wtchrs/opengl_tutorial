#ifndef __CONTEXT_H__
#define __CONTEXT_H__


#include <memory>
#include "glex/program.h"

/// # Context
///
/// A class that manages the OpenGL context, including shaders and buffer objects.
class Context {
protected:
    ///@{
    /// Default parameters
    static constexpr float CAMERA_PITCH{0.0f};
    static constexpr float CAMERA_YAW{0.0f};

    static constexpr glm::vec3 CAMERA_POS{0.0f, 0.0f, 3.0f};
    static constexpr glm::vec3 CAMERA_FRONT{0.0f, 0.0f, -1.0f};
    static constexpr glm::vec3 CAMERA_UP{0.0f, 1.0f, 0.0f};
    ///@}

    ///@{
    /// Camera parameters
    float camera_pitch_{CAMERA_PITCH};
    float camera_yaw_{CAMERA_YAW};

    glm::vec3 camera_pos_{CAMERA_POS};
    glm::vec3 camera_front_{CAMERA_FRONT};
    glm::vec3 camera_up_{CAMERA_UP};
    ///@}

    bool camera_rot_control_{false};
    glm::vec2 prev_mouse_pos_{0.0f};

    int width_{WINDOW_WIDTH}, height_{WINDOW_HEIGHT};
    float aspect_ratio_{static_cast<float>(WINDOW_WIDTH) / static_cast<float>(WINDOW_HEIGHT)};

public:
    /// ## Context::create
    ///
    /// Creates and initializes a new `Context` object.
    ///
    /// @returns `Context` object wrapped in `std::unique_ptr` if successful, or `nullptr` if initialization fails.
    static std::unique_ptr<Context> create();

    virtual ~Context() = default;

    /// ## Context::init
    ///
    /// Initializes a `Context` object.
    virtual bool init() = 0;

    /// ## Context::render
    ///
    /// Renders the scene using the current OpenGL context.
    virtual void render() = 0;

    /// ## Context::draw_ui
    ///
    /// Draw UI components using Dear ImGui API.
    virtual void draw_ui() = 0;

    /// ## Context::draw_scene
    ///
    /// Draw scene.
    virtual void draw_scene(const glm::mat4 &view, const glm::mat4 &projection, const Program &program) = 0;

    /// ## Context::reshape
    ///
    /// Updates the aspect ratio based on the new window dimensions.
    ///
    /// @param width: The new width of the window.
    /// @param height: The new height of the window.
    virtual void reshape(int width, int height) = 0;

    /// ## Context::process_input
    ///
    /// Processes input from the specified GLFW window to update the camera position.
    ///
    /// @param window: A pointer to the GLFW window from which to process input.
    void process_input(GLFWwindow *window);

    /// ## Context::mouse_move
    ///
    /// Handles mouse movement events to update the camera's orientation.
    ///
    /// @param x: The current x-coordinate of the mouse cursor.
    /// @param y: The current y-coordinate of the mouse cursor.
    void mouse_move(const double x, const double y);

    /// ## Context::mouse_button
    ///
    /// Handles mouse button events to enable or disable camera rotation control.
    ///
    /// @param button: The mouse button that was pressed or released.
    /// @param action: The action performed (press or release).
    /// @param x: The x-coordinate of the mouse cursor at the time of the event.
    /// @param y: The y-coordinate of the mouse cursor at the time of the event.
    void mouse_button(const int button, const int action, const double x, const double y);

protected:
    Context() {}
};


#endif // __CONTEXT_H__
