set(OpenGL_GL_PREFERENCE GLVND)
find_package(OpenGL REQUIRED)
find_package(SDL2   REQUIRED)
if(NOT TARGET SDL2::SDL2)
  add_library(SDL2::SDL2 INTERFACE IMPORTED)

  set_target_properties(SDL2::SDL2 PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${SDL2_INCLUDE_DIRS}"
  )

  set_target_properties(SDL2::SDL2 PROPERTIES
    INTERFACE_LINK_LIBRARIES "${SDL2_LIBRARIES}"
  )

  add_library(SDL2::SDL2main INTERFACE IMPORTED)

  set_target_properties(SDL2::SDL2main PROPERTIES
    INTERFACE_LINK_LIBRARIES "-lSDL2main"
  )
endif()
find_package(GLM    REQUIRED)
#find_package(SOIL   REQUIRED)

#add_subdirectory(Common)
add_subdirectory(Chapter01)
#add_subdirectory(Chapter02)
#add_subdirectory(Chapter03)
#add_subdirectory(Chapter04)
#add_subdirectory(Chapter05)
#add_subdirectory(Chapter06)
