// This is an open source non-commercial project. Dear PVS-Studio, please check
// it. PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include <GL/glew.h>

#include <cmath>
#include <iostream>
#include <vector>

#include <GL/freeglut.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "GLSLShader.hpp"
#include "Grid.hpp"

#define GL_CHECK_ERRORS assert(glGetError() == GL_NO_ERROR)

// Vertex struct with position and normal
struct Vertex {
  glm::vec3 pos;
  glm::vec3 normal;
};

struct Common {
  // screen size
  static constexpr int WIDTH = 1024;
  static constexpr int HEIGHT = 768;

  // shadowmap texture dimensions
  static constexpr int SHADOWMAP_WIDTH = 512;
  static constexpr int SHADOWMAP_HEIGHT = 512;

  // shadowmapping and flat shader
  GLSLShader shader, flatshader;

  // sphere vertex array and vertex buffer object IDs
  GLuint sphereVAOID;
  GLuint sphereVerticesVBO;
  GLuint sphereIndicesVBO;

  // cube vertex array and vertex buffer object IDs
  GLuint cubeVAOID;
  GLuint cubeVerticesVBO;
  GLuint cubeIndicesVBO;

  // plane vertex array and vertex buffer object IDs
  GLuint planeVAOID;
  GLuint planeVerticesVBO;
  GLuint planeIndicesVBO;

  // light direction line gizmo vertex array and vertex buffer object IDs
  GLuint lightVAOID;
  GLuint lightVerticesVBO;

  // projection and  modelview matrices
  glm::mat4 P = glm::mat4(1);
  glm::mat4 MV = glm::mat4(1);

  // camera transformation variables
  int state = 0, oldX = 0, oldY = 0;
  float rX = 25, rY = -40, dist = -10;

  // objectspace light position
  glm::vec3 lightPosOS = glm::vec3(0, 2, 0);

  // vertices and indices for sphere/cube
  std::vector<Vertex> vertices;
  std::vector<GLushort> indices;
  int totalSphereTriangles = 0;

  // spherical coorindates for light direction
  float theta = -7;
  float phi = -0.77f;
  float radius = 5;

  // shadow map texture ID
  GLuint shadowMapTexID;

  // FBO ID
  GLuint fboID;

  glm::mat4 MV_L; // light modelview matrix
  glm::mat4 P_L;  // light projection matrix
  glm::mat4 B;    // light bias matrix
  glm::mat4 BP;   // light bias and projection matrix combined
  glm::mat4 S;    // light's combined MVPB matrix
};
static Common *g_pCommon = nullptr;

// Add the given sphere indices to the indices vector
inline void push_indices(int sectors, int r, int s,
                         std::vector<GLushort> &indices) {
  int curRow = r * sectors;
  int nextRow = (r + 1) * sectors;

  indices.push_back(static_cast<GLushort>(curRow + s));
  indices.push_back(static_cast<GLushort>(nextRow + s));
  indices.push_back(static_cast<GLushort>(nextRow + (s + 1)));

  indices.push_back(static_cast<GLushort>(curRow + s));
  indices.push_back(static_cast<GLushort>(nextRow + (s + 1)));
  indices.push_back(static_cast<GLushort>(curRow + (s + 1)));
}

// Generates a sphere primitive with the given radius, slices and stacks
void CreateSphere(float radius, unsigned int slices, unsigned int stacks,
                  std::vector<Vertex> &vertices,
                  std::vector<GLushort> &indices) {
  const double R = 1.0 / (slices - 1);
  const double S = 1.0 / (stacks - 1);

  for (size_t r = 0; r < slices; ++r) {
    for (size_t s = 0; s < stacks; ++s) {
      const float y = static_cast<float>(sin(-M_PI_2 + M_PI * r * R));
      const float x =
          static_cast<float>(cos(2 * M_PI * s * S) * sin(M_PI * r * R));
      const float z =
          static_cast<float>(sin(2 * M_PI * s * S) * sin(M_PI * r * R));

      Vertex v;
      v.pos = glm::vec3(x, y, z) * radius;
      v.normal = glm::normalize(v.pos);
      vertices.push_back(v);
      push_indices(static_cast<int>(stacks), static_cast<int>(r),
                   static_cast<int>(s), indices);
    }
  }
}

// Generates a cube of the given size
void CreateCube(const float &size, std::vector<Vertex> &vertices,
                std::vector<GLushort> &indices) {
  float halfSize = size / 2.0f;
  glm::vec3 positions[8];
  positions[0] = glm::vec3(-halfSize, -halfSize, -halfSize);
  positions[1] = glm::vec3(halfSize, -halfSize, -halfSize);
  positions[2] = glm::vec3(halfSize, halfSize, -halfSize);
  positions[3] = glm::vec3(-halfSize, halfSize, -halfSize);
  positions[4] = glm::vec3(-halfSize, -halfSize, halfSize);
  positions[5] = glm::vec3(halfSize, -halfSize, halfSize);
  positions[6] = glm::vec3(halfSize, halfSize, halfSize);
  positions[7] = glm::vec3(-halfSize, halfSize, halfSize);

  glm::vec3 normals[6];
  normals[0] = glm::vec3(-1.0, 0.0, 0.0);
  normals[1] = glm::vec3(1.0, 0.0, 0.0);
  normals[2] = glm::vec3(0.0, 1.0, 0.0);
  normals[3] = glm::vec3(0.0, -1.0, 0.0);
  normals[4] = glm::vec3(0.0, 0.0, 1.0);
  normals[5] = glm::vec3(0.0, 0.0, -1.0);

  indices.resize(36);
  vertices.resize(36);

  // fill indices array
  GLushort *id = &indices[0];
  // left face
  *id++ = 7;
  *id++ = 3;
  *id++ = 4;
  *id++ = 3;
  *id++ = 0;
  *id++ = 4;

  // right face
  *id++ = 2;
  *id++ = 6;
  *id++ = 1;
  *id++ = 6;
  *id++ = 5;
  *id++ = 1;

  // top face
  *id++ = 7;
  *id++ = 6;
  *id++ = 3;
  *id++ = 6;
  *id++ = 2;
  *id++ = 3;
  // bottom face
  *id++ = 0;
  *id++ = 1;
  *id++ = 4;
  *id++ = 1;
  *id++ = 5;
  *id++ = 4;

  // front face
  *id++ = 6;
  *id++ = 4;
  *id++ = 5;
  *id++ = 6;
  *id++ = 7;
  *id++ = 4;
  // back face
  *id++ = 0;
  *id++ = 2;
  *id++ = 1;
  *id++ = 0;
  *id++ = 3;
  *id++ = 2;

  for (std::vector<Vertex>::size_type i = 0; i < 36; i++) {
    auto normal_index = i / 6;
    vertices[i].pos = positions[indices[i]];
    vertices[i].normal = normals[normal_index];
    indices[i] = static_cast<GLushort>(i);
  }
}

// Generates a plane of the given width and depth
void CreatePlane(const float width, const float depth,
                 std::vector<Vertex> &vertices,
                 std::vector<GLushort> &indices) {
  float halfW = width / 2.0f;
  float halfD = depth / 2.0f;

  indices.resize(6);
  vertices.resize(4);
  glm::vec3 normal = glm::vec3(0, 1, 0);

  vertices[0].pos = glm::vec3(-halfW, 0.01, -halfD);
  vertices[0].normal = normal;
  vertices[1].pos = glm::vec3(-halfW, 0.01, halfD);
  vertices[1].normal = normal;
  vertices[2].pos = glm::vec3(halfW, 0.01, halfD);
  vertices[2].normal = normal;
  vertices[3].pos = glm::vec3(halfW, 0.01, -halfD);
  vertices[3].normal = normal;

  // fill indices array
  indices[0] = 0;
  indices[1] = 1;
  indices[2] = 2;

  indices[3] = 0;
  indices[4] = 2;
  indices[5] = 3;
}

// Mouse click handler
void OnMouseDown(int button, int s, int x, int y) {
  if (s == GLUT_DOWN) {
    g_pCommon->oldX = x;
    g_pCommon->oldY = y;
  }

  if (button == GLUT_MIDDLE_BUTTON) {
    g_pCommon->state = 0;
  } else {
    g_pCommon->state = 1;
  }
}

// Mouse move handler
void OnMouseMove(int x, int y) {
  if (g_pCommon->state == 0) {
    g_pCommon->dist *= (1 + (y - g_pCommon->oldY) / 60.0f);
  } else if (g_pCommon->state == 2) {
    g_pCommon->theta += (g_pCommon->oldX - x) / 60.0f;
    g_pCommon->phi += (y - g_pCommon->oldY) / 60.0f;

    // calculate light direction vector
    g_pCommon->lightPosOS.x =
        std::cos(g_pCommon->theta) * std::sin(g_pCommon->phi);
    g_pCommon->lightPosOS.y = std::cos(g_pCommon->phi);
    g_pCommon->lightPosOS.z =
        std::sin(g_pCommon->theta) * std::sin(g_pCommon->phi);

    // update light's MV matrix
    g_pCommon->MV_L = glm::lookAt(g_pCommon->lightPosOS, glm::vec3(0, 0, 0),
                                  glm::vec3(0, 1, 0));
    g_pCommon->S = g_pCommon->BP * g_pCommon->MV_L;
  } else {
    g_pCommon->rY += (x - g_pCommon->oldX) / 5.0f;
    g_pCommon->rX += (y - g_pCommon->oldY) / 5.0f;
  }
  g_pCommon->oldX = x;
  g_pCommon->oldY = y;

  // call display function
  glutPostRedisplay();
}

// OpenGL initialization
void OnInit() {
  // Load the flat shader
  g_pCommon->flatshader.LoadFromFile(GL_VERTEX_SHADER,
                                     "shaders/flatshader.vert");
  g_pCommon->flatshader.LoadFromFile(GL_FRAGMENT_SHADER,
                                     "shaders/flatshader.frag");
  // compile and link shader
  g_pCommon->flatshader.CreateAndLinkProgram();
  g_pCommon->flatshader.Use();
  // add attributes and uniforms
  g_pCommon->flatshader.AddAttribute("vVertex");
  g_pCommon->flatshader.AddUniform("MVP");
  g_pCommon->flatshader.UnUse();

  // load the shadow mapping shader
  g_pCommon->shader.LoadFromFile(GL_VERTEX_SHADER,
                                 "shaders/PointLightShadowMapped.vert");
  g_pCommon->shader.LoadFromFile(GL_FRAGMENT_SHADER,
                                 "shaders/PointLightShadowMapped.frag");
  // compile and link shader
  g_pCommon->shader.CreateAndLinkProgram();
  g_pCommon->shader.Use();
  // add attributes and uniforms
  g_pCommon->shader.AddAttribute("vVertex");
  g_pCommon->shader.AddAttribute("vNormal");
  g_pCommon->shader.AddUniform("MVP");
  g_pCommon->shader.AddUniform("MV");
  g_pCommon->shader.AddUniform("M");
  g_pCommon->shader.AddUniform("N");
  g_pCommon->shader.AddUniform("S");
  g_pCommon->shader.AddUniform("light_position");
  g_pCommon->shader.AddUniform("diffuse_color");
  g_pCommon->shader.AddUniform("bIsLightPass");
  g_pCommon->shader.AddUniform("shadowMap");
  // pass value of constant uniforms at initialization
  glUniform1i(g_pCommon->shader("shadowMap"), 0);
  g_pCommon->shader.UnUse();

  GL_CHECK_ERRORS;

  // setup sphere geometry
  CreateSphere(1.0f, 10, 10, g_pCommon->vertices, g_pCommon->indices);

  // setup sphere vao and vbo stuff
  glGenVertexArrays(1, &g_pCommon->sphereVAOID);
  glGenBuffers(1, &g_pCommon->sphereVerticesVBO);
  glGenBuffers(1, &g_pCommon->sphereIndicesVBO);
  glBindVertexArray(g_pCommon->sphereVAOID);

  glBindBuffer(GL_ARRAY_BUFFER, g_pCommon->sphereVerticesVBO);
  // pass vertices to the buffer object
  glBufferData(GL_ARRAY_BUFFER, g_pCommon->vertices.size() * sizeof(Vertex),
               &g_pCommon->vertices[0], GL_STATIC_DRAW);
  GL_CHECK_ERRORS;
  // enable vertex attribute array for position
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), nullptr);
  GL_CHECK_ERRORS;
  // enable vertex attribute array for normal
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(
      1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
      reinterpret_cast<const GLvoid *>(offsetof(Vertex, normal)));
  GL_CHECK_ERRORS;
  // pass sphere indices to element array buffer
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_pCommon->sphereIndicesVBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER,
               g_pCommon->indices.size() * sizeof(GLushort),
               &g_pCommon->indices[0], GL_STATIC_DRAW);

  // store the total number of sphere triangles
  g_pCommon->totalSphereTriangles = static_cast<int>(g_pCommon->indices.size());

  // clear the vertices and indices vectors as we will reuse them
  // for cubes
  g_pCommon->vertices.clear();
  g_pCommon->indices.clear();

  // setup cube geometry
  CreateCube(2, g_pCommon->vertices, g_pCommon->indices);

  // setup cube vao and vbo stuff
  glGenVertexArrays(1, &g_pCommon->cubeVAOID);
  glGenBuffers(1, &g_pCommon->cubeVerticesVBO);
  glGenBuffers(1, &g_pCommon->cubeIndicesVBO);
  glBindVertexArray(g_pCommon->cubeVAOID);

  glBindBuffer(GL_ARRAY_BUFFER, g_pCommon->cubeVerticesVBO);
  // pass vertices to the buffer object
  glBufferData(GL_ARRAY_BUFFER, g_pCommon->vertices.size() * sizeof(Vertex),
               &g_pCommon->vertices[0], GL_STATIC_DRAW);
  GL_CHECK_ERRORS;
  // enable vertex attribute array for position
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), nullptr);
  GL_CHECK_ERRORS;
  // enable vertex attribute array for normals
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(
      1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
      reinterpret_cast<const GLvoid *>(offsetof(Vertex, normal)));
  GL_CHECK_ERRORS;
  // pass cube indices to element array buffer
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_pCommon->cubeIndicesVBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER,
               g_pCommon->indices.size() * sizeof(GLushort),
               &g_pCommon->indices[0], GL_STATIC_DRAW);

  GL_CHECK_ERRORS;

  // clear the vertices and indices vectors as we will reuse them
  // for plane
  g_pCommon->vertices.clear();
  g_pCommon->indices.clear();
  // create a plane object
  CreatePlane(100, 100, g_pCommon->vertices, g_pCommon->indices);

  // setup plane VAO and VBO
  glGenVertexArrays(1, &g_pCommon->planeVAOID);
  glGenBuffers(1, &g_pCommon->planeVerticesVBO);
  glGenBuffers(1, &g_pCommon->planeIndicesVBO);
  glBindVertexArray(g_pCommon->planeVAOID);

  glBindBuffer(GL_ARRAY_BUFFER, g_pCommon->planeVerticesVBO);
  // pass vertices to the buffer object
  glBufferData(GL_ARRAY_BUFFER, g_pCommon->vertices.size() * sizeof(Vertex),
               &g_pCommon->vertices[0], GL_STATIC_DRAW);
  GL_CHECK_ERRORS;
  // enable vertex attribute array for position
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), nullptr);
  GL_CHECK_ERRORS;
  // enable vertex attribute array for normals
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(
      1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
      reinterpret_cast<const GLvoid *>(offsetof(Vertex, normal)));
  GL_CHECK_ERRORS;
  // pass plane indices to element array buffer
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_pCommon->planeIndicesVBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER,
               g_pCommon->indices.size() * sizeof(GLushort),
               &g_pCommon->indices[0], GL_STATIC_DRAW);

  GL_CHECK_ERRORS;

  // setup vao and vbo stuff for the light position crosshair
  glm::vec3 crossHairVertices[6];
  crossHairVertices[0] = glm::vec3(-0.5f, 0, 0);
  crossHairVertices[1] = glm::vec3(0.5f, 0, 0);
  crossHairVertices[2] = glm::vec3(0, -0.5f, 0);
  crossHairVertices[3] = glm::vec3(0, 0.5f, 0);
  crossHairVertices[4] = glm::vec3(0, 0, -0.5f);
  crossHairVertices[5] = glm::vec3(0, 0, 0.5f);

  // setup light gizmo vertex array and buffer object
  glGenVertexArrays(1, &g_pCommon->lightVAOID);
  glGenBuffers(1, &g_pCommon->lightVerticesVBO);
  glBindVertexArray(g_pCommon->lightVAOID);

  glBindBuffer(GL_ARRAY_BUFFER, g_pCommon->lightVerticesVBO);
  // pass light crosshair gizmo vertices to buffer object
  glBufferData(GL_ARRAY_BUFFER, sizeof(crossHairVertices),
               &(crossHairVertices[0].x), GL_STATIC_DRAW);
  GL_CHECK_ERRORS;
  // enable vertex attribute array for position
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

  GL_CHECK_ERRORS;

  // get light position from spherical coordinates
  g_pCommon->lightPosOS.x =
      g_pCommon->radius * std::cos(g_pCommon->theta) * std::sin(g_pCommon->phi);
  g_pCommon->lightPosOS.y = g_pCommon->radius * std::cos(g_pCommon->phi);
  g_pCommon->lightPosOS.z =
      g_pCommon->radius * std::sin(g_pCommon->theta) * std::sin(g_pCommon->phi);

  // setup the shadowmap texture
  glGenTextures(1, &g_pCommon->shadowMapTexID);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, g_pCommon->shadowMapTexID);

  // set texture parameters
  GLfloat border[4] = {1, 0, 0, 0};
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE,
                  GL_COMPARE_REF_TO_TEXTURE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
  glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, Common::SHADOWMAP_WIDTH,
               Common::SHADOWMAP_HEIGHT, 0, GL_DEPTH_COMPONENT,
               GL_UNSIGNED_BYTE, nullptr);

  // set up FBO to get the depth component
  glGenFramebuffers(1, &g_pCommon->fboID);
  glBindFramebuffer(GL_FRAMEBUFFER, g_pCommon->fboID);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D,
                         g_pCommon->shadowMapTexID, 0);

  // Check framebuffer completeness status
  GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
  if (status == GL_FRAMEBUFFER_COMPLETE) {
    std::cout << "FBO setup successful." << std::endl;
  } else {
    std::cout << "Problem in FBO setup." << std::endl;
  }

  // unbind FBO
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  // set the light MV, P and bias matrices
  g_pCommon->MV_L = glm::lookAt(g_pCommon->lightPosOS, glm::vec3(0, 0, 0),
                                glm::vec3(0, 1, 0));
  g_pCommon->P_L = glm::perspective(50.0f, 1.0f, 1.0f, 25.0f);
  g_pCommon->B =
      glm::scale(glm::translate(glm::mat4(1), glm::vec3(0.5, 0.5, 0.5)),
                 glm::vec3(0.5, 0.5, 0.5));
  g_pCommon->BP = g_pCommon->B * g_pCommon->P_L;
  g_pCommon->S = g_pCommon->BP * g_pCommon->MV_L;

  // enable depth testing and culling
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);

  std::cout << "Initialization successfull" << std::endl;
}

// Release all allocated resources
void OnShutdown() {
  // Destroy shader
  g_pCommon->shader.DeleteShaderProgram();
  // Destroy vao and vbo
  glDeleteBuffers(1, &g_pCommon->sphereVerticesVBO);
  glDeleteBuffers(1, &g_pCommon->sphereIndicesVBO);
  glDeleteVertexArrays(1, &g_pCommon->sphereVAOID);

  glDeleteBuffers(1, &g_pCommon->cubeVerticesVBO);
  glDeleteBuffers(1, &g_pCommon->cubeIndicesVBO);
  glDeleteVertexArrays(1, &g_pCommon->cubeVAOID);

  glDeleteVertexArrays(1, &g_pCommon->lightVAOID);
  glDeleteBuffers(1, &g_pCommon->lightVerticesVBO);

  std::cout << "Shutdown successfull" << std::endl;
}

// Resize event handler
void OnResize(int w, int h) {
  // set the viewport
  glViewport(0, 0, static_cast<GLsizei>(w), static_cast<GLsizei>(h));

  // setup the projection matrix
  g_pCommon->P =
      glm::perspective(45.0f, static_cast<GLfloat>(w) / h, 0.1f, 1000.f);
}

// idle callback just calls the display function
void OnIdle() { glutPostRedisplay(); }

// Scene rendering function
void DrawScene(glm::mat4 View, glm::mat4 Proj, int isLightPass = 1) {
  GL_CHECK_ERRORS;

  // bind the current shader
  g_pCommon->shader.Use();
  // render plane first
  glBindVertexArray(g_pCommon->planeVAOID);
  {
    // set the shader uniforms
    glUniform3fv(g_pCommon->shader("light_position"), 1,
                 &(g_pCommon->lightPosOS.x));
    glUniformMatrix4fv(g_pCommon->shader("S"), 1, GL_FALSE,
                       glm::value_ptr(g_pCommon->S));
    glUniformMatrix4fv(g_pCommon->shader("M"), 1, GL_FALSE,
                       glm::value_ptr(glm::mat4(1)));
    glUniform1i(g_pCommon->shader("bIsLightPass"), isLightPass);
    glUniformMatrix4fv(g_pCommon->shader("MVP"), 1, GL_FALSE,
                       glm::value_ptr(Proj * View));
    glUniformMatrix4fv(g_pCommon->shader("MV"), 1, GL_FALSE,
                       glm::value_ptr(View));
    glUniformMatrix3fv(g_pCommon->shader("N"), 1, GL_FALSE,
                       glm::value_ptr(glm::inverseTranspose(glm::mat3(View))));
    glUniform3f(g_pCommon->shader("diffuse_color"), 1.0f, 1.0f, 1.0f);
    // draw plane triangles
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr);
  }

  // render the cube
  glBindVertexArray(g_pCommon->cubeVAOID);
  {
    // set the cube's transform
    glm::mat4 T = glm::translate(glm::mat4(1), glm::vec3(-1, 1, 0));
    glm::mat4 M = T;
    glm::mat4 MV = View * M;
    glm::mat4 MVP = Proj * MV;
    // pass shader uniforms
    glUniformMatrix4fv(g_pCommon->shader("S"), 1, GL_FALSE,
                       glm::value_ptr(g_pCommon->S));
    glUniformMatrix4fv(g_pCommon->shader("M"), 1, GL_FALSE, glm::value_ptr(M));
    glUniformMatrix4fv(g_pCommon->shader("MVP"), 1, GL_FALSE,
                       glm::value_ptr(MVP));
    glUniformMatrix4fv(g_pCommon->shader("MV"), 1, GL_FALSE,
                       glm::value_ptr(MV));
    glUniformMatrix3fv(g_pCommon->shader("N"), 1, GL_FALSE,
                       glm::value_ptr(glm::inverseTranspose(glm::mat3(MV))));
    glUniform3f(g_pCommon->shader("diffuse_color"), 1.0f, 0.0f, 0.0f);
    // draw cube triangles
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, nullptr);
  }
  // render the sphere
  glBindVertexArray(g_pCommon->sphereVAOID);
  {
    // set the sphere's transform
    glm::mat4 T = glm::translate(glm::mat4(1), glm::vec3(1, 1, 0));
    glm::mat4 M = T;
    glm::mat4 MV = View * M;
    glm::mat4 MVP = Proj * MV;
    // set the shader uniforms
    glUniformMatrix4fv(g_pCommon->shader("S"), 1, GL_FALSE,
                       glm::value_ptr(g_pCommon->S));
    glUniformMatrix4fv(g_pCommon->shader("M"), 1, GL_FALSE, glm::value_ptr(M));
    glUniformMatrix4fv(g_pCommon->shader("MVP"), 1, GL_FALSE,
                       glm::value_ptr(MVP));
    glUniformMatrix4fv(g_pCommon->shader("MV"), 1, GL_FALSE,
                       glm::value_ptr(MV));
    glUniformMatrix3fv(g_pCommon->shader("N"), 1, GL_FALSE,
                       glm::value_ptr(glm::inverseTranspose(glm::mat3(MV))));
    glUniform3f(g_pCommon->shader("diffuse_color"), 0.0f, 0.0f, 1.0f);
    // draw sphere triangles
    glDrawElements(GL_TRIANGLES, g_pCommon->totalSphereTriangles,
                   GL_UNSIGNED_SHORT, nullptr);
  }

  // unbind shader
  g_pCommon->shader.UnUse();

  GL_CHECK_ERRORS;
}

// display callback function
void OnRender() {
  GL_CHECK_ERRORS;

  // clear colour and depth buffers
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // set the camera transform
  glm::mat4 T =
      glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, g_pCommon->dist));
  glm::mat4 Rx = glm::rotate(T, g_pCommon->rX, glm::vec3(1.0f, 0.0f, 0.0f));
  glm::mat4 MV = glm::rotate(Rx, g_pCommon->rY, glm::vec3(0.0f, 1.0f, 0.0f));

  // 1) Render scene from the light's POV
  // enable rendering to FBO
  glBindFramebuffer(GL_FRAMEBUFFER, g_pCommon->fboID);
  // clear depth buffer
  glClear(GL_DEPTH_BUFFER_BIT);
  // reset viewport to the shadow map texture size
  glViewport(0, 0, Common::SHADOWMAP_WIDTH, Common::SHADOWMAP_HEIGHT);

  // enable front face culling
  glCullFace(GL_FRONT);
  // draw scene from the point of view of light
  DrawScene(g_pCommon->MV_L, g_pCommon->P_L);
  // enable back face culling
  glCullFace(GL_BACK);

  // restore normal rendering path
  // unbind FBO, set the default back buffer and reset the viewport to screen
  // size
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glDrawBuffer(GL_BACK_LEFT);
  glViewport(0, 0, Common::WIDTH, Common::HEIGHT);

  // 2) Render scene from point of view of eye
  DrawScene(MV, g_pCommon->P, 0);

  // bind light gizmo vertex array object
  glBindVertexArray(g_pCommon->lightVAOID);
  {
    // set the flat shader
    g_pCommon->flatshader.Use();
    // set the light's transform and render 3 lines
    auto T_ = glm::translate(glm::mat4(1), g_pCommon->lightPosOS);
    glUniformMatrix4fv(g_pCommon->flatshader("MVP"), 1, GL_FALSE,
                       glm::value_ptr(g_pCommon->P * MV * T_));
    glDrawArrays(GL_LINES, 0, 6);
    // unbind shader
    g_pCommon->flatshader.UnUse();
  }

  // unbind the vertex array object
  glBindVertexArray(0);

  // swap front and back buffers to show the rendered result
  glutSwapBuffers();
}

int main(int argc, char **argv) {
  Common common;
  g_pCommon = &common;
  // freeglut initialization
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
  glutInitContextVersion(3, 3);
  glutInitContextFlags(GLUT_CORE_PROFILE | GLUT_DEBUG);
  glutInitWindowSize(Common::WIDTH, Common::HEIGHT);
  glutCreateWindow("Directional Light - OpenGL 3.3");

  // initialize glew
  glewExperimental = GL_TRUE;
  GLenum err = glewInit();
  if (GLEW_OK != err) {
    std::cerr << "Error: " << glewGetErrorString(err) << std::endl;
  } else {
    if (GLEW_VERSION_3_3) {
      std::cout << "Driver supports OpenGL 3.3\nDetails:" << std::endl;
    }
  }
  err = glGetError(); // this is to ignore INVALID ENUM error 1282
  GL_CHECK_ERRORS;

  // output hardware information
  std::cout << "\tUsing GLEW " << glewGetString(GLEW_VERSION) << std::endl;
  std::cout << "\tVendor: " << glGetString(GL_VENDOR) << std::endl;
  std::cout << "\tRenderer: " << glGetString(GL_RENDERER) << std::endl;
  std::cout << "\tVersion: " << glGetString(GL_VERSION) << std::endl;
  std::cout << "\tGLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION)
            << std::endl;
  GL_CHECK_ERRORS;

  // OpenGL initialization
  OnInit();

  // callback hooks
  glutCloseFunc(OnShutdown);
  glutDisplayFunc(OnRender);
  glutReshapeFunc(OnResize);
  glutMouseFunc(OnMouseDown);
  glutMotionFunc(OnMouseMove);
  glutIdleFunc(OnIdle);

  // mainloop call
  glutMainLoop();

  return 0;
}
