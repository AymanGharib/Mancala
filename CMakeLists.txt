cmake_minimum_required(VERSION 3.20)
project(Mancala3D VERSION 1.0.0 LANGUAGES C CXX)
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include(FetchContent)

find_package(OpenGL REQUIRED)

# GLFW
find_package(glfw3 3.3 QUIET)
if(NOT glfw3_FOUND)
    message(STATUS "GLFW not found, fetching from GitHub...")
    FetchContent_Declare(
        glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG 3.3.8
    )
    set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(glfw)
endif()

# GLM
find_package(glm QUIET)
if(NOT glm_FOUND)
    message(STATUS "GLM not found, fetching from GitHub...")
    FetchContent_Declare(
        glm
        GIT_REPOSITORY https://github.com/g-truc/glm.git
        GIT_TAG 0.9.9.8
    )
    FetchContent_MakeAvailable(glm)
endif()
if(TARGET glm AND NOT TARGET glm::glm)
    add_library(glm::glm ALIAS glm)
endif()

# GLAD (your repo has src/gl.c and include/glad/gl.h)
add_library(glad STATIC
    external/glad/src/gl.c
)
target_include_directories(glad PUBLIC
    external/glad/include
)

# stb_image (header only)
add_library(stb_image INTERFACE)
target_include_directories(stb_image INTERFACE
    external/stb
)

# Sources (match your folders)
file(GLOB_RECURSE SOURCES
    core/*.cpp core/*.h
    Rendering/*.cpp Rendering/*.h
    Scene/*.cpp Scene/*.h
    Game/*.cpp Game/*.h
)

add_executable(${PROJECT_NAME}
    main.cpp
    ${SOURCES}
)

target_include_directories(${PROJECT_NAME} PRIVATE
    .
    core Rendering Scene Game
    external/glad/include
    external/stb
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    OpenGL::GL
    glfw
    glad
    glm::glm
    stb_image
)
