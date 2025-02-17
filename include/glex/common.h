#ifndef __COMMON_H__
#define __COMMON_H__


#include <glad/glad.h>
#include <glfw/glfw3.h>
#include <optional>
#include <string>

std::optional<std::string> loadTextFile(const std::string &filename);


#endif // __COMMON_H__
