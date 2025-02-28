#ifndef __BUFFER_H__
#define __BUFFER_H__


#include <cstddef>
#include <cstdint>
#include <memory>

/// # Buffer
///
/// A class that encapsulates an OpenGL buffer object.
///
/// ## Examples
///
/// ```cpp
/// float vertices[] = {
///     // ...
/// };
/// auto buffer = Buffer::create_with_data(
///         GL_ARRAY_BUFFER, GL_STATIC_DRAW, vertices, sizeof(float), sizeof(vertices) / sizeof(float)
/// );
/// ```
///
/// The above single line is equivalent to:
///
/// ```cpp
/// float vertices[] = {
///     // ...
/// };
/// uint32_t vertex_buffer;
/// glGenBuffers(1, &vertex_buffer);
/// glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
/// glBufferData(GL_ARRAY_BUFFER, sizeof(float) * len, vertices, GL_STATIC_DRAW);
///
/// // ...do something
///
/// glDeleteBuffers(1, &vertex_buffer); // `glDeleteBuffers` function is automatically called in the destructor.
/// ```
class Buffer {
    const uint32_t buffer_;
    const uint32_t buffer_type_;
    const uint32_t usage_;
    const size_t stride_;
    const size_t count_;

public:
    /// ## Buffer::create_with_data
    ///
    /// Generates an OpenGL buffer and sets the provided data.
    ///
    /// @param buffer_type: Buffer type to bind.
    /// @param usage: Usage pattern of the data store.
    /// @param data: Pointer to the data to be set in the generated buffer.
    /// @param stride: The size of each element in the buffer.
    /// @param count: The number of elements in the buffer.
    ///
    /// @returns `Buffer` object wrapped in `std::unique_ptr`.
    static std::unique_ptr<Buffer>
    create_with_data(uint32_t buffer_type, uint32_t usage, const void *data, size_t stride, size_t count);

    /// ## Buffer::~Buffer
    ///
    /// Destructor that deletes the OpenGL buffer. It calls `glDeleteBuffers` function to delete the buffer.
    ~Buffer();

    /// ## Buffer::get
    ///
    /// @returns OpenGL buffer ID.
    [[nodiscard]]
    uint32_t get() const {
        return buffer_;
    }

    /// ## Buffer::get_stride
    ///
    /// @returns size of each element in the buffer.
    [[nodiscard]]
    size_t get_stride() const {
        return stride_;
    }

    /// ## Buffer::get_count
    ///
    /// @returns number of elements in the buffer.
    [[nodiscard]]
    size_t get_count() const {
        return count_;
    }

    /// ## Buffer::bind
    ///
    /// Binds the OpenGL buffer.
    void bind() const;

private:
    Buffer(uint32_t buffer_id, uint32_t buffer_type, uint32_t usage, size_t stride, size_t count);
};


#endif // __BUFFER_H__
