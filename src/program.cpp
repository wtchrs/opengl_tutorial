#include "glex/program.h"
#include <cstddef>
#include <memory>
#include <spdlog/spdlog.h>
#include <vector>
#include "glex/common.h"

std::unique_ptr<Program> Program::create(const std::vector<std::shared_ptr<Shader>> &shaders) {
    auto program = std::unique_ptr<Program>{new Program{}};
    if (!program->link(shaders)) {
        return nullptr;
    }
    return std::move(program);
}

Program::~Program() {
    if (program_) {
        glDeleteProgram(program_);
    }
}

bool Program::link(const std::vector<std::shared_ptr<Shader>> &shaders) {
    program_ = glCreateProgram();
    // Attach shaders into program.
    for (auto &shader : shaders) {
        glAttachShader(program_, shader->get());
    }
    // Link program.
    glLinkProgram(program_);

    // Check if linking is successful.
    int success = 0;
    glGetProgramiv(program_, GL_LINK_STATUS, &success);
    if (!success) {
        constexpr size_t LOG_SIZE = 1024;
        char info_log[LOG_SIZE];
        glGetProgramInfoLog(program_, LOG_SIZE, nullptr, info_log);
        SPDLOG_ERROR("Failed to link program: {}", info_log);
        return false;
    }

    return true;
}

void Program::use() const {
    glUseProgram(program_);
}
