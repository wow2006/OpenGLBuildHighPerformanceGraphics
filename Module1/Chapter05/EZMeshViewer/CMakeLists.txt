# ${CMAKE_SOURCE_DIR}/Module1/Chapter05/EZMeshViewer/CMakeLists.txt
add_executable(
  EZMeshViewer
  main.cpp
  Ezm.cpp
  3rdParty/pugi_xml/pugixml.cpp
  MeshImport.cpp)

target_link_libraries(EZMeshViewer PUBLIC Common)
