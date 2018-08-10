add_library(Common STATIC
  AbstractCamera.cpp
  FreeCamera.cpp
  GLSLShader.cpp
  Grid.cpp
  Plane.cpp
  RenderableObject.cpp
  TargetCamera.cpp
  TexturedPlane.cpp
  UnitCube.cpp)
target_link_libraries(Common
  PUBLIC
  OpenGL::GL
  GLUT::GLUT
  GLEW::GLEW
  ${SOIL_LIBRARIES})
target_include_directories(Common
  PUBLIC
  ${CMAKE_CURRENT_LIST_DIR}
  ${GLM_INCLUDE_DIR}
  ${SOIL_INCLUDE_DIRS})
target_compile_features(Common
  PUBLIC
  cxx_std_14)
target_compile_options(Common
  PUBLIC
  $<$<CXX_COMPILER_ID:Clang>:-Wall -Weverything -Wno-c++98-compat -Wno-c++98-compat-pedantic -Werror -Wno-missing-prototypes -Wno-padded>
  $<$<CXX_COMPILER_ID:GNU>:-pedantic -Wall -Wextra -Werror>
  $<$<CXX_COMPILER_ID:MSVC>:/W4>)
