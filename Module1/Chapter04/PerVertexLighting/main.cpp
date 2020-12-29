// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
// STL
#include <cmath>
#include <vector>
#include <iostream>
// GL
#include <GL/glew.h>
// GLFW
#include <GLFW/glfw3.h>
// ImGui
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
// glm
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/matrix_transform.hpp>
// Internal
#include "GLSLShader.hpp"
#include "Grid.hpp"

// Vertex struct with position and normal
struct Vertex {
  glm::vec3 pos;
  glm::vec3 normal;
};

enum State {
  None = 0, Left, Middle, Right
};

struct Matrices {
  glm::mat4 MVP;
  glm::mat4 MV;
  glm::mat4 Normal;
};

struct Common {
  // screen size
  static constexpr int WIDTH = 1024;
  static constexpr int HEIGHT = 768;

  float MouseDeltaX = 0.001F;
  float MouseDeltaY = 0.001F;

  // per-vertex lighting shader
  GLSLShader shader;

  GLuint UBO = 0;

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
  glm::mat4 P = glm::mat4(1);
  glm::mat4 MV = glm::mat4(1);
  glm::mat4 Rot;

  // camera transformation variables
  State state = State::None;
  int oldX = 0;
  int oldY = 0;
  float rX = 45.F;
  float rY = 45.F;
  float dist =-20.F;

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
  glm::vec3 colors[8] = {glm::vec3(1, 0, 0), glm::vec3(0, 1, 0),
                         glm::vec3(0, 0, 1), glm::vec3(1, 1, 0),
                         glm::vec3(1, 0, 1), glm::vec3(0, 1, 1),
                         glm::vec3(1, 1, 1), glm::vec3(0.5, 0.5, 0.5)};
};
static Common *g_pCommon = nullptr;

constexpr auto BindingPoint = 0;

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
   float const R = 1.0f / static_cast<float>(slices - 1);
   float const S = 1.0f / static_cast<float>(stacks - 1);

   for(uint r = 0; r < slices; ++r) {
    for(uint s = 0; s < stacks; ++s) {
      float const y = static_cast<float>(sinf(-static_cast<float>(M_PI_2) + glm::pi<float>() * static_cast<float>(r) * R));
      float const x = static_cast<float>(cosf(2.F * glm::pi<float>() * static_cast<float>(s) * S) * sinf(glm::pi<float>() * static_cast<float>(r) * R));
      float const z = static_cast<float>(sinf(2.F * glm::pi<float>() * static_cast<float>(s) * S) * sinf(glm::pi<float>() * static_cast<float>(r) * R));

      Vertex v;
      v.pos = glm::vec3(x, y, z) * radius;
      v.normal = glm::normalize(v.pos);
      vertices.push_back(v);
      push_indices(static_cast<int>(stacks), static_cast<int>(r), static_cast<int>(s), indices);
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
  //g_pCommon->shader.AddUniform("MVP");
  //g_pCommon->shader.AddUniform("MV");
  //g_pCommon->shader.AddUniform("N");
  g_pCommon->shader.AddUniform("light_position");
  g_pCommon->shader.AddUniform("diffuse_color");
  g_pCommon->shader.AddUniform("specular_color");
  g_pCommon->shader.AddUniform("shininess");
  g_pCommon->shader.UnUse();

  // Cerate sphere geometry
  CreateSphere(1.0f, 10, 10, g_pCommon->vertices, g_pCommon->indices);

  // setup sphere vao and vbo stuff
  glGenVertexArrays(1, &g_pCommon->sphereVAOID);
  glGenBuffers(1, &g_pCommon->sphereVerticesVBO);
  glGenBuffers(1, &g_pCommon->sphereIndicesVBO);
  glBindVertexArray(g_pCommon->sphereVAOID);

  glBindBuffer(GL_ARRAY_BUFFER, g_pCommon->sphereVerticesVBO);
  // pass vertices to the buffer object
  glBufferData(GL_ARRAY_BUFFER, g_pCommon->vertices.size() * sizeof(Vertex), &g_pCommon->vertices[0], GL_STATIC_DRAW);
  // enable vertex attribute array for position
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), nullptr);
  // enable vertex attribute array for normal
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<const GLvoid *>(offsetof(Vertex, normal)));

  // pass sphere indices to element array buffer
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_pCommon->sphereIndicesVBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, g_pCommon->indices.size() * sizeof(GLushort), &g_pCommon->indices[0], GL_STATIC_DRAW);

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
  glGenBuffers(1, &g_pCommon->cubeVerticesVBO);
  glGenBuffers(1, &g_pCommon->cubeIndicesVBO);
  glBindVertexArray(g_pCommon->cubeVAOID);

  glBindBuffer(GL_ARRAY_BUFFER, g_pCommon->cubeVerticesVBO);
  // pass vertices to the buffer object
  glBufferData(GL_ARRAY_BUFFER, g_pCommon->vertices.size() * sizeof(Vertex), &g_pCommon->vertices[0], GL_STATIC_DRAW);

  // enable vertex attribut array for position
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), nullptr);

  // enable vertex attribut array for normal
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<const GLvoid *>(offsetof(Vertex, normal)));

  // pass cube indices to element array buffer
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_pCommon->cubeIndicesVBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, g_pCommon->indices.size() * sizeof(GLushort), &g_pCommon->indices[0], GL_STATIC_DRAW);

  // enable depth testing and culling
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);

  // create a grid of 10x10 size in XZ plane
  g_pCommon->grid = new CGrid();

  std::cout << "Initialization successfull" << std::endl;
}

// scene rendering function
void DrawScene(glm::mat4 View, glm::mat4 Proj) {
  // Bind the current shader
  g_pCommon->shader.Use();

  // bind the cube vertex array object
  glBindVertexArray(g_pCommon->cubeVAOID);

  const auto UBO     = g_pCommon->UBO;
  const auto program = g_pCommon->shader._program;
  glUniformBlockBinding(program, 0, BindingPoint);

  // draw the 8 cubes first
  for (int i = 0; i < 8; i++) {
    // set the cube's transform
    const float theta = static_cast<float>(i) / 8.0F * 2.F * glm::pi<float>();
    glm::mat4 T = glm::translate(glm::mat4(1), glm::vec3(g_pCommon->radius * std::cos(theta), 0.5f, g_pCommon->radius * std::sin(theta)));
    glm::mat4 M   = T;         // Model matrix
    glm::mat4 MV  = View * M;  // ModelView matrix
    glm::mat4 MVP = Proj * MV; // combined ModelView  Projection matrix
    const auto matrices = Matrices {
      MVP,
      MV,
      glm::mat3(glm::inverse(MV))
    };

    // pass shader uniforms
    {
      glBindBuffer(GL_UNIFORM_BUFFER, UBO);
      glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(Matrices), &matrices);
      glBindBuffer(GL_UNIFORM_BUFFER, 0);
    }

    glUniform3fv(g_pCommon->shader("diffuse_color"), 1, &(g_pCommon->colors[i].x));
    glUniform3f(g_pCommon->shader("specular_color"), 1.0f, 1.0f, 1.0f);
    glUniform1f(g_pCommon->shader("shininess"), 100);
    glUniform3fv(g_pCommon->shader("light_position"), 1, &(g_pCommon->lightPosOS.x));
    // draw triangles
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, nullptr);
  }
  // bind the sphere vertex array object
  glBindVertexArray(g_pCommon->sphereVAOID);
  // set the sphere's transform
  auto T = glm::translate(glm::mat4(1), glm::vec3(0, 1, 0));
  auto M = T;
  auto MV = View * M;
  auto MVP = Proj * MV;
  const auto matrices = Matrices {
    MVP,
    MV,
    glm::mat3(glm::inverse(MV))
  };
  // pass shader uniforms
  {
    glBindBuffer(GL_UNIFORM_BUFFER, UBO);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(Matrices), &matrices);
    glUniformBlockBinding(g_pCommon->shader._program, 0, BindingPoint);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
  }
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

  // render the grid object
  g_pCommon->grid->Render(glm::value_ptr(Proj * View));
}

int main(int argc, char **argv) {
  (void)argc; (void)argv;
  Common common;
  g_pCommon = &common;

  glfwSetErrorCallback([](int errorCode, const char* message) {
    fprintf(stderr, "GLFW ERROR(%d): %s\n", errorCode, message);
  });

  // glfw initialization
  if (glfwInit() != GLFW_TRUE) {
    return EXIT_FAILURE;
  }

  glfwWindowHint(GLFW_RED_BITS,   8);
  glfwWindowHint(GLFW_GREEN_BITS, 8);
  glfwWindowHint(GLFW_BLUE_BITS,  8);
  glfwWindowHint(GLFW_DEPTH_BITS, 16);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);

  glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);
  // clang-format off
  auto *pWindow = glfwCreateWindow(Common::WIDTH, Common::HEIGHT,
                                   "Per-vertex Lighting - OpenGL 3.3",
                                   nullptr, nullptr);
  if (pWindow == nullptr) {
    glfwTerminate();
    return EXIT_FAILURE;
  }
  glfwMakeContextCurrent(pWindow);
  glfwSetMouseButtonCallback(pWindow, [](GLFWwindow* pWin, int button, int action, [[maybe_unused]]int mod) {
    if (button == GLFW_MOUSE_BUTTON_LEFT  && action == GLFW_PRESS) {
      g_pCommon->state = State::Left;
    } else if(button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_PRESS) {
      g_pCommon->state = State::Middle;
    } else if(button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
      g_pCommon->state = State::Right;
    } else {
      g_pCommon->state = State::None;
    }
    double x, y;
    glfwGetCursorPos(pWin, &x, &y);
    g_pCommon->oldX = static_cast<int>(x);
    g_pCommon->oldY = static_cast<int>(y);
  });
  glfwSetCursorPosCallback(pWindow, []([[maybe_unused]]GLFWwindow* pWin, double x, double y) {
    if (g_pCommon->state == State::Right) {
      g_pCommon->dist *= (1.F + float(y - g_pCommon->oldY) / 60.0F);
    } else if(g_pCommon->state == State::Left) {
      g_pCommon->rX += float(x - g_pCommon->oldX) * g_pCommon->MouseDeltaX;
      g_pCommon->rY += float(y - g_pCommon->oldY) * g_pCommon->MouseDeltaY;
    }
    g_pCommon->oldX = static_cast<int>(x);
    g_pCommon->oldY = static_cast<int>(y);
  });
  glfwSetWindowSizeCallback(pWindow, []([[maybe_unused]]GLFWwindow* pWin, int w, int h) {
    // set the viewport
    glViewport(0, 0, w, h);

    // setup the projection matrix
    g_pCommon->P = glm::perspective(
        glm::radians(45.0F), static_cast<GLfloat>(w) / static_cast<GLfloat>(h), 0.1F, 1000.F);
  });
  // clang-format on

  const auto ratio = static_cast<GLfloat>(Common::WIDTH) /
                     static_cast<GLfloat>(Common::HEIGHT);
  g_pCommon->P = glm::perspective(glm::radians(45.0F), ratio, 0.1F, 1000.F);

  // initialize glew
  glewExperimental = GL_TRUE;
  GLenum err = glewInit();
  if (GLEW_OK != err) {
    std::cerr << "Error: " << glewGetErrorString(err) << std::endl;
    return EXIT_FAILURE;
  } else {
    if (GLEW_VERSION_3_3) {
      std::cout << "Driver supports OpenGL 3.3\nDetails:" << std::endl;
    }
  }
  err = glGetError(); // this is to ignore INVALID ENUM error 1282

  if(glDebugMessageCallbackARB) {
    std::cout << "OpenGL Debug\n";
    glDebugMessageCallbackARB([](
      GLenum, GLenum, GLuint, GLenum severity,
      GLsizei, const char* message, const void*) {
        if(severity == GL_DEBUG_SEVERITY_HIGH_ARB) {
          std::cerr << "OpenGL ERROR: " << message << '\n';
        }
    }, nullptr);
  }

  // output hardware information
  std::cout << "\tUsing GLEW " << glewGetString(GLEW_VERSION) << std::endl;
  std::cout << "\tVendor: "    << glGetString(GL_VENDOR)      << std::endl;
  std::cout << "\tRenderer: "  << glGetString(GL_RENDERER)    << std::endl;
  std::cout << "\tVersion: "   << glGetString(GL_VERSION)     << std::endl;
  std::cout << "\tGLSL: "      << glGetString(GL_SHADING_LANGUAGE_VERSION)
            << std::endl;

  // OpenGL initialization
  OnInit();

  // Initialize ImGui
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO();
  (void)io;

  // Setup Dear ImGui style
  ImGui::StyleColorsDark();

  // Setup Platform/Renderer backends
  ImGui_ImplGlfw_InitForOpenGL(pWindow, true);
  ImGui_ImplOpenGL3_Init("#version 330 core");

  GLuint& UBO = g_pCommon->UBO;
  glGenBuffers(1, &UBO);
  glBindBuffer(GL_UNIFORM_BUFFER, UBO);
  glBufferData(GL_UNIFORM_BUFFER, sizeof(Matrices), nullptr, GL_DYNAMIC_DRAW);
  glBindBufferBase(GL_UNIFORM_BUFFER, BindingPoint, UBO);
  glBindBuffer(GL_UNIFORM_BUFFER, 0);

  while(!glfwWindowShouldClose(pWindow)) {
    glfwPollEvents();

    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("Widget");
    {
      double x, y;
      glfwGetCursorPos(pWindow, &x, &y);
      ImGui::Text("Mouse    (%03.3F,%03.3F)", x, y);
      ImGui::Text("Rotation (%03.3F,%03.3F)", g_pCommon->rX,   g_pCommon->rY);
      ImGui::Text("Old      (%03d,%03d)",     g_pCommon->oldY, g_pCommon->oldY);
      ImGui::Text("Zoom : %03.3F", g_pCommon->dist);
      if(ImGui::Button("Reset View")) {
        g_pCommon->rX   = 56.F;
        g_pCommon->rY   = 44.F;
        g_pCommon->dist =-5.F;
      }
      ImGui::DragFloat("DeltaX", &g_pCommon->MouseDeltaX, 0.01F, 1.0F / 1000.F, 1.0F);
      ImGui::DragFloat("DeltaY", &g_pCommon->MouseDeltaY, 0.01F, 1.0F / 1000.F, 1.0F);

      const char* items[] = {
        "None",
        "Left",
        "Middle",
        "Right"
      };
      ImGui::Combo("Mouse", reinterpret_cast<int*>(&g_pCommon->state),
                   items, IM_ARRAYSIZE(items));
    }
    ImGui::End();

    // increment the radius so cubes are radially displaced each frame
    g_pCommon->radius += g_pCommon->dx;

    // if we are at limits, change the movement direction
    if (g_pCommon->radius < 1 || g_pCommon->radius > 5) {
      g_pCommon->dx = -g_pCommon->dx;
    }

    // Rendering
    ImGui::Render();

    // clear colour and depth buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // set the camera transform
    const auto T  = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, g_pCommon->dist));
    const auto Rx = glm::rotate(T,  g_pCommon->rY, glm::vec3(1.0f, 0.0f, 0.0f));
    const auto MV = glm::rotate(Rx, g_pCommon->rX, glm::vec3(0.0f, 1.0f, 0.0f));

    // render scene
    DrawScene(MV, g_pCommon->P);

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    // swap front and back buffers to show the rendered result
    glfwSwapBuffers(pWindow);
  }

  // Destroy shader
  g_pCommon->shader.DeleteShaderProgram();
  // Destroy vao and vbo
  glDeleteBuffers(1, &g_pCommon->sphereVerticesVBO);
  glDeleteBuffers(1, &g_pCommon->sphereIndicesVBO);
  glDeleteVertexArrays(1, &g_pCommon->sphereVAOID);

  glDeleteBuffers(1, &g_pCommon->cubeVerticesVBO);
  glDeleteBuffers(1, &g_pCommon->cubeIndicesVBO);
  glDeleteVertexArrays(1, &g_pCommon->cubeVAOID);

  // Cleanup
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();

  delete g_pCommon->grid;
  g_pCommon->grid = nullptr;
  std::cout << "Shutdown successfull" << std::endl;

  return EXIT_SUCCESS;
}

