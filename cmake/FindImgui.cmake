# ${CMAKE_SOURCE_DIR}/cmake/FindImgui.cmake
if(TARGET Imgui::SDL_OpenGL)
  return()
endif()

set(IMGUI_DIR ${CMAKE_SOURCE_DIR}/thirdparty/imgui/)
if(NOT EXISTS ${CMAKE_SOURCE_DIR}/thirdparty/imgui/)
  message(FATAL_ERROR "Please run `git submodule update --init`")
endif()

# ========================
# IMGUI Core
# ========================
add_library(
  Imgui_core
  STATIC
)

target_sources(
  Imgui_core
  PRIVATE
  ${IMGUI_DIR}/imgui.cpp
  ${IMGUI_DIR}/imgui_draw.cpp
  ${IMGUI_DIR}/imgui_demo.cpp
  ${IMGUI_DIR}/imgui_tables.cpp
  ${IMGUI_DIR}/imgui_widgets.cpp
)

target_include_directories(
  Imgui_core
  SYSTEM PUBLIC
  ${IMGUI_DIR}
)

add_library(
  Imgui::core
  ALIAS
  Imgui_core
)

# ========================
# IMGUI GLFW
# ========================
if(NOT TARGET GLFW::glfw3)
  find_package(GLFW REQUIRED)
endif()

if(NOT TARGET GLEW::GLEW)
  find_package(GLEW REQUIRED)
endif()

add_library(
  Imgui_glfw
  STATIC
)

add_library(
  Imgui::GLFW
  ALIAS
  Imgui_glfw
)

target_sources(
  Imgui_glfw
  PRIVATE
  ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
)

target_include_directories(
  Imgui_glfw
  SYSTEM PUBLIC
  ${IMGUI_DIR}/examples/
)

target_link_libraries(
  Imgui_glfw
  PUBLIC
  Imgui::core
  GLFW::glfw3
)

# ========================
# IMGUI OpenGL
# ========================
if(NOT TARGET OpenGL::GL)
  set(OpenGL_GL_PREFERENCE GLVND)
  find_package(OpenGL REQUIRED)
endif()

add_library(
  Imgui_GLFW_opengl
  STATIC
)

add_library(
  Imgui::GLFW_OpenGL
  ALIAS
  Imgui_GLFW_opengl
)

target_sources(
  Imgui_GLFW_opengl
  PRIVATE
  ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)

target_include_directories(
  Imgui_GLFW_opengl
  SYSTEM PUBLIC
  ${IMGUI_DIR}/backends/
)

target_link_libraries(
  Imgui_GLFW_opengl
  PUBLIC
  Imgui::GLFW
  OpenGL::GL
)

