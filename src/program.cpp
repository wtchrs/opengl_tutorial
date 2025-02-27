#include "glex/program.h"
#include <cassert>
#include <cstddef>
#include <glm/gtc/type_ptr.hpp>
#include <memory>
#include <spdlog/spdlog.h>
#include <vector>
#include "glex/common.h"

#define CONCAT(x1, x2) x1##(x2)

std::unique_ptr<Program> Program::create(const std::vector<std::shared_ptr<Shader>> &shaders) {
    auto program = std::unique_ptr<Program>{new Program{}};
    if (!program->link(shaders)) {
        SPDLOG_ERROR("Failed to create pipeline program.");
        return nullptr;
    }
    SPDLOG_INFO("Pipeline program has been created: {}", program->get());
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

void Program::set_uniform(const std::string &name, int value) const {
    auto loc = glGetUniformLocation(program_, name.c_str());
    glUniform1i(loc, value);
}

void Program::set_uniform(const std::string &name, float value) const {
    auto loc = glGetUniformLocation(program_, name.c_str());
    glUniform1f(loc, value);
}

void Program::set_uniform(const std::string &name, const glm::vec3 &value) const {
    auto loc = glGetUniformLocation(program_, name.c_str());
    glUniform3fv(loc, 1, glm::value_ptr(value));
}

void Program::set_uniform(const std::string &name, const glm::mat4 &value) const {
    auto loc = glGetUniformLocation(program_, name.c_str());
    glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(value));
}

void Program::set_texture(const std::string &name, int slot, const Texture &texture) const {
    if (slot < 0 || slot >= 32) {
        SPDLOG_ERROR("Failed to set texture: slot must be between 0 and 31: received: {}", slot);
        return;
    }
    glActiveTexture(GL_TEXTURE0 + slot);
    texture.bind();
    // Provide texture slot numbers to uniform locations.
    set_uniform(name, slot);
    SPDLOG_INFO("Texture has been set to slot GL_TEXTURE{}", slot);
}
