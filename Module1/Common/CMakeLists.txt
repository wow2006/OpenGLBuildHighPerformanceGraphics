# ${CMAKE_SOURCE_DIR}/Module1/Common/CMakeLists.txt
add_library(
  Common
  STATIC
  AbstractCamera.cpp
  FreeCamera.cpp
  GLSLShader.cpp
  Grid.cpp
  Plane.cpp
  Skybox.cpp
  RenderableObject.cpp
  TargetCamera.cpp
  TexturedPlane.cpp
  UnitCube.cpp
  UnitColorCube.cpp
  Quad.cpp
)

target_link_libraries(
  Common
  PUBLIC
  OpenGL::GL
  GLUT::GLUT
  GLEW::GLEW
  ${SOIL_LIBRARIES}
  options::options
)

target_include_directories(
  Common
  PUBLIC
  ${CMAKE_CURRENT_LIST_DIR}
)

target_include_directories(
  Common
  SYSTEM PUBLIC
  ${GLM_INCLUDE_DIR}
  ${SOIL_INCLUDE_DIRS}
)

