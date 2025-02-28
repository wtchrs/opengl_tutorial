#include "glex/program.h"
#include <cassert>
#include <cstddef>
#include <glm/gtc/type_ptr.hpp>
#include <memory>
#include <spdlog/spdlog.h>
#include <vector>
#include "glex/common.h"

std::unique_ptr<Program> Program::create(const std::vector<std::shared_ptr<Shader>> &shaders) {
    const auto program_id = glCreateProgram();
    if (program_id == 0) {
        SPDLOG_ERROR("Failed to create shader program.");
    }
    auto program = std::unique_ptr<Program>{new Program{program_id}};
    if (!program->link(shaders)) {
        SPDLOG_ERROR("Failed to create shader program");
        return nullptr;
    }
    SPDLOG_INFO("Shader program has been created: {}", program->get());
    return std::move(program);
}

std::unique_ptr<Program>
Program::create(const std::string &vertex_shader_filename, const std::string &frag_shader_filename) {
    std::shared_ptr vertex = Shader::create_from_file(vertex_shader_filename, GL_VERTEX_SHADER);
    std::shared_ptr fragment = Shader::create_from_file(frag_shader_filename, GL_FRAGMENT_SHADER);
    if (!vertex || !fragment) {
        SPDLOG_ERROR("Failed to create shader program");
        return nullptr;
    }
    return create({vertex, fragment});
}

Program::Program(const uint32_t program)
    : program_{program} {}

Program::~Program() {
    if (program_) {
        SPDLOG_INFO("Delete shader program: {}", program_);
        glDeleteProgram(program_);
    }
}

bool Program::link(const std::vector<std::shared_ptr<Shader>> &shaders) const {
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
    const auto loc = glGetUniformLocation(program_, name.c_str());
    glUniform1i(loc, value);
}

void Program::set_uniform(const std::string &name, float value) const {
    const auto loc = glGetUniformLocation(program_, name.c_str());
    glUniform1f(loc, value);
}

void Program::set_uniform(const std::string &name, const glm::vec2 &value) const {
    const auto loc = glGetUniformLocation(program_, name.c_str());
    glUniform2fv(loc, 1, glm::value_ptr(value));
}

void Program::set_uniform(const std::string &name, const glm::vec3 &value) const {
    const auto loc = glGetUniformLocation(program_, name.c_str());
    glUniform3fv(loc, 1, glm::value_ptr(value));
}

void Program::set_uniform(const std::string &name, const glm::vec4 &value) const {
    const auto loc = glGetUniformLocation(program_, name.c_str());
    glUniform4fv(loc, 1, glm::value_ptr(value));
}

void Program::set_uniform(const std::string &name, const glm::mat4 &value) const {
    const auto loc = glGetUniformLocation(program_, name.c_str());
    glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(value));
}

void Program::set_texture(int slot, const Texture &texture) const {
    if (slot < 0 || slot >= 32) {
        SPDLOG_ERROR("Failed to set texture: slot must be between 0 and 31: received: {}", slot);
        return;
    }
    glActiveTexture(GL_TEXTURE0 + slot);
    texture.bind();
    // Provide texture slot numbers to uniform locations.
    // set_uniform(name, slot);
    SPDLOG_INFO("Texture has been set to slot GL_TEXTURE{}:", slot);
    SPDLOG_INFO(" Use this texture by calling `set_uniform(\"uniformName\", {})`", slot);
}
