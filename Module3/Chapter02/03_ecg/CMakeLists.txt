# ${CMAKE_SOURCE_DIR}/Module3/Chapter02/03_ecg/CMakeLists.txt

add_executable(
  03_ecg
  main.cpp
  ECG_Data.cpp
)

target_link_libraries(
  03_ecg
  PUBLIC
  OpenGL::GL
  glfw
)

