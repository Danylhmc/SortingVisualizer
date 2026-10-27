cmake_minimum_required(VERSION 3.20)

project(SortingVisualizer)

file(GLOB_RECURSE CONFIGURE_DEPENDS SOURCES src/*.cpp)

add_executable(SortingVisualizer
    src/main.cpp
    src/shader.cpp
    src/cube.cpp
    src/benchMark.cpp
    src/csv_reader.cpp
    src/graph.cpp
    external/glad/src/glad.c
)

target_include_directories(SortingVisualizer
  PRIVATE
    include
    external/glad/include
)

target_compile_features(SortingVisualizer
  PRIVATE
    cxx_std_20
)

target_compile_definitions(SortingVisualizer
  PRIVATE
    SORTING_VISUALIZER_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
)

find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)

target_link_libraries(SortingVisualizer
  PRIVATE
  OpenGL::GL
    glfw
    glm::glm
)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
