#ifndef __BUFFER_H__
#define __BUFFER_H__


#include <cstddef>
#include <cstdint>
#include <memory>

class Buffer {
private:
    uint32_t buffer_{0};
    uint32_t buffer_type_{0};
    uint32_t usage_{0};

public:
    static std::unique_ptr<Buffer>
    create_with_data(uint32_t buffer_type, uint32_t usage, const void *data, size_t data_size);

    ~Buffer();

    uint32_t get() const {
        return buffer_;
    }

    void bind() const;

private:
    Buffer() {}
    bool init(uint32_t buffer_type, uint32_t usage, const void *data, size_t data_size);
};


#endif // __BUFFER_H__
