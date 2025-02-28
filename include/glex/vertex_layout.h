#ifndef __VERTEX_LAYOUT_H__
#define __VERTEX_LAYOUT_H__


#include <cstddef>
#include <cstdint>
#include <memory>

/// # VertexLayout
///
/// A class that encapsulates an OpenGL vertex array object.
class VertexLayout {
    const uint32_t vertex_array_object_;

public:
    /// # VertexLayout::create
    ///
    /// Creates and initializes a new `VertexLayout` object.
    ///
    /// ## Returns
    /// A `VertexLayout` object wrapped in `std::unique_ptr` if successful, or `nullptr` if initialization fails.
    static std::unique_ptr<VertexLayout> create();

    /// # VertexLayout::~VertexLayout
    ///
    /// Destructor that deletes the OpenGL Vertex Array Object.
    ~VertexLayout();

    /// # VertexLayout::get
    ///
    /// Returns the OpenGL vertex array object ID.
    ///
    /// ## Returns
    /// The OpenGL vertex array object ID.
    [[nodiscard]]
    uint32_t get() const {
        return vertex_array_object_;
    }

    /// # VertexLayout::bind
    ///
    /// Binds the OpenGL vertex array object.
    void bind() const;

    /// # VertexLayout::set_attrib
    ///
    /// Sets the vertex attribute pointer for the specified attribute index.
    ///
    /// ## Parameters
    /// - `attrib_index`: The index of the vertex attribute.
    /// - `count`: The number of components per vertex attribute.
    /// - `type`: The data type of each component in the array.
    /// - `normalized`: Whether fixed-point data values should be normalized.
    /// - `stride`: The byte offset between consecutive vertex attributes.
    /// - `offset`: The offset of the first component of the first vertex attribute in the array.
    void
    set_attrib(uint32_t attrib_index, int count, uint32_t type, bool normalized, size_t stride, uint64_t offset) const;

    /// # VertexLayout::disable_attrib
    ///
    /// Disables the vertex attribute array at the specified index.
    ///
    /// ## Parameters
    /// - `attrib_index`: The index of the vertex attribute to disable.
    void disable_attrib(int attrib_index) const;

private:
    explicit VertexLayout(uint32_t vertex_array_object);
};


#endif // __VERTEX_LAYOUT_H__
