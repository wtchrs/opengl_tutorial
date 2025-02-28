#include "glex/common.h"
#include <fstream>
#include <optional>
#include <spdlog/spdlog.h>
#include <sstream>
#include <string>

std::optional<std::string> load_text_file(const std::string &filename) {
    std::ifstream is{filename};
    if (!is.is_open()) {
        SPDLOG_ERROR("Failed to open file: {}", filename);
        return std::nullopt;
    }
    std::stringstream ss;
    ss << is.rdbuf();
    return ss.str();
}

glm::vec3 get_attenuation_coefficient(const float dist) {
    constexpr auto linear_co = glm::vec4{8.4523112e-05, 4.4712582e+00, -1.8516388e+00, 3.3955811e+01};
    constexpr auto quad_co = glm::vec4{-7.6103583e-04, 9.0120201e+00, -1.1618500e+01, 1.0000464e+02};
    const float k_c = 1.0f;
    const float d = 1.0f / dist;
    const auto dvec = glm::vec4{1.0f, d, d * d, d * d * d};
    const float k_l = glm::dot(linear_co, dvec);
    const float k_q = glm::dot(quad_co, dvec);
    return {k_c, glm::max(k_l, 0.0f), glm::max(k_q * k_q, 0.0f)};
}
