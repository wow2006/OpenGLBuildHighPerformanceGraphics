#pragma once
// STL
#include <map>
#include <string>
#include <vector>
// GLM
#include <glm/glm.hpp>

struct Vertex {
  glm::vec3 pos, normal;
  glm::vec2 uv;
};

struct Face {
  unsigned short a, b, c, // pos indices
      d, e, f,            // normal indices
      g, h, i;            // uv indices
};

struct SubMesh {
  const char *materialName;
  std::vector<unsigned int> indices;
};

class EzmLoader {
public:
  bool Load(const std::string &filename, std::vector<SubMesh> &meshes,
            std::vector<Vertex> &verts, std::vector<unsigned short> &inds,
            std::map<std::string, std::string> &materialNames, glm::vec3 &min,
            glm::vec3 &max);
};
