#pragma once
// STL
#include <string>
#include <vector>
// GLM
#include <glm/glm.hpp>

using namespace std;

struct Vertex {
  glm::vec3 pos, normal;
  glm::vec2 uv;
};

struct Face {
  unsigned short a, b, c, // pos indices
                 d, e, f, // normal indices
                 g, h, i; // uv indices
};

class Mesh {
public:
  string name;
  int material_index = -1;
};

class Material {
public:
  float ambient[3];
  float diffuse[3];
  float specular[3];
  float Tf[3];
  int illum;
  float Ka[3];
  float Kd[3];
  float Ks[3];
  float Ke[3];
  std::string map_Ka, map_Kd, name;
  float Ns, Ni, d, Tr;
  vector<unsigned short> sub_indices;
  int offset;
  int count;
};

class ObjLoader {
public:
  bool Load(const string &filename, vector<Mesh *> &meshes,
            vector<Vertex> &verts, vector<unsigned short> &inds,
            vector<Material *> &materials);
};
