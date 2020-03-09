// STL
#include <cassert>
#include <cstdlib>
#include <iostream>
// GLEW
#include <GL/glew.h>
// GLUT
#include <GL/freeglut.h>
// GLM
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
// SOIL
#include <SOIL/SOIL.h>
// Internal
#include "Obj.hpp"
#include "GLSLShader.hpp"

#define GL_CHECK_ERRORS assert(glGetError() == GL_NO_ERROR)

namespace {
// output screen resolution
constexpr int WIDTH = 1280;
constexpr int HEIGHT = 960;
// spherical harmonics shader, mesh rendering shader and flat shader
GLSLShader sh_shader, shader, flatShader;
// pointer to current shader
GLSLShader *pCurrentShader;
// IDs for vertex array and buffer object
GLuint vaoID;
GLuint vboVerticesID;
GLuint vboIndicesID;
// light crosshair gizmo vetex array and buffer object IDs
GLuint lightVAOID;
GLuint lightVerticesVBO;
// projection and modelview matrices
glm::mat4 P = glm::mat4(1);
glm::mat4 MV = glm::mat4(1);
// Objloader instance
ObjLoader obj;
vector<Mesh *> meshes;          // all meshes
vector<Material *> materials;   // all materials
vector<unsigned short> indices; // all mesh indices
vector<Vertex> vertices;        // all mesh vertices
vector<GLuint> textures;        // all textures
// camera transformation variables
int state = 0, oldX = 0, oldY = 0;
float rX = 22, rY = 116, dist = -120;
// spherical cooridate variables for light rotation
float theta = 0.66f;
float phi = -1.0f;
float radius = 70;
// OBJ mesh filename to load
const std::string mesh_filename = "media/blocks.obj";
// objectspace light position
glm::vec3 lightPosOS = glm::vec3(0, 2, 0);
} // namespace

// OpenGL initialization
void OnInit();

// Resize event handler
void OnResize(int w, int h);

// display callback function
void OnRender();

// Release allocated resources
void OnShutdown();

namespace Mouse {
// mouse click handler
static void OnMouseDown(int button, int s, int x, int y) {
  if (s == GLUT_DOWN) {
    oldX = x;
    oldY = y;
  }

  if (button == GLUT_MIDDLE_BUTTON)
    state = 0;
  else if (button == GLUT_RIGHT_BUTTON)
    state = 2;
  else
    state = 1;
}

// mouse move handler
static void OnMouseMove(int x, int y) {
  if (state == 0)
    dist *= (1 + (y - oldY) / 60.0f);
  else if (state == 2) {
    theta += (oldX - x) / 60.0f;
    phi += (y - oldY) / 60.0f;

    // update the light position
    lightPosOS.x = radius * cos(theta) * sin(phi);
    lightPosOS.y = radius * cos(phi);
    lightPosOS.z = radius * sin(theta) * sin(phi);

  } else {
    rY += (x - oldX) / 5.0f;
    rX += (y - oldY) / 5.0f;
  }
  oldX = x;
  oldY = y;

  glutPostRedisplay();
}
} // namespace Mouse

namespace Keyboard {
// keyboard event handler
void OnKey(unsigned char key, int x, int y) {
  switch (key) {
  case ' ':
    if (pCurrentShader == &sh_shader)
      pCurrentShader = &shader;
    else
      pCurrentShader = &sh_shader;
    break;
  }
  glutPostRedisplay();
}
} // namespace Keyboard

auto main(int argc, char *argv[]) -> int {
  // freeglut initialization
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
  glutInitContextVersion(3, 3);
  glutInitContextFlags(GLUT_CORE_PROFILE | GLUT_DEBUG);
  glutInitWindowSize(WIDTH, HEIGHT);
  glutCreateWindow("Spherical Harmonics - OpenGL 3.3");

  // initialize glew
  glewExperimental = GL_TRUE;
  GLenum err = glewInit();
  if (GLEW_OK != err) {
    std::cerr << "Error: " << glewGetErrorString(err) << '\n';
  } else {
    if (GLEW_VERSION_3_3) {
      std::cout << "Driver supports OpenGL 3.3\nDetails:\n";
    }
  }
  err = glGetError(); // this is to ignore INVALID ENUM error 1282
  GL_CHECK_ERRORS;

  // output hardware information
  std::cout << "\tUsing GLEW " << glewGetString(GLEW_VERSION) << '\n';
  std::cout << "\tVendor: " << glGetString(GL_VENDOR) << '\n';
  std::cout << "\tRenderer: " << glGetString(GL_RENDERER) << '\n';
  std::cout << "\tVersion: " << glGetString(GL_VERSION) << '\n';
  std::cout << "\tGLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << '\n';
  GL_CHECK_ERRORS;

  // OpenGL initialization
  OnInit();

  // Callback Hooks
  glutCloseFunc(OnShutdown);
  glutDisplayFunc(OnRender);
  glutReshapeFunc(OnResize);
  glutKeyboardFunc(Keyboard::OnKey);
  glutMouseFunc(Mouse::OnMouseDown);
  glutMotionFunc(Mouse::OnMouseMove);

  // main loop call
  glutMainLoop();

  return EXIT_SUCCESS;
}

// OpenGL initialization
void OnInit() {
  // get the OBJ mesh path
  std::string mesh_path =
      mesh_filename.substr(0, mesh_filename.find_last_of("/") + 1);

  // load the OBJ model
  if (!obj.Load(mesh_filename.c_str(), meshes, vertices, indices, materials)) {
    std::cout << "Cannot load the obj mesh\n";
    exit(EXIT_FAILURE);
  }
  GL_CHECK_ERRORS;

  // load material textures
  for (size_t k = 0; k < materials.size(); k++) {
    // if the diffuse texture name is not empty
    if (materials[k]->map_Kd != "") {
      GLuint id = 0;
      // generate a new OpenGL texture
      glGenTextures(1, &id);
      glBindTexture(GL_TEXTURE_2D, id);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
      int texture_width = 0, texture_height = 0, channels = 0;

      const std::string &filename = materials[k]->map_Kd;

      std::string full_filename = mesh_path;
      full_filename.append(filename);

      // use SOIL to load the texture
      GLubyte *pData =
          SOIL_load_image(full_filename.c_str(), &texture_width,
                          &texture_height, &channels, SOIL_LOAD_AUTO);
      if (pData == NULL) {
        std::cerr << "Cannot load image: " << full_filename.c_str() << '\n';
        exit(EXIT_FAILURE);
      }

      // Flip the image on Y axis
      int i, j;
      for (j = 0; j * 2 < texture_height; ++j) {
        int index1 = j * texture_width * channels;
        int index2 = (texture_height - 1 - j) * texture_width * channels;
        for (i = texture_width * channels; i > 0; --i) {
          GLubyte temp = pData[index1];
          pData[index1] = pData[index2];
          pData[index2] = temp;
          ++index1;
          ++index2;
        }
      }
      // get the image format
      GLenum format = GL_RGBA;
      switch (channels) {
      case 2:
        format = GL_RG32UI;
        break;
      case 3:
        format = GL_RGB;
        break;
      case 4:
        format = GL_RGBA;
        break;
      }
      // allocate the texture
      glTexImage2D(GL_TEXTURE_2D, 0, format, texture_width, texture_height, 0,
                   format, GL_UNSIGNED_BYTE, pData);

      // release the SOIL image data
      SOIL_free_image_data(pData);

      // add the texture id to a vector
      textures.push_back(id);
    }
  }
  GL_CHECK_ERRORS;

  // load the flat shader
  flatShader.LoadFromFile(GL_VERTEX_SHADER, "shaders/flat.vert");
  flatShader.LoadFromFile(GL_FRAGMENT_SHADER, "shaders/flat.frag");
  // compile and link shader
  flatShader.CreateAndLinkProgram();
  flatShader.Use();
  // add attribute and uniform
  flatShader.AddAttribute("vVertex");
  flatShader.AddUniform("MVP");
  flatShader.UnUse();

  // load spherical harmonics shader
  sh_shader.LoadFromFile(GL_VERTEX_SHADER, "shaders/sh_shader.vert");
  sh_shader.LoadFromFile(GL_FRAGMENT_SHADER, "shaders/sh_shader.frag");
  // compile and link shader
  sh_shader.CreateAndLinkProgram();
  sh_shader.Use();
  // add attribute and uniform
  sh_shader.AddAttribute("vVertex");
  sh_shader.AddAttribute("vNormal");
  sh_shader.AddAttribute("vUV");
  sh_shader.AddUniform("MV");
  sh_shader.AddUniform("N");
  sh_shader.AddUniform("P");
  sh_shader.AddUniform("textureMap");
  sh_shader.AddUniform("light_position");
  sh_shader.AddUniform("useDefault");
  sh_shader.AddUniform("diffuse_color");
  // set values of constant uniforms as initialization
  glUniform1i(sh_shader("textureMap"), 0);
  sh_shader.UnUse();
  GL_CHECK_ERRORS;

  // load mesh rendering shader
  shader.LoadFromFile(GL_VERTEX_SHADER, "shaders/shader.vert");
  shader.LoadFromFile(GL_FRAGMENT_SHADER, "shaders/sh_shader.frag");
  shader.CreateAndLinkProgram();
  shader.Use();
  // add attribute and uniform
  shader.AddAttribute("vVertex");
  shader.AddAttribute("vNormal");
  shader.AddAttribute("vUV");
  shader.AddUniform("MV");
  shader.AddUniform("N");
  shader.AddUniform("P");
  shader.AddUniform("textureMap");
  shader.AddUniform("light_position");
  shader.AddUniform("useDefault");
  shader.AddUniform("diffuse_color");

  // set values of constant uniforms as initialization
  glUniform1i(shader("textureMap"), 0);
  shader.UnUse();
  GL_CHECK_ERRORS;

  // setup the vertex array object and vertex buffer object for the mesh
  // geometry handling
  glGenVertexArrays(1, &vaoID);
  glGenBuffers(1, &vboVerticesID);
  glGenBuffers(1, &vboIndicesID);

  glBindVertexArray(vaoID);
  glBindBuffer(GL_ARRAY_BUFFER, vboVerticesID);
  // pass mesh vertices
  glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertices.size(),
               &(vertices[0].pos.x), GL_STATIC_DRAW);
  GL_CHECK_ERRORS;
  // enable vertex attribute array for vertex position
  glEnableVertexAttribArray(sh_shader["vVertex"]);
  glVertexAttribPointer(sh_shader["vVertex"], 3, GL_FLOAT, GL_FALSE,
                        sizeof(Vertex), 0);
  GL_CHECK_ERRORS;
  // enable vertex attribute array for vertex normal
  glEnableVertexAttribArray(sh_shader["vNormal"]);
  glVertexAttribPointer(sh_shader["vNormal"], 3, GL_FLOAT, GL_FALSE,
                        sizeof(Vertex),
                        (const GLvoid *)(offsetof(Vertex, normal)));
  GL_CHECK_ERRORS;
  // enable vertex attribute array for vertex texture coordinates
  glEnableVertexAttribArray(sh_shader["vUV"]);
  glVertexAttribPointer(sh_shader["vUV"], 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                        (const GLvoid *)(offsetof(Vertex, uv)));
  GL_CHECK_ERRORS;

  // if we have a single material, it means the 3ds model contains one mesh
  // we therefore load it into an element array buffer
  if (materials.size() == 1) {
    // pass indices to the element array buffer if there is a single material
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboIndicesID);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLushort) * indices.size(),
                 &(indices[0]), GL_STATIC_DRAW);
  }
  GL_CHECK_ERRORS;

  // set spherical harmonics as the current shader
  pCurrentShader = &sh_shader;

  // enable depth test and culling
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);

  // set the clear colour to corn blue
  glClearColor(0.5, 0.5, 1, 1);

  // get the light position
  lightPosOS.x = radius * cos(theta) * sin(phi);
  lightPosOS.y = radius * cos(phi);
  lightPosOS.z = radius * sin(theta) * sin(phi);

  // setup vao and vbo stuff for the light position crosshair
  glm::vec3 crossHairVertices[6];
  crossHairVertices[0] = glm::vec3(-0.5f, 0, 0);
  crossHairVertices[1] = glm::vec3(0.5f, 0, 0);
  crossHairVertices[2] = glm::vec3(0, -0.5f, 0);
  crossHairVertices[3] = glm::vec3(0, 0.5f, 0);
  crossHairVertices[4] = glm::vec3(0, 0, -0.5f);
  crossHairVertices[5] = glm::vec3(0, 0, 0.5f);

  // setup light gizmo vertex array and vertex buffer object IDs
  glGenVertexArrays(1, &lightVAOID);
  glGenBuffers(1, &lightVerticesVBO);
  glBindVertexArray(lightVAOID);

  glBindBuffer(GL_ARRAY_BUFFER, lightVerticesVBO);
  // pass crosshair vertices to the buffer object
  glBufferData(GL_ARRAY_BUFFER, sizeof(crossHairVertices),
               &(crossHairVertices[0].x), GL_STATIC_DRAW);
  GL_CHECK_ERRORS;

  // enable vertex attribute array for vertex position
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
  GL_CHECK_ERRORS;

  std::cout << "Initialization successfull\n";
}

void OnResize(int w, int h) {
  // set the viewport
  glViewport(0, 0, (GLsizei)w, (GLsizei)h);
  // setup the projection matrix
  P = glm::perspective(60.0f, (float)w / h, 0.1f, 1000.0f);
}

void OnRender() {
  // clear the colour and depth buffer
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // set the viewing transformation
  glm::mat4 T = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, dist));
  glm::mat4 Rx = glm::rotate(T, rX, glm::vec3(1.0f, 0.0f, 0.0f));
  glm::mat4 MV = glm::rotate(Rx, rY, glm::vec3(0.0f, 1.0f, 0.0f));

  // bind the mesh vertex array object
  glBindVertexArray(vaoID);
  {
    // bind the current shader
    pCurrentShader->Use();
    // pass the shader uniforms
    glUniformMatrix4fv((*pCurrentShader)("MV"), 1, GL_FALSE,
                       glm::value_ptr(MV));
    glUniformMatrix3fv((*pCurrentShader)("N"), 1, GL_FALSE,
                       glm::value_ptr(glm::inverseTranspose(glm::mat3(MV))));
    glUniformMatrix4fv((*pCurrentShader)("P"), 1, GL_FALSE, glm::value_ptr(P));
    glUniform3fv((*pCurrentShader)("light_position"), 1, &(lightPosOS.x));

    // loop through all materials
    for (size_t i = 0; i < materials.size(); i++) {
      Material *pMat = materials[i];
      // if material texture filename is not empty
      if (pMat->map_Kd != "") {
        glUniform1f((*pCurrentShader)("useDefault"), 0.0);

        // get the currently bound texture and check if the current texture ID
        // is not equal, if so bind the new texture
        GLint whichID[1];
        glGetIntegerv(GL_TEXTURE_BINDING_2D, whichID);
        if (whichID[0] != textures[i])
          glBindTexture(GL_TEXTURE_2D, textures[i]);
      } else
        // otherwise we have no texture, we use a defaul colour
        glUniform1f((*pCurrentShader)("useDefault"), 1.0);

      // if we have a single material, we render the whole mesh in a single call
      if (materials.size() == 1)
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_SHORT, 0);
      else
        // otherwise we render the submesh
        glDrawElements(GL_TRIANGLES, pMat->count, GL_UNSIGNED_SHORT,
                       (const GLvoid *)(&indices[pMat->offset]));
    }

    // unbind the current shader
    pCurrentShader->UnUse();
  }

  // draw the light gizmo
  glBindVertexArray(lightVAOID);
  {
    // set the modelling transform for the light crosshair gizmo
    glm::mat4 T = glm::translate(glm::mat4(1), lightPosOS);
    // bind the shader
    flatShader.Use();
    // set shader uniforms and draw lines
    glUniformMatrix4fv(flatShader("MVP"), 1, GL_FALSE,
                       glm::value_ptr(P * MV * T));
    glDrawArrays(GL_LINES, 0, 6);

    // unbind the shader
    flatShader.UnUse();
  }

  // swap front and back buffers to show the rendered result
  glutSwapBuffers();
}

void OnShutdown() {
  // delete all textures
  size_t total_textures = textures.size();
  for (size_t i = 0; i < total_textures; i++) {
    glDeleteTextures(1, &textures[i]);
  }
  textures.clear();

  // delete all meshes
  size_t total_meshes = meshes.size();
  for (size_t i = 0; i < total_meshes; i++) {
    delete meshes[i];
    meshes[i] = 0;
  }
  meshes.clear();

  size_t total_materials = materials.size();
  for (size_t i = 0; i < total_materials; i++) {
    delete materials[i];
    materials[i] = 0;
  }
  materials.clear();

  // Destroy shader
  pCurrentShader = NULL;
  flatShader.DeleteShaderProgram();
  sh_shader.DeleteShaderProgram();
  shader.DeleteShaderProgram();

  // Destroy vao and vbo
  glDeleteBuffers(1, &vboVerticesID);
  glDeleteBuffers(1, &vboIndicesID);
  glDeleteVertexArrays(1, &vaoID);

  glDeleteVertexArrays(1, &lightVAOID);
  glDeleteBuffers(1, &lightVerticesVBO);

  cout << "Shutdown successfull" << endl;
}
