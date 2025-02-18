#ifndef __BUFFER_H__
#define __BUFFER_H__


#include <cstddef>
#include <cstdint>
#include <memory>

/// # Buffer
///
/// A class that encapsulates an OpenGL buffer object.
class Buffer {
    uint32_t buffer_{0};
    uint32_t buffer_type_{0};
    uint32_t usage_{0};

public:
    /// # Buffer::create_with_data
    ///
    /// Generates an OpenGL buffer and sets the provided data.
    ///
    /// ## Parameters
    /// - `buffer_type`: Buffer type to bind.
    /// - `usage`: Usage pattern of the data store.
    /// - `data`: Pointer to the data to be set in the generated buffer.
    /// - `data_size`: Size of the data in bytes.
    ///
    /// ## Returns
    /// A `Buffer` object wrapped in `std::unique_ptr`.
    ///
    /// ## Examples
    ///
    /// ```cpp
    /// float vertices[] = { /* ... */ };
    /// auto buffer = Buffer::create_with_data(GL_ARRAY_BUFFER, GL_STATIC_DRAW, vertices, sizeof(vertices));
    /// ```
    ///
    /// The above single line is equivalent to:
    ///
    /// ```cpp
    /// float vertices[] = { /* ... */ };
    /// uint32_t vertex_buffer;
    /// glGenBuffers(1, &vertex_buffer);
    /// glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    /// glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    ///
    /// // ...do something
    ///
    /// glDeleteBuffers(1, &vertex_buffer); // `glDeleteBuffers` function is automatically called in the destructor.
    /// ```
    static std::unique_ptr<Buffer>
    create_with_data(uint32_t buffer_type, uint32_t usage, const void *data, size_t data_size);

    /// # Buffer::~Buffer
    ///
    /// Destructor that deletes the OpenGL buffer. It calls `glDeleteBuffers` function to delete the buffer.
    ~Buffer();

    /// # Buffer::get
    ///
    /// Returns the OpenGL buffer ID.
    ///
    /// ## Returns
    /// The OpenGL buffer ID.
    [[nodiscard]]
    uint32_t get() const {
        return buffer_;
    }

    /// # Buffer::bind
    ///
    /// Binds the OpenGL buffer.
    void bind() const;

private:
    Buffer() {}
    bool init(uint32_t buffer_type, uint32_t usage, const void *data, size_t data_size);
};


#endif // __BUFFER_H__
