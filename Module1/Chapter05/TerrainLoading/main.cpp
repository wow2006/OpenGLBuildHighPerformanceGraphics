#include <GL/glew.h>

#include <GL/freeglut.h>

#include <vector>
#include <string>
#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <SOIL.h>

#include <GLSLShader.hpp>

#define GL_CHECK_ERRORS assert(glGetError() == GL_NO_ERROR)


struct Common {
  // output screen resolution
  static constexpr int WIDTH = 1280;
  static constexpr int HEIGHT = 960;

  // shaders for use in the recipe
  GLSLShader shader;

  // IDs for vertex array and buffer object
  GLuint vaoID;
  GLuint vboVerticesID;
  GLuint vboIndicesID;

  // Heighmap texture ID
  GLuint heightMapTextureID;

  // heightmap texture dimensions and half dimensions
  static constexpr int TERRAIN_WIDTH = 512;
  static constexpr int TERRAIN_DEPTH = 512;
  static constexpr int TERRAIN_HALF_WIDTH = TERRAIN_WIDTH >> 1;
  static constexpr int TERRAIN_HALF_DEPTH = TERRAIN_DEPTH >> 1;

  // heightmap height scale and half scale values
  float scale = 50;
  float half_scale = scale / 2.0f;

  // total vertices and indices in the terrain
  static constexpr int TOTAL = (TERRAIN_WIDTH * TERRAIN_DEPTH);
  static constexpr int TOTAL_INDICES = TOTAL * 2 * 3;

  // heightmap filename
  const std::string filename = "media/heightmap512x512.png";

  // height map vertices and indices
  std::vector<glm::vec3> vertices = std::vector<glm::vec3>(TOTAL);
  std::vector<GLuint> indices = std::vector<GLuint>(TOTAL_INDICES);

  // projection and modelview matrices
  glm::mat4 P  = glm::mat4(1);
  glm::mat4 MV = glm::mat4(1);

  // camera transform variables
  int state = 0, oldX = 0, oldY = 0;
  float rX = 25, rY = -40, dist = -7;
};
static Common* g_pCommon = nullptr;

// mouse click handler
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

// mouse move handler
void OnMouseMove(int x, int y) {
  if (g_pCommon->state == 0)
    g_pCommon->dist *= (1 + (y - g_pCommon->oldY) / 60.0f);
  else {
    g_pCommon->rY += (x - g_pCommon->oldX) / 5.0f;
    g_pCommon->rX += (y - g_pCommon->oldY) / 5.0f;
  }
  g_pCommon->oldX = x;
  g_pCommon->oldY = y;

  glutPostRedisplay();
}

// OpenGL initialization
void OnInit() {
  GL_CHECK_ERRORS;
  // load heightmap shader
  g_pCommon->shader.LoadFromFile(GL_VERTEX_SHADER,   "shaders/TerrainLoading.vert");
  g_pCommon->shader.LoadFromFile(GL_FRAGMENT_SHADER, "shaders/TerrainLoading.frag");
  // compile and link shader
  g_pCommon->shader.CreateAndLinkProgram();
  g_pCommon->shader.Use();
  // add attributes and uniforms
  g_pCommon->shader.AddAttribute("vVertex");
  g_pCommon->shader.AddUniform("heightMapTexture");
  g_pCommon->shader.AddUniform("scale");
  g_pCommon->shader.AddUniform("half_scale");
  g_pCommon->shader.AddUniform("HALF_TERRAIN_SIZE");
  g_pCommon->shader.AddUniform("MVP");
  // set values of constant uniforms as initialization
  glUniform1i(g_pCommon->shader("heightMapTexture"), 0);
  glUniform2i(g_pCommon->shader("HALF_TERRAIN_SIZE"),
      Common::TERRAIN_WIDTH >> 1, Common::TERRAIN_DEPTH >> 1);
  glUniform1f(g_pCommon->shader("scale"), g_pCommon->scale);
  glUniform1f(g_pCommon->shader("half_scale"), g_pCommon->half_scale);
  g_pCommon->shader.UnUse();

  GL_CHECK_ERRORS;

  // fill indices array
  GLuint *id = &g_pCommon->indices[0];
  int i = 0, j = 0;

  // setup vertices
  int count = 0;
  // fill terrain vertices
  for (j = 0; j < Common::TERRAIN_DEPTH; j++) {
    for (i = 0; i < Common::TERRAIN_WIDTH; i++) {
      const auto index = static_cast<std::size_t>(count);
      g_pCommon->vertices[index] = glm::vec3((float(i) / (Common::TERRAIN_WIDTH - 1)), 0,
          (float(j) / (Common::TERRAIN_DEPTH - 1)));
      count++;
    }
  }

  count = 0;
  // fill terrain indices
  for (i = 0; i < Common::TERRAIN_DEPTH - 1; i++) {
    for (j = 0; j < Common::TERRAIN_WIDTH - 1; j++) {
      uint i0 = static_cast<uint>(j + i * Common::TERRAIN_WIDTH);
      uint i1 = i0 + 1;
      uint i2 = static_cast<uint>(i0 + Common::TERRAIN_WIDTH);
      uint i3 = i2 + 1;
      *id++ = i0;
      *id++ = i2;
      *id++ = i1;
      *id++ = i1;
      *id++ = i2;
      *id++ = i3;
    }
  }

  GL_CHECK_ERRORS;

  // setup terrain vertex array and vertex buffer objects
  glGenVertexArrays(1, &g_pCommon->vaoID);
  glGenBuffers(1, &g_pCommon->vboVerticesID);
  glGenBuffers(1, &g_pCommon->vboIndicesID);

  glBindVertexArray(g_pCommon->vaoID);

  glBindBuffer(GL_ARRAY_BUFFER, g_pCommon->vboVerticesID);
  // pass terrain vertices to buffer object
  glBufferData(GL_ARRAY_BUFFER, g_pCommon->vertices.size() * sizeof(glm::vec3), &g_pCommon->vertices[0], GL_STATIC_DRAW);
  GL_CHECK_ERRORS;
  // enable vertex attribute array for position
  glEnableVertexAttribArray(g_pCommon->shader["vVertex"]);
  glVertexAttribPointer(g_pCommon->shader["vVertex"], 3, GL_FLOAT, GL_FALSE, 0, nullptr);
  GL_CHECK_ERRORS;
  // pass the terrain indices array to element array buffer
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_pCommon->vboIndicesID);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, g_pCommon->indices.size() * sizeof(GLuint), &g_pCommon->indices[0],
      GL_STATIC_DRAW);
  GL_CHECK_ERRORS;

  // load the heightmap texture using SOIL
  int texture_width = 0, texture_height = 0, channels = 0;
  GLubyte *pData = SOIL_load_image(g_pCommon->filename.c_str(), &texture_width,
      &texture_height, &channels, SOIL_LOAD_L);

  // vertically flip the heightmap image on Y axis since it is inverted
  for (j = 0; j * 2 < texture_height; ++j) {
    int index1 = j * texture_width;
    int index2 = (texture_height - 1 - j) * texture_width;
    for (i = texture_width; i > 0; --i) {
      GLubyte temp = pData[index1];
      pData[index1] = pData[index2];
      pData[index2] = temp;
      ++index1;
      ++index2;
    }
  }

  // setup OpenGL texture
  glGenTextures(1, &g_pCommon->heightMapTextureID);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, g_pCommon->heightMapTextureID);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, texture_width, texture_height, 0,
      GL_RED, GL_UNSIGNED_BYTE, pData);

  // free SOIL image data
  SOIL_free_image_data(pData);

  GL_CHECK_ERRORS;

  // set polygon mode to draw lines
  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

  GL_CHECK_ERRORS;

  std::cout << "Initialization successfull" << std::endl;
}

//release all allocated resources
void OnShutdown() {
  //Destroy shader
  g_pCommon->shader.DeleteShaderProgram();

  //Destroy vao and vbo
  glDeleteBuffers(1, &g_pCommon->vboVerticesID);
  glDeleteBuffers(1, &g_pCommon->vboIndicesID);
  glDeleteVertexArrays(1, &g_pCommon->vaoID);

  //Delete textures
  glDeleteTextures(1, &g_pCommon->heightMapTextureID);
  std::cout << "Shutdown successfull" << std::endl;
}

//resize event handler
void OnResize(int w, int h) {
  //set the viewport
  glViewport (0, 0, static_cast<GLsizei>(w), static_cast<GLsizei>(h));

  //setup the projection matrix
  g_pCommon->P = glm::perspective(glm::radians(45.0f), static_cast<GLfloat>(w)/h, 0.01f, 10000.f);
}

// display function
void OnRender() {
  // clear colour and depth buffers
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // set the camera transform
  const auto T   = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, g_pCommon->dist));
  const auto Rx  = glm::rotate(T,  g_pCommon->rX, glm::vec3(1.0f, 0.0f, 0.0f));
  const auto Ry  = glm::rotate(Rx, g_pCommon->rY, glm::vec3(0.0f, 1.0f, 0.0f));
  const auto MV  = Ry;
  const auto MVP = g_pCommon->P * MV;

  // since we have kept the terrain vertex array object bound
  // it is still bound to the context so we can directly call draw element
  // which will draw vertices from the bound vertex array object
  // bind the terrain shader
  g_pCommon->shader.Use();
  // pass shader uniforms
  glUniformMatrix4fv(g_pCommon->shader("MVP"), 1, GL_FALSE, glm::value_ptr(MVP));
  // draw terrain mesh
  glDrawElements(GL_TRIANGLES, Common::TOTAL_INDICES, GL_UNSIGNED_INT, nullptr);
  // unbind shader
  g_pCommon->shader.UnUse();

  // swap front and back buffers to show the rendered result
  glutSwapBuffers();
}

int main(int argc, char* argv[]) {
  Common common;
  g_pCommon = &common;

  // freeglut initialization
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
  glutInitContextVersion(3, 3);
  glutInitContextFlags(GLUT_CORE_PROFILE | GLUT_DEBUG);
  glutInitWindowSize(Common::WIDTH, Common::HEIGHT);
  glutCreateWindow("Simple terrain - OpenGL 3.3");

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
  std::cout << "\tGLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;

  GL_CHECK_ERRORS;

  // OpenGL initialization
  OnInit();

  // callback hooks
  glutCloseFunc(OnShutdown);
  glutDisplayFunc(OnRender);
  glutReshapeFunc(OnResize);
  glutMouseFunc(OnMouseDown);
  glutMotionFunc(OnMouseMove);

  // call main loop
  glutMainLoop();

  return 0;
}
