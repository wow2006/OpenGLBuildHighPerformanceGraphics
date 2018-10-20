#include <GL/glew.h>

#include <iostream>
#include <vector>
#include <cmath>

#include <GL/freeglut.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Grid.hpp"
#include "GLSLShader.hpp"

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

  // per-vertex lighting shader
  GLSLShader shader;

  // sphere vertex array and vertex buffer object IDs
  GLuint sphereVAOID;
  GLuint sphereVerticesVBO;
  GLuint sphereIndicesVBO;

  // distance of cubes from the origin
  float radius = 2.f;

  // cube vertex array and vertex buffer object IDs
  GLuint cubeVAOID;
  GLuint cubeVerticesVBO;
  GLuint cubeIndicesVBO;

  // projection, modelview and rotation matrices
  glm::mat4 P  = glm::mat4(1);
  glm::mat4 MV = glm::mat4(1);
  glm::mat4 Rot;

  // camera transformation variables
  int state = 0, oldX = 0, oldY = 0;
  float rX = 56.f, rY = -44.f, dist = -5.f;

  // Grid object
  CGrid *grid = nullptr;

  glm::vec3 lightPosOS = glm::vec3(0, 2, 0); // objectspace light position

  // for animation of the cubes
  float dx = -0.1f;

  // vertices and indices for sphere/cube
  std::vector<Vertex>   vertices;
  std::vector<GLushort> indices;
  int totalSphereTriangles = 0;

  // constant colours for cubes
  glm::vec3 colors[8] = {
    glm::vec3(1, 0, 0), glm::vec3(0, 1, 0),
    glm::vec3(0, 0, 1), glm::vec3(1, 1, 0),
    glm::vec3(1, 0, 1), glm::vec3(0, 1, 1),
    glm::vec3(1, 1, 1), glm::vec3(0.5, 0.5, 0.5)
  };
};
static Common *g_pCommon = nullptr;

// add the given sphere indices to the indices vector
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
  (void)radius;
  (void)slices;
  (void)stacks;
  (void)vertices;
  (void)indices;
  //float const R = 1.0f / (float)(slices - 1);
  //float const S = 1.0f / (float)(stacks - 1);

  //for (size_t r = 0; r < slices; ++r) {
  //  for (size_t s = 0; s < stacks; ++s) {
  //    float const y = (float)(sin(-M_PI_2 + M_PI * r * R));
  //    float const x = (float)(cos(2 * M_PI * s * S) * sin(M_PI * r * R));
  //    float const z = (float)(sin(2 * M_PI * s * S) * sin(M_PI * r * R));

  //    Vertex v;
  //    v.pos = glm::vec3(x, y, z) * radius;
  //    v.normal = glm::normalize(v.pos);
  //    vertices.push_back(v);
  //    push_indices(stacks, r, s, indices);
  //  }
  //}
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
  } else {
    g_pCommon->rY += (x - g_pCommon->oldX) / 5.0f;
    g_pCommon->rX += (y - g_pCommon->oldY) / 5.0f;
  }
  g_pCommon->oldX = x;
  g_pCommon->oldY = y;

  glutPostRedisplay();
}

// OpenGL initialization
void OnInit() {
  // load the per-vertex lighting shader
  g_pCommon->shader.LoadFromFile(GL_VERTEX_SHADER,   "shaders/perVertexLighting.vert");
  g_pCommon->shader.LoadFromFile(GL_FRAGMENT_SHADER, "shaders/perVertexLighting.frag");
  // compile and link shader
  g_pCommon->shader.CreateAndLinkProgram();
  g_pCommon->shader.Use();
  // add attributes and uniforms
  g_pCommon->shader.AddAttribute("vVertex");
  g_pCommon->shader.AddAttribute("vNormal");
  g_pCommon->shader.AddUniform("MVP");
  g_pCommon->shader.AddUniform("MV");
  g_pCommon->shader.AddUniform("N");
  g_pCommon->shader.AddUniform("light_position");
  g_pCommon->shader.AddUniform("diffuse_color");
  g_pCommon->shader.AddUniform("specular_color");
  g_pCommon->shader.AddUniform("shininess");
  g_pCommon->shader.UnUse();
  GL_CHECK_ERRORS;

  // Cerate sphere geometry
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
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
      reinterpret_cast<const GLvoid *>(offsetof(Vertex, normal)));
  GL_CHECK_ERRORS;

  // pass sphere indices to element array buffer
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_pCommon->sphereIndicesVBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, g_pCommon->indices.size() * sizeof(GLushort),
               &g_pCommon->indices[0], GL_STATIC_DRAW);

  // store the total number of sphere triangles
  g_pCommon->totalSphereTriangles = static_cast<int>(g_pCommon->indices.size());

  // clear the vertices and indices vectors as we will reuse them
  // for cubes
  g_pCommon->vertices.clear();
  g_pCommon->indices.clear();

  // setup cube geometry
  CreateCube(1, g_pCommon->vertices, g_pCommon->indices);

  // setup cube vao and vbo stuff
  glGenVertexArrays(1, &g_pCommon->cubeVAOID);
  glGenBuffers(1,      &g_pCommon->cubeVerticesVBO);
  glGenBuffers(1,      &g_pCommon->cubeIndicesVBO);
  glBindVertexArray(g_pCommon->cubeVAOID);

  glBindBuffer(GL_ARRAY_BUFFER, g_pCommon->cubeVerticesVBO);
  // pass vertices to the buffer object
  glBufferData(GL_ARRAY_BUFFER, g_pCommon->vertices.size() * sizeof(Vertex),
               &g_pCommon->vertices[0], GL_STATIC_DRAW);
  GL_CHECK_ERRORS;

  // enable vertex attribut array for position
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), nullptr);
  GL_CHECK_ERRORS;

  // enable vertex attribut array for normal
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
      reinterpret_cast<const GLvoid *>(offsetof(Vertex, normal)));
  GL_CHECK_ERRORS;

  // pass cube indices to element array buffer
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_pCommon->cubeIndicesVBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, g_pCommon->indices.size() * sizeof(GLushort),
      &g_pCommon->indices[0], GL_STATIC_DRAW);
  GL_CHECK_ERRORS;

  // enable depth testing and culling
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);

  // create a grid of 10x10 size in XZ plane
  g_pCommon->grid = new CGrid();

  GL_CHECK_ERRORS;

  std::cout << "Initialization successfull" << std::endl;
}

// release all allocated resources
void OnShutdown() {
  // Destroy shader
  g_pCommon->shader.DeleteShaderProgram();
  // Destroy vao and vbo
  glDeleteBuffers(1,      &g_pCommon->sphereVerticesVBO);
  glDeleteBuffers(1,      &g_pCommon->sphereIndicesVBO);
  glDeleteVertexArrays(1, &g_pCommon->sphereVAOID);

  glDeleteBuffers(1,      &g_pCommon->cubeVerticesVBO);
  glDeleteBuffers(1,      &g_pCommon->cubeIndicesVBO);
  glDeleteVertexArrays(1, &g_pCommon->cubeVAOID);

  delete g_pCommon->grid;
  g_pCommon->grid = nullptr;
  std::cout << "Shutdown successfull" << std::endl;
}

// Resize event handler
void OnResize(int w, int h) {
  // set the viewport
  glViewport(0, 0, static_cast<GLsizei>(w), static_cast<GLsizei>(h));

  // setup the projection matrix
  g_pCommon->P = glm::perspective(45.0f, static_cast<GLfloat>(w) / h,
                                  0.1f, 1000.f);
}

// idle callback just calls the display function
void OnIdle() { glutPostRedisplay(); }

// scene rendering function
void DrawScene(glm::mat4 View, glm::mat4 Proj) {
  GL_CHECK_ERRORS;

  // Bind the current shader
  g_pCommon->shader.Use();

  // bind the cube vertex array object
  glBindVertexArray(g_pCommon->cubeVAOID);

  // draw the 8 cubes first
  for (int i = 0; i < 8; i++) {
    // set the cube's transform
    const float theta = i / 8.0f * 2.f * static_cast<float>(M_PI);
    glm::mat4 T = glm::translate(
        glm::mat4(1), glm::vec3(g_pCommon->radius * std::cos(theta),
                                0.5f,
                                g_pCommon->radius * std::sin(theta)));
    glm::mat4 M   = T;         // Model matrix
    glm::mat4 MV  = View * M;  // ModelView matrix
    glm::mat4 MVP = Proj * MV; // combined ModelView  Projection matrix

    // pass shader uniforms
    glUniformMatrix4fv(g_pCommon->shader("MVP"), 1, GL_FALSE, glm::value_ptr(MVP));
    glUniformMatrix4fv(g_pCommon->shader("MV"), 1, GL_FALSE, glm::value_ptr(MV));
    glUniformMatrix3fv(g_pCommon->shader("N"), 1, GL_FALSE,
        glm::value_ptr(glm::inverseTranspose(glm::mat3(MV))));
    glUniform3fv(g_pCommon->shader("diffuse_color"), 1, &(g_pCommon->colors[i].x));
    glUniform3f(g_pCommon->shader("specular_color"), 1.0f, 1.0f, 1.0f);
    glUniform1f(g_pCommon->shader("shininess"), 100);
    glUniform3fv(g_pCommon->shader("light_position"), 1, &(g_pCommon->lightPosOS.x));
    // draw triangles
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, nullptr);
    GL_CHECK_ERRORS;
  }

  // bind the sphere vertex array object
  glBindVertexArray(g_pCommon->sphereVAOID);
  // set the sphere's transform
  auto T   = glm::translate(glm::mat4(1), glm::vec3(0, 1, 0));
  auto M   = T;
  auto MV  = View * M;
  auto MVP = Proj * MV;
  // pass shader uniforms
  glUniformMatrix4fv(g_pCommon->shader("MVP"), 1, GL_FALSE, glm::value_ptr(MVP));
  glUniformMatrix4fv(g_pCommon->shader("MV"), 1, GL_FALSE, glm::value_ptr(MV));
  glUniformMatrix3fv(g_pCommon->shader("N"), 1, GL_FALSE,
      glm::value_ptr(glm::inverseTranspose(glm::mat3(MV))));
  glUniform3f(g_pCommon->shader("diffuse_color"), 0.9f, 0.9f, 1.0f);
  glUniform3f(g_pCommon->shader("specular_color"), 1.0f, 1.0f, 1.0f);
  glUniform1f(g_pCommon->shader("shininess"), 300);
  glUniform3fv(g_pCommon->shader("light_position"), 1, &(g_pCommon->lightPosOS.x));
  // draw triangles
  glDrawElements(GL_TRIANGLES, g_pCommon->totalSphereTriangles, GL_UNSIGNED_SHORT, nullptr);

  // unbind shader
  g_pCommon->shader.UnUse();

  // unbind vertex array object
  glBindVertexArray(0);

  GL_CHECK_ERRORS;;

  // render the grid object
  g_pCommon->grid->Render(glm::value_ptr(Proj * View));
}

// display callback function
void OnRender() {
  // increment the radius so cubes are radially displaced each frame
  g_pCommon->radius += g_pCommon->dx;

  // if we are at limits, change the movement direction
  if (g_pCommon->radius < 1 || g_pCommon->radius > 5) {
    g_pCommon->dx = -g_pCommon->dx;
  }

  GL_CHECK_ERRORS;

  // clear colour and depth buffers
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // set the camera transform
  const auto T  = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, g_pCommon->dist));
  const auto Rx = glm::rotate(T,  g_pCommon->rX, glm::vec3(1.0f, 0.0f, 0.0f));
  const auto MV = glm::rotate(Rx, g_pCommon->rY, glm::vec3(0.0f, 1.0f, 0.0f));

  // render scene
  DrawScene(MV, g_pCommon->P);

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
  glutCreateWindow("Per-vertex Lighting - OpenGL 3.3");

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
  std::cout << "\tUsing GLEW " << glewGetString(GLEW_VERSION)              << std::endl;
  std::cout << "\tVendor: "    << glGetString(GL_VENDOR)                   << std::endl;
  std::cout << "\tRenderer: "  << glGetString(GL_RENDERER)                 << std::endl;
  std::cout << "\tVersion: "   << glGetString(GL_VERSION)                  << std::endl;
  std::cout << "\tGLSL: "      << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
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
