# ${CMAKE_SOURCE_DIR}/Module1/Chapter01/ImageLoader/CMakeLists.txt
add_executable(ImageLoader main.cpp GLSLShader.cpp)

target_link_libraries(
  ImageLoader
  PUBLIC fmt::fmt
         glm::glm
         OpenGL::GL
         glfw
         glbinding::glbinding
         options::options
         ${SOIL_LIBRARIES})

target_include_directories(ImageLoader SYSTEM PUBLIC ${Stb_INCLUDE_DIR} ${SOIL_INCLUDE_DIRS})

file(COPY ${CMAKE_CURRENT_LIST_DIR}/shaders ${CMAKE_CURRENT_LIST_DIR}/media
     DESTINATION ${PROJECT_BINARY_DIR}/bin/Module1/Chapter01/)
