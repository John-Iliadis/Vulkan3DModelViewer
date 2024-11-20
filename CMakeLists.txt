cmake_minimum_required(VERSION 3.28)
project(Vulkan3DModelViewer)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

add_executable(Vulkan3DModelViewer src/main.cpp
        src/vk/vulkan_types.hpp
        src/vk/debug.hpp
        src/vk/vulkan_functions.hpp
        src/vk/vulkan_functions.cpp
        src/vk/debug.cpp
        src/window.hpp
        src/window.cpp
        src/model/model.cpp
        src/model/model.hpp
        src/model/vertex.hpp
        src/model/mesh.cpp
        src/model/mesh.hpp)

set(DEPENDENCIES_DIR ${PROJECT_SOURCE_DIR}/dependencies)
set(GLFW_DIR ${DEPENDENCIES_DIR}/glfw)
set(GLM_DIR ${DEPENDENCIES_DIR}/glm)
set(STB_DIR ${DEPENDENCIES_DIR}/stb)
set(ASSIMP_DIR ${DEPENDENCIES_DIR}/assimp)

find_package(Vulkan REQUIRED)

if (NOT Vulkan_FOUND)
    message(FATAL_ERROR "Failed to find Vulkan")
endif ()

target_include_directories(${PROJECT_NAME} PRIVATE
    ${GLFW_DIR}/include
    ${GLM_DIR}/include
    ${STB_DIR}/include
    ${ASSIMP_DIR}/include
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    Vulkan::Vulkan
    ${GLFW_DIR}/lib/libglfw3.a
    ${ASSIMP_DIR}/lib/libassimp.a
    ${ASSIMP_DIR}/lib/libzlibstatic.a
)

target_compile_definitions(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:Debug>:DEBUG_MODE>
    GLFW_INCLUDE_VULKAN
    STB_IMAGE_IMPLEMENTATION
    GLM_FORCE_DEPTH_ZERO_TO_ONE
)

set_target_properties(${PROJECT_NAME} PROPERTIES DEBUG_POSTFIX -d)
