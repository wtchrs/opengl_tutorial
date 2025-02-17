#ifndef __PROGRAM_H__
#define __PROGRAM_H__


#include <cstdint>
#include <memory>
#include <vector>
#include "shader.h"

class Program {
private:
    uint32_t program_{0};

public:
    static std::unique_ptr<Program> create(const std::vector<std::shared_ptr<Shader>> &shaders);

    ~Program();
    uint32_t get() const {
        return program_;
    }

private:
    Program() {}
    bool link(const std::vector<std::shared_ptr<Shader>> &shaders);
};


#endif // __PROGRAM_H__
