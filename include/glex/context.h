#ifndef __CONTEXT_H__
#define __CONTEXT_H__


#include <memory>
#include "glex/mesh.h"
#include "glex/program.h"
#include "glex/shadow_map.h"

/// # Context
///
/// A class that manages the OpenGL context, including shaders and buffer objects.
class Context {
    struct Light {
        glm::vec3 position;
        float distance;
        bool directional;
        glm::vec3 direction;
        glm::vec2 cutoff; // inner cutoff angle, outer offset angle
        glm::vec3 ambient;
        glm::vec3 diffuse;
        glm::vec3 specular;
    };

    /// The shader programs used for rendering.
    std::unique_ptr<Program> program_, simple_program_, texture_program_, postprocess_program_, skybox_program_,
            env_map_program_, grass_program_, lighting_shadow_program_;
    /// The mesh object used for rendering vertices.
    std::shared_ptr<Mesh> cube_mesh_, plain_mesh_;

    std::unique_ptr<Buffer> grass_pos_buffer_;
    std::unique_ptr<VertexLayout> grass_instance_;

    std::shared_ptr<Material> floor_material_, cube_material1_, cube_material2_, window_material_, grass_material_;
    std::unique_ptr<CubeTexture> cube_texture_;

    std::unique_ptr<ShadowMap> shadow_map_;

    ///@{
    /// Default parameters
    static constexpr float CAMERA_PITCH{-40.0f}; ///< Camera pitch
    static constexpr float CAMERA_YAW{40.0f}; ///< Camera yaw

    static constexpr glm::vec3 CAMERA_POS{3.0f, 6.0f, 6.0f}; ///< Camera position
    static constexpr glm::vec3 CAMERA_FRONT{0.0f, 0.0f, -1.0f}; ///< Direction that camera is looking
    static constexpr glm::vec3 CAMERA_UP{0.0f, 1.0f, 0.0f}; ///< Camera up vector

    static constexpr Light LIGHT{
            {2.0f, 4.0f, 4.0f},
            150.0f,
            false,
            {-0.5f, -1.5f, -1.0f},
            {50.0f, 5.0f},
            {0.1f, 0.1f, 0.1f},
            {0.5f, 0.5f, 0.5f},
            {1.0f, 1.0f, 1.0f},
    };
    ///@}

    /// Lighting parameters
    Light light_ = LIGHT;
    bool blinn_ = true;
    float gamma_{1.0f};
    bool flash_light_mode_{false};

    ///@{
    /// Camera parameters
    float camera_pitch_{CAMERA_PITCH}; ///< Camera pitch
    float camera_yaw_{CAMERA_YAW}; ///< Camera yaw

    glm::vec3 camera_pos_{CAMERA_POS}; ///< Camera position
    glm::vec3 camera_front_{CAMERA_FRONT}; ///< Direction that camera is looking
    glm::vec3 camera_up_{CAMERA_UP}; ///< Camera up vector

    bool camera_rot_control_{false}; ///< Camera control flag
    glm::vec2 prev_mouse_pos_{0.0f}; ///< Previous mouse position
    ///@}

    /// Window width and height
    int width_{WINDOW_WIDTH}, height_{WINDOW_HEIGHT};
    /// Aspect ratio of window
    float aspect_ratio_{static_cast<float>(WINDOW_WIDTH) / static_cast<float>(WINDOW_HEIGHT)};

    glm::vec4 clear_color_{0.1f, 0.2f, 0.3f, 0.0f};
    float scale_{0.2};

    bool animation_{true};

public:
    /// ## Context::create
    ///
    /// Creates and initializes a new `Context` object.
    ///
    /// @returns `Context` object wrapped in `std::unique_ptr` if successful, or `nullptr` if initialization fails.
    static std::unique_ptr<Context> create();

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
