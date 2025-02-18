#ifndef __CONTEXT_H__
#define __CONTEXT_H__


#include <cstdint>
#include <memory>
#include "glex/buffer.h"
#include "program.h"

class Context {
private:
    std::unique_ptr<Program> program_;

    /** VBO, Vertex Buffer Object */
    // uint32_t vertex_buffer_{0};
    std::unique_ptr<Buffer> vertex_buffer_;
    /** VAO, Vertex Array Object */
    uint32_t vertex_array_object_{0};
    /** EBO, Element Buffer Object */
    std::unique_ptr<Buffer> index_buffer_;

public:
    static std::unique_ptr<Context> create();
    void render();

private:
    Context() {};
    bool init();
};


#endif // __CONTEXT_H__
