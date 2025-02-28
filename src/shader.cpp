#include "glex/shader.h"
#include <cstddef>
#include <cstdint>
#include <memory>
#include <spdlog/spdlog.h>
#include "glex/common.h"

std::unique_ptr<Shader> Shader::create_from_file(const std::string &filename, const GLenum shader_type) {
    const auto shader_id = glCreateShader(shader_type);
    if (shader_id == 0) {
        SPDLOG_ERROR("Failed to create shader");
        return nullptr;
    }
    auto shader = std::unique_ptr<Shader>{new Shader{shader_id}};
    if (!shader->load_file(filename)) {
        SPDLOG_ERROR("Failed to create shader");
        return nullptr;
    }
    SPDLOG_INFO("Shader has been created: \"{}\", id: {}", filename, shader->get());
    return std::move(shader);
}

bool Shader::load_file(const std::string &filename) const {
    const auto code = load_text_file(filename);
    if (!code) {
        return false;
    }

    const char *code_ptr = code->c_str();
    const auto code_length = static_cast<int32_t>(code->length());

    // Compile shader.
    glShaderSource(shader_, 1, &code_ptr, &code_length);
    glCompileShader(shader_);

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

Shader::Shader(const uint32_t shader)
    : shader_{shader} {}

Shader::~Shader() {
    if (shader_) {
        SPDLOG_INFO("Delete shader: {}", shader_);
        glDeleteShader(shader_);
    }
}
