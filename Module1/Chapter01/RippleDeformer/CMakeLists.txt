add_executable(RippleDeformer main.cpp)

target_link_libraries(
  RippleDeformer
  PUBLIC fmt::fmt
         glm::glm
         OpenGL::GL
         glfw
         glbinding::glbinding
         options::options
         GLSLShader)

file(COPY ${CMAKE_CURRENT_LIST_DIR}/shaders DESTINATION ${PROJECT_BINARY_DIR}/bin/Module1/Chapter01/)
