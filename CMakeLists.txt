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

# Sources (reliable)
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/core/*.cpp"
    "${CMAKE_SOURCE_DIR}/Rendering/*.cpp"
    "${CMAKE_SOURCE_DIR}/Scene/*.cpp"
    "${CMAKE_SOURCE_DIR}/Game/*.cpp"
)



FetchContent_Declare(
  imgui
  GIT_REPOSITORY https://github.com/ocornut/imgui.git
  GIT_TAG v1.91.0
)

FetchContent_GetProperties(imgui)
if(NOT imgui_POPULATED)
  FetchContent_Populate(imgui)
endif()

add_library(imgui STATIC
  ${imgui_SOURCE_DIR}/imgui.cpp
  ${imgui_SOURCE_DIR}/imgui_draw.cpp
  ${imgui_SOURCE_DIR}/imgui_tables.cpp
  ${imgui_SOURCE_DIR}/imgui_widgets.cpp

  ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
  ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)

target_include_directories(imgui PUBLIC
  ${imgui_SOURCE_DIR}
  ${imgui_SOURCE_DIR}/backends
)

# Tell ImGui OpenGL backend we use GLAD loader
target_compile_definitions(imgui PUBLIC IMGUI_IMPL_OPENGL_LOADER_GLAD)

# Link ImGui against GLFW + OpenGL (glad isn't required here, but ok if you want)
target_link_libraries(imgui PUBLIC glfw OpenGL::GL)












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
    imgui
)
