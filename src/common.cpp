#include "glex/common.h"
#include <fstream>
#include <optional>
#include <spdlog/spdlog.h>
#include <sstream>
#include <string>

std::optional<std::string> loadTextFile(const std::string &filename) {
    std::ifstream is{filename};
    if (!is.is_open()) {
        SPDLOG_ERROR("Failed to open file: {}", filename);
        return std::nullopt;
    }
    std::stringstream ss;
    ss << is.rdbuf();
    return ss.str();
}
