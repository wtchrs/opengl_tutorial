#ifndef __MESH_H__
#define __MESH_H__


#include <memory>
#include <vector>
#include "glex/buffer.h"
#include "glex/common.h"
#include "glex/program.h"
#include "glex/texture.h"
#include "glex/vertex_layout.h"

/// # Vertex
///
/// A structure that represents a single vertex in a mesh.
struct Vertex {
    /// Position
    glm::vec3 position;
    /// Normal vector
    glm::vec3 normal;
    /// Texture coordinate
    glm::vec2 tex_coord;

    /// ## Vertex::Vertex
    ///
    /// Constructor to initialize a `Vertex` object.
    ///
    /// @param position: The position of the vertex.
    /// @param normal: The normal vector of the vertex.
    /// @param tex_coord: The texture coordinate of the vertex.
    constexpr Vertex(const glm::vec3 position, const glm::vec3 normal, const glm::vec2 tex_coord)
        : position{position}
        , normal{normal}
        , tex_coord{tex_coord} {}
};

/// # Material
///
/// A class that represents the material properties of a mesh.
struct Material {
    /// Diffuse map texture
    const std::shared_ptr<Texture> diffuse_;
    /// Specular map texture
    const std::shared_ptr<Texture> specular_;
    /// Shininess factor
    const float shininess_;

    /// ## Material::Material
    ///
    /// Constructor to initialize a `Material` object.
    ///
    /// @param diffuse: Shared pointer to the diffuse texture.
    /// @param specular: Shared pointer to the specular texture.
    /// @param shininess: The shininess factor of the material (default is 32.0f).
    Material(
            const std::shared_ptr<Texture> &diffuse, const std::shared_ptr<Texture> &specular,
            const float shininess = 32.0f
    )
        : diffuse_{diffuse}
        , specular_{specular}
        , shininess_{shininess} {}

    /// ## Material::set_to_program
    ///
    /// Sets the material properties to the specified shader program.
    ///
    /// @param program: Pointer to the `Program` object.
    void set_to_program(const Program *program) const;
};

/// # Mesh
///
/// A class that encapsulates the OpenGL vertex layout, vertex buffer, and element buffer for a mesh.
class Mesh {
    /// Type of primitive to render (e.g., GL_TRIANGLES)
    const uint32_t primitive_type_;
    /// VAO, Vertex Array Object
    const std::unique_ptr<VertexLayout> vertex_layout_;
    /// VBO, Vertex Buffer Object
    const std::shared_ptr<Buffer> vertex_buffer_;
    /// EBO, Element Buffer Object
    const std::shared_ptr<Buffer> index_buffer_;
    /// Material
    std::shared_ptr<Material> material_;

public:
    /// ## Mesh::create
    ///
    /// Creates and initializes a new `Mesh` object from the provided vertex and index data.
    ///
    /// @param vertices: Pointer to an array of `Vertex` structures.
    /// @param vertices_size: The number of vertices in the array.
    /// @param indices: Pointer to an array of indices.
    /// @param indices_size: The number of indices in the array.
    /// @param primitive_type: The type of primitive to render (e.g., GL_TRIANGLES).
    ///
    /// @returns `Mesh` object wrapped in `std::unique_ptr` if successful, or `nullptr` if initialization fails.
    static std::unique_ptr<Mesh>
    create(const Vertex *vertices, size_t vertices_size, const uint32_t *indices, size_t indices_size,
           uint32_t primitive_type);

    /// ## Mesh::create
    ///
    /// Creates and initializes a new `Mesh` object from the provided vertex and index data.
    ///
    /// @param vertices: A vector of `Vertex` structures.
    /// @param indices: A vector of indices.
    /// @param primitive_type: The type of primitive to render (e.g., GL_TRIANGLES).
    ///
    /// @returns `Mesh` object wrapped in `std::unique_ptr` if successful, or `nullptr` if initialization fails.
    static std::unique_ptr<Mesh>
    create(const std::vector<Vertex> &vertices, const std::vector<uint32_t> &indices, uint32_t primitive_type);

    /// ## Mesh::create_cube
    ///
    /// Creates and initializes a new `Mesh` object representing a cube.
    ///
    /// @returns `Mesh` object wrapped in `std::unique_ptr` if successful, or `nullptr` if initialization fails.
    static std::unique_ptr<Mesh> create_cube();

    /// ## Mesh::get_vertex_layout
    ///
    /// @returns Pointer to the `VertexLayout` object.
    [[nodiscard]]
    const VertexLayout *get_vertex_layout() const {
        return vertex_layout_.get();
    }

    /// ## Mesh::get_vertex_buffer
    ///
    /// @returns Shared pointer to the `Buffer` object representing the vertex buffer.
    [[nodiscard]]
    std::shared_ptr<Buffer> get_vertex_buffer() const {
        return vertex_buffer_;
    }

    /// ## Mesh::get_index_buffer
    ///
    /// @returns Shared pointer to the `Buffer` object representing the index buffer.
    [[nodiscard]]
    std::shared_ptr<Buffer> get_index_buffer() const {
        return index_buffer_;
    }

    /// ## Mesh::set_material
    ///
    /// Sets the material for the mesh.
    ///
    /// @param material: Shared pointer to the `Material` object.
    void set_material(const std::shared_ptr<Material> &material) {
        material_ = material;
    }

    /// ## Mesh::get_material
    ///
    /// @returns Shared pointer to the `Material` object.
    [[nodiscard]]
    std::shared_ptr<Material> get_material() const {
        return material_;
    }

    /// ## Mesh::draw
    ///
    /// @param program Pointer to the `Program` object.
    ///
    /// Draws the mesh using the current OpenGL context.
    void draw(const Program *program) const;

private:
    Mesh(uint32_t primitive_type, std::unique_ptr<VertexLayout> &&vertex_layout,
         const std::shared_ptr<Buffer> &vertex_buffer, const std::shared_ptr<Buffer> &index_buffer);
};


#endif
