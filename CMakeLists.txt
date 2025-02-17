cmake_minimum_required(VERSION 3.30)

set(PROJECT_NAME opengl_tutorial)
project(${PROJECT_NAME})

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENTION OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# vcpkg packages
find_package(spdlog CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)

set(WINDOW_NAME "OpenGL Example")
# set(WINDOW_WIDTH 640)
# set(WINDOW_HEIGHT 480)
set(WINDOW_WIDTH 960)
set(WINDOW_HEIGHT 540)

# targets
file(GLOB_RECURSE SOURCES
    src/main.cpp
    src/common.cpp
    src/shader.cpp
    src/program.cpp
)
add_executable(${PROJECT_NAME} ${SOURCES})
target_include_directories(${PROJECT_NAME} PUBLIC include)
target_link_libraries(${PROJECT_NAME} PRIVATE
    spdlog::spdlog
    glfw
    glad::glad
)
target_compile_definitions(${PROJECT_NAME} PUBLIC
    WINDOW_NAME="${WINDOW_NAME}"
    WINDOW_WIDTH=${WINDOW_WIDTH}
    WINDOW_HEIGHT=${WINDOW_HEIGHT}
)

