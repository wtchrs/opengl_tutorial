#include "glex/shader.h"
#include <cstddef>
#include <cstdint>
#include <memory>
#include <spdlog/spdlog.h>
#include "glex/common.h"

std::unique_ptr<Shader> Shader::create_from_file(const std::string &filename, GLenum shader_type) {
    auto shader = std::unique_ptr<Shader>{new Shader{}};
    if (!shader->load_file(filename, shader_type)) {
        SPDLOG_ERROR("Failed to create shader.");
        return nullptr;
    }
    SPDLOG_INFO("Shader has been created: {}", shader->get());
    return std::move(shader);
}

bool Shader::load_file(const std::string &filename, GLenum shader_type) {
    auto code = loadTextFile(filename);
    if (!code) {
        return false;
    }

    const char *code_ptr = code->c_str();
    int32_t code_length = static_cast<int32_t>(code->length());

    // Create shader and compile.
    shader_ = glCreateShader(shader_type);
    glShaderSource(shader_, 1, &code_ptr, &code_length);
    glad_glCompileShader(shader_);

    // Check if compilation is successful.
    int success = 0;
    glGetShaderiv(shader_, GL_COMPILE_STATUS, &success);
    if (!success) {
        constexpr size_t LOG_SIZE = 1024;
        char info_log[LOG_SIZE];
        glad_glGetShaderInfoLog(shader_, LOG_SIZE, nullptr, info_log);
        SPDLOG_ERROR("Failed to compile shader: \"{}\"", filename);
        SPDLOG_ERROR("reason: {}", info_log);
        return false;
    }

    return true;
}

Shader::~Shader() {
    if (shader_) {
        glDeleteShader(shader_);
    }
}
