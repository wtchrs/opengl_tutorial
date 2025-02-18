#ifndef __COMMON_H__
#define __COMMON_H__


#include <glad/glad.h>
#include <glfw/glfw3.h>
#include <optional>
#include <string>

/// # loadTextFile
///
/// Loads the content of a text file.
///
/// ## Parameters
/// - `filename`: The path to the text file to be loaded.
///
/// ## Returns
/// An `std::optional<std::string>` containing the content of the file if successful,
/// or `std::nullopt` if the file could not be loaded.
std::optional<std::string> loadTextFile(const std::string &filename);


#endif // __COMMON_H__
