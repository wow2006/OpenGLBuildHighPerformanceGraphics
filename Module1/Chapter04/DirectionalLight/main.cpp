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

  // direcitonal lighting shader
  GLSLShader shader;
  GLSLShader *pFlatShader; // we will reuse the grid shader for crosshair at
                           // light position

  // sphere vertex array and vertex buffer object IDs
  GLuint sphereVAOID;
  GLuint sphereVerticesVBO;
  GLuint sphereIndicesVBO;

  // cube vertex array and vertex buffer object IDs
  GLuint cubeVAOID;
  GLuint cubeVerticesVBO;
  GLuint cubeIndicesVBO;

  // light direction line gizmo vertex array and vertex buffer object IDs
  GLuint lightVAOID;
  GLuint lightVerticesVBO;

  // projection and  modelview matrices
  glm::mat4 P = glm::mat4(1);
  glm::mat4 MV = glm::mat4(1);

  // camera transformation variables
  int state = 0, oldX = 0, oldY = 0;
  float rX = 25, rY = -40, dist = -10;

  // Grid object
  CGrid *grid = nullptr;

  glm::vec3 lightDirectionOS = glm::vec3(0, 0, 0); // objectspace light position

  // vertices and indices for sphere/cube
  std::vector<Vertex> vertices;
  std::vector<GLushort> indices;
  int totalSphereTriangles = 0;

  // spherical coorindates for light direction
  float theta = 0.9f;
  float phi = 1.37f;
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
    g_pCommon->lightDirectionOS.x =
        std::cos(g_pCommon->theta) * std::sin(g_pCommon->phi);
    g_pCommon->lightDirectionOS.y = std::cos(g_pCommon->phi);
    g_pCommon->lightDirectionOS.z =
        std::sin(g_pCommon->theta) * std::sin(g_pCommon->phi);

    // update the light gizmo buffer object
    glBindBuffer(GL_ARRAY_BUFFER, g_pCommon->lightVerticesVBO);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(glm::vec3), sizeof(glm::vec3),
                    &g_pCommon->lightDirectionOS.x);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
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
  // load the directional lighting shader
  g_pCommon->shader.LoadFromFile(GL_VERTEX_SHADER,
                                 "shaders/DirectionalLight.vert");
  g_pCommon->shader.LoadFromFile(GL_FRAGMENT_SHADER,
                                 "shaders/DirectionalLight.frag");
  // compile and link shader
  g_pCommon->shader.CreateAndLinkProgram();
  g_pCommon->shader.Use();
  // add attributes and uniforms
  g_pCommon->shader.AddAttribute("vVertex");
  g_pCommon->shader.AddAttribute("vNormal");
  g_pCommon->shader.AddUniform("MV");
  g_pCommon->shader.AddUniform("MVP");
  g_pCommon->shader.AddUniform("N");
  g_pCommon->shader.AddUniform("light_direction");
  g_pCommon->shader.AddUniform("diffuse_color");
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
  std::vector<GLushort> x;
  std::vector<Vertex> y;
  CreateCube(2.0f, y, x);
  CreateCube(2.0f, g_pCommon->vertices, g_pCommon->indices);

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
  // enable vertex attribute array for normal
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

  // calculate the light direction vector using spherical coordinates
  g_pCommon->lightDirectionOS.x =
      std::cos(g_pCommon->theta) * std::sin(g_pCommon->phi);
  g_pCommon->lightDirectionOS.y = std::cos(g_pCommon->phi);
  g_pCommon->lightDirectionOS.z =
      std::sin(g_pCommon->theta) * std::sin(g_pCommon->phi);

  // setup vao and vbo stuff for the light position crosshair
  glm::vec3 crossHairVertices[2];
  crossHairVertices[0] = glm::vec3(0, 0, 0);
  crossHairVertices[1] = g_pCommon->lightDirectionOS;

  // setup vertex array object and buffer object for storing the light direction
  // as a line segment from origin
  glGenVertexArrays(1, &g_pCommon->lightVAOID);
  glGenBuffers(1, &g_pCommon->lightVerticesVBO);
  glBindVertexArray(g_pCommon->lightVAOID);

  glBindBuffer(GL_ARRAY_BUFFER, g_pCommon->lightVerticesVBO);
  // pass crosshair vertices to buffer object
  glBufferData(GL_ARRAY_BUFFER, sizeof(crossHairVertices),
               &(crossHairVertices[0].x), GL_STATIC_DRAW);
  GL_CHECK_ERRORS;
  // enable vertex attribute array for position
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
  GL_CHECK_ERRORS;

  // enable depth testing and culling
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);
  GL_CHECK_ERRORS;

  // create a grid of 10x10 size in XZ plane
  g_pCommon->grid = new CGrid();
  GL_CHECK_ERRORS;

  // get the pointer to the grid's shader for rendering of the direcitonal
  // light gizmo
  g_pCommon->pFlatShader = g_pCommon->grid->GetShader();

  std::cout << "Initialization successfull" << std::endl;
}

// Release all allocated resources
void OnShutdown() {
  g_pCommon->pFlatShader = nullptr;

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

  delete g_pCommon->grid;
  g_pCommon->grid = nullptr;

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
void DrawScene(glm::mat4 MView, glm::mat4 Proj) {
  GL_CHECK_ERRORS;

  // bind the current shader
  g_pCommon->shader.Use();

  // bind the cube vertex array object
  glBindVertexArray(g_pCommon->cubeVAOID);
  {
    // set the cube's transform
    const auto T    = glm::translate(glm::mat4(1), glm::vec3(-1, 1, 0));
    const auto cM   = T;            // model matrix
    const auto cMV  = MView * cM;   // modelview matrix
    const auto cMVP = Proj * cMV;   // combined modelview projection matrix

    // pass shader uniforms
    glUniformMatrix4fv(g_pCommon->shader("MVP"), 1, GL_FALSE,
                       glm::value_ptr(cMVP));
    glUniformMatrix4fv(g_pCommon->shader("MV"), 1, GL_FALSE,
                       glm::value_ptr(cMV));
    glUniformMatrix3fv(g_pCommon->shader("N"), 1, GL_FALSE,
                       glm::value_ptr(glm::inverseTranspose(glm::mat3(cMV))));

    glUniform3f(g_pCommon->shader("diffuse_color"), 1.0f, 0.0f, 0.0f);
    glUniform3fv(g_pCommon->shader("light_direction"), 1,
                 &(g_pCommon->lightDirectionOS.x));

    // draw triangles
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, nullptr);
  }

  // bind the sphere vertex array object
  glBindVertexArray(g_pCommon->sphereVAOID);
  {
    // set the sphere's transform
    const auto T = glm::translate(glm::mat4(1), glm::vec3(1, 1, 0));
    const auto cM = T;
    const auto cMV = MView * cM;
    const auto cMVP = Proj * cMV;

    // Pass shader uniforms
    glUniformMatrix4fv(g_pCommon->shader("MVP"), 1, GL_FALSE,
                       glm::value_ptr(cMVP));
    glUniformMatrix4fv(g_pCommon->shader("MV"), 1, GL_FALSE,
                       glm::value_ptr(cMV));
    glUniformMatrix3fv(g_pCommon->shader("N"), 1, GL_FALSE,
                       glm::value_ptr(glm::inverseTranspose(glm::mat3(cMV))));
    glUniform3f(g_pCommon->shader("diffuse_color"), 0.0f, 0.0f, 1.0f);

    // Draw triangles
    glDrawElements(GL_TRIANGLES, g_pCommon->totalSphereTriangles,
                   GL_UNSIGNED_SHORT, nullptr);
  }

  // unbind shader
  g_pCommon->shader.UnUse();

  // show the light vector on screen
  glBindVertexArray(g_pCommon->lightVAOID);
  {
    // bind the flat shader
    g_pCommon->pFlatShader->Use();
    // set shader uniform
    glUniformMatrix4fv((*g_pCommon->pFlatShader)("MVP"), 1, GL_FALSE,
                       glm::value_ptr(Proj * MView));
    // draw line segment
    glDrawArrays(GL_LINES, 0, 2);
    // unbind flat shader
    g_pCommon->pFlatShader->UnUse();
  }

  // unbind vertex array object
  glBindVertexArray(0);

  GL_CHECK_ERRORS;

  // render the grid object
  g_pCommon->grid->Render(glm::value_ptr(Proj * MView));
}

// display callback function
void OnRender() {
  GL_CHECK_ERRORS;

  // clear colour and depth buffers
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // set the camera transform
  const auto T =
      glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, g_pCommon->dist));
  const auto Rx = glm::rotate(T, g_pCommon->rX, glm::vec3(1.0f, 0.0f, 0.0f));
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
  std::cout << "\tVendor: "    << glGetString(GL_VENDOR)      << std::endl;
  std::cout << "\tRenderer: "  << glGetString(GL_RENDERER)    << std::endl;
  std::cout << "\tVersion: "   << glGetString(GL_VERSION)     << std::endl;
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
