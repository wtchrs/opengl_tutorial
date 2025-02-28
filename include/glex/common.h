#ifndef __COMMON_H__
#define __COMMON_H__


#include <glad/glad.h>
#include <glfw/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <optional>
#include <string>

/// ## loadTextFile
///
/// Loads the content of a text file.
///
/// @param filename: The path to the text file to be loaded.
///
/// @returns
/// `std::optional<std::string>` containing the content of the file if successful,
/// or `std::nullopt` if the file could not be loaded.
std::optional<std::string> load_text_file(const std::string &filename);

/// ## get_attenuation_coefficient
///
/// Calculates the attenuation coefficients for a given distance.
///
/// @param dist: The distance from the light source.
///
/// @returns `glm::vec3` containing the constant, linear, and quadratic attenuation coefficients.
///
/// #### Details
/// This function calculates the attenuation coefficients based on the given distance using predefined
/// linear and quadratic coefficients. The result is a vector containing the constant, linear, and quadratic
/// attenuation coefficients.
glm::vec3 get_attenuation_coefficient(float dist);


#endif // __COMMON_H__
