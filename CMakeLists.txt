cmake_minimum_required(VERSION 3.24)
project(ShaderLab LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
include(FetchContent)
if(MSVC)
  add_compile_options(/utf-8)
endif()
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(glew-cmake_BUILD_SHARED OFF CACHE BOOL "" FORCE)
set(glew-cmake_BUILD_STATIC ON CACHE BOOL "" FORCE)
FetchContent_Declare(glfw GIT_REPOSITORY https://github.com/glfw/glfw.git GIT_TAG 3.4 GIT_SHALLOW TRUE)
FetchContent_Declare(glew GIT_REPOSITORY https://github.com/Perlmint/glew-cmake.git GIT_TAG glew-cmake-2.2.0 GIT_SHALLOW TRUE)
FetchContent_Declare(imgui GIT_REPOSITORY https://github.com/ocornut/imgui.git GIT_TAG v1.91.8 GIT_SHALLOW TRUE)
FetchContent_Declare(meshoptimizer GIT_REPOSITORY https://github.com/zeux/meshoptimizer.git GIT_TAG v0.22 GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(glfw glew imgui meshoptimizer)
find_package(OpenGL REQUIRED)
add_library(lab_ui STATIC
  ${imgui_SOURCE_DIR}/imgui.cpp ${imgui_SOURCE_DIR}/imgui_draw.cpp
  ${imgui_SOURCE_DIR}/imgui_tables.cpp ${imgui_SOURCE_DIR}/imgui_widgets.cpp
  ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
  ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp)
target_include_directories(lab_ui PUBLIC ${imgui_SOURCE_DIR} ${imgui_SOURCE_DIR}/backends)
target_link_libraries(lab_ui PUBLIC glfw OpenGL::GL)
add_executable(ShaderLab
  ShaderLab/main.cpp ShaderLab/Application.cpp ShaderLab/EditorUI.cpp
  ShaderLab/Renderer.cpp ShaderLab/ShaderLibrary.cpp ShaderLab/Camera.cpp
  ShaderLab/SceneSettings.cpp ShaderLab/ImageCapture.cpp ShaderLab/SmokeTests.cpp
  ShaderLab/VoxelGI.cpp ShaderLab/ClusterMesh.cpp ShaderLab/Light.cpp ShaderLab/LightEditor.cpp
  SimpleGame/LoadPng.cpp)
target_link_libraries(ShaderLab PRIVATE lab_ui libglew_static meshoptimizer)
target_compile_definitions(ShaderLab PRIVATE NOMINMAX)
if(MSVC)
  target_compile_options(ShaderLab PRIVATE /W4 /utf-8)
endif()
add_custom_target(ShaderLabAssets ALL
  COMMAND ${CMAKE_COMMAND} -E rm -f
  $<TARGET_FILE_DIR:ShaderLab>/Shaders/material.vert
  $<TARGET_FILE_DIR:ShaderLab>/Shaders/material.frag
  $<TARGET_FILE_DIR:ShaderLab>/Shaders/Lighting.vert
  $<TARGET_FILE_DIR:ShaderLab>/Shaders/Lighting.frag
  $<TARGET_FILE_DIR:ShaderLab>/Shaders/Toon.vert
  $<TARGET_FILE_DIR:ShaderLab>/Shaders/Toon.frag
  $<TARGET_FILE_DIR:ShaderLab>/Shaders/Fresnel.vert
  $<TARGET_FILE_DIR:ShaderLab>/Shaders/Fresnel.frag
  $<TARGET_FILE_DIR:ShaderLab>/Shaders/Dissolve.vert
  $<TARGET_FILE_DIR:ShaderLab>/Shaders/Dissolve.frag
  $<TARGET_FILE_DIR:ShaderLab>/Shaders/Wave.vert
  $<TARGET_FILE_DIR:ShaderLab>/Shaders/Wave.frag
  COMMAND ${CMAKE_COMMAND} -E copy_directory
  ${CMAKE_SOURCE_DIR}/ShaderLab/Shaders $<TARGET_FILE_DIR:ShaderLab>/Shaders
  COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:ShaderLab>/Licenses
  COMMAND ${CMAKE_COMMAND} -E copy_if_different ${meshoptimizer_SOURCE_DIR}/LICENSE.md
  $<TARGET_FILE_DIR:ShaderLab>/Licenses/meshoptimizer.txt)
add_dependencies(ShaderLab ShaderLabAssets)
set_property(TARGET ShaderLab PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:ShaderLab>")
enable_testing()
add_test(NAME ShaderLabSmoke COMMAND ShaderLab --smoke-test)
add_test(NAME ShaderLabUiSmoke COMMAND ShaderLab --ui-smoke-test)
add_test(NAME ShaderLabResearch COMMAND ShaderLab --research-test)
