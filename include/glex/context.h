#ifndef __CONTEXT_H__
#define __CONTEXT_H__


#include <cstdint>
#include <memory>
#include "program.h"

class Context {
private:
    std::unique_ptr<Program> program_;

    /** VBO, Vertex Buffer Object */
    uint32_t vertex_buffer_{0};
    /** VAO, Vertex Array Object */
    uint32_t vertex_array_object_{0};
    /** EBO, Element Buffer Object */
    uint32_t index_buffer_{0};

public:
    static std::unique_ptr<Context> create();
    void render();

private:
    Context() {};
    bool init();
};


#endif // __CONTEXT_H__
