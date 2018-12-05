project(gl3w C)

add_library(
	gl3w STATIC src/gl3w.c
)

target_include_directories(
	gl3w PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)