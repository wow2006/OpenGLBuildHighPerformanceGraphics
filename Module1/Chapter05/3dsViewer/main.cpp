// GLEW
#include <GL/glew.h>
// Freeglut
#include <GL/freeglut.h>
// STL
#include <iostream>
#include <string>
#include <vector>
// GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>
// SOIL
#include <SOIL.h>
// Internal
#include "3ds.hpp"
#include "GLSLShader.hpp"

#define GL_CHECK_ERRORS assert(glGetError() == GL_NO_ERROR)

namespace {
// output screen resolution
static constexpr int WIDTH = 1280;
static constexpr int HEIGHT = 960;
// shaders for use in the recipe
// one for model shading and second for point light gizmo
GLSLShader shader, flatShader;
// 3DS mesh filename to load
static const std::string mesh_filename = "media/blocks.3ds";
// 3dsloader instance
static C3dsLoader loader;
// IDs for vertex array and buffer object
// here we will load each attribute in a separate buffer object
GLuint vaoID;         // mesh vertex array object
GLuint vboVerticesID; // mesh vertices buffer object
GLuint vboUVsID;      // mesh texture coordinates buffer object
GLuint vboNormalsID;  // mesh normals buffer object
GLuint vboIndicesID;  // mesh indices element array buffer object

// projection and modelview matrices
glm::mat4 P = glm::mat4(1);
glm::mat4 MV = glm::mat4(1);

std::vector<C3dsMesh *> meshes;    // vector of meshes in 3DS file
std::vector<Material *> materials; // vector of materials
std::map<std::string, GLuint>
    textureMaps; // map of texture filename and OpenGL texture ID
typedef std::map<std::string, GLuint>::iterator
    iter;                            // iterator for texture map
std::vector<glm::vec3> vertices;     // mesh vertices
std::vector<glm::vec3> normals;      // mesh normals
std::vector<glm::vec2> uvs;          // mesh texture coordinates
std::vector<Face> faces;             // mesh faces (triangles)
std::vector<unsigned short> indices; // mesh indices

// camera transform variables
int state = 0, oldX = 0, oldY = 0;
float rX = -68, rY = 33, dist = -2;

// spherical cooridate variables for light rotation
float theta = 2.0f;
float phi = 2.0f;
float radius = 70;

// light crosshair gizmo vetex array and buffer object IDs
GLuint lightVAOID;
GLuint lightVerticesVBO;
glm::vec3 lightPosOS = glm::vec3(0, 2, 0); // objectspace light position
}                                         // namespace

// mouse click handler
void OnMouseDown(int button, int s, int x, int y) {
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
void OnMouseMove(int x, int y) {
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

  // recall display function
  glutPostRedisplay();
}

// OpenGL initialization
void OnInit() {
  // get the mesh path for loading of textures
  std::string mesh_path =
      mesh_filename.substr(0, mesh_filename.find_last_of("/") + 1);

  // load the 3DS file
  if (!loader.Load3DS(mesh_filename.c_str(), meshes, vertices, normals, uvs,
                      faces, indices, materials)) {
    std::cout << "Cannot load the 3ds mesh" << std::endl;
    exit(EXIT_FAILURE);
  }
  GL_CHECK_ERRORS;

  // load material textures
  // loop through all materials
  for (size_t k = 0; k < materials.size(); k++) {
    // loop through all material texturemaps
    for (size_t m = 0; m < materials[k]->textureMaps.size(); m++) {
      GLuint id = 0;
      // generate the OpenGL texture and set texture parameters
      glGenTextures(1, &id);
      glBindTexture(GL_TEXTURE_2D, id);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
      int texture_width = 0, texture_height = 0, channels = 0;

      const auto &filename = materials[k]->textureMaps[m]->filename;
      std::string full_filename = mesh_path;
      full_filename.append(filename);

      // use SOIL to load the image
      GLubyte *pData =
          SOIL_load_image(full_filename.c_str(), &texture_width,
                          &texture_height, &channels, SOIL_LOAD_AUTO);
      if (pData == NULL) {
        std::cerr << "Cannot load image: " << full_filename.c_str()
                  << std::endl;
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
      // allocate texture
      glTexImage2D(GL_TEXTURE_2D, 0, format, texture_width, texture_height, 0,
                   format, GL_UNSIGNED_BYTE, pData);

      // free SOIL image data
      SOIL_free_image_data(pData);

      // store texture id in map
      textureMaps[filename] = id;
    }
  }

  GL_CHECK_ERRORS;

  // load flat shader
  flatShader.LoadFromFile(GL_VERTEX_SHADER, "shaders/flat.vert");
  flatShader.LoadFromFile(GL_FRAGMENT_SHADER, "shaders/flat.frag");
  // compile and link
  flatShader.CreateAndLinkProgram();
  flatShader.Use();
  // add attribute and uniform
  flatShader.AddAttribute("vVertex");
  flatShader.AddUniform("MVP");
  flatShader.UnUse();

  // load mesh rendering shader
  shader.LoadFromFile(GL_VERTEX_SHADER, "shaders/shader.vert");
  shader.LoadFromFile(GL_FRAGMENT_SHADER, "shaders/shader.frag");
  // compile and link
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
  shader.AddUniform("hasTexture");
  shader.AddUniform("light_position");
  shader.AddUniform("diffuse_color");
  // set values of constant uniforms as initialization
  glUniform1i(shader("textureMap"), 0);
  shader.UnUse();

  GL_CHECK_ERRORS;

  // setup the vertex array object and vertex buffer object for the mesh
  // geometry handling
  glGenVertexArrays(1, &vaoID);
  glGenBuffers(1, &vboVerticesID);
  glGenBuffers(1, &vboUVsID);
  glGenBuffers(1, &vboNormalsID);
  glGenBuffers(1, &vboIndicesID);

  // get the mesh bounding box
  glm::vec3 min = glm::vec3(1000.0f), max = glm::vec3(-1000);
  for (size_t j = 0; j < meshes.size(); j++) {
    C3dsMesh *pMesh = meshes[j];
    for (size_t i = 0; i < pMesh->vertices.size(); i++) {
      min = glm::min(pMesh->vertices[i], min);
      max = glm::max(pMesh->vertices[i], max);
    }
  }
  // use the bounding information to move the camera such that the whole
  // mesh is visible on screen
  glm::vec3 center = (max + min) / 2.0f;
  float r = std::max(glm::distance(center, max), glm::distance(center, min));
  dist = -(r + (r * .5f));

  // bind mesh vertex array object
  glBindVertexArray(vaoID);
  glBindBuffer(GL_ARRAY_BUFFER, vboVerticesID);
  // pass mesh vertices data to buffer object
  glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * vertices.size(),
               &(vertices[0].x), GL_STATIC_DRAW);

  GL_CHECK_ERRORS;
  // enable vertex attribute array for vertex position
  glEnableVertexAttribArray(shader["vVertex"]);
  glVertexAttribPointer(shader["vVertex"], 3, GL_FLOAT, GL_FALSE, 0, 0);

  GL_CHECK_ERRORS;

  // bind the texture coordinate buffer object and pass the texture coordinate
  // to buffer object
  glBindBuffer(GL_ARRAY_BUFFER, vboUVsID);
  glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * uvs.size(), &(uvs[0].x),
               GL_STATIC_DRAW);

  GL_CHECK_ERRORS;
  // enable vertex attribute array for vertex texture coordinates
  glEnableVertexAttribArray(shader["vUV"]);
  glVertexAttribPointer(shader["vUV"], 2, GL_FLOAT, GL_FALSE, 0, 0);

  GL_CHECK_ERRORS;

  // bind the normals buffer object and pass the normals to buffer object
  glBindBuffer(GL_ARRAY_BUFFER, vboNormalsID);
  glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * normals.size(),
               &(normals[0].x), GL_STATIC_DRAW);

  GL_CHECK_ERRORS;
  // enable vertex attribute array for vertex normals
  glEnableVertexAttribArray(shader["vNormal"]);
  glVertexAttribPointer(shader["vNormal"], 3, GL_FLOAT, GL_FALSE, 0, 0);

  GL_CHECK_ERRORS;

  // if we have a single material, it means the 3ds model contains one mesh
  // we therefore load it into an element array buffer
  if (materials.size() == 1) {
    // pass indices to the element array buffer if there is a single material
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboIndicesID);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLushort) * 3 * faces.size(),
                 0, GL_STATIC_DRAW);

    // fill the element array buffer memory with the indices array
    GLushort *pIndices = static_cast<GLushort *>(
        glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY));
    for (size_t i = 0; i < faces.size(); i++) {
      *(pIndices++) = faces[i].a;
      *(pIndices++) = faces[i].b;
      *(pIndices++) = faces[i].c;
    }
    glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
  }

  GL_CHECK_ERRORS;

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
  // pass crosshair vertices to the buffer object
  glBindBuffer(GL_ARRAY_BUFFER, lightVerticesVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(crossHairVertices),
               &(crossHairVertices[0].x), GL_STATIC_DRAW);

  GL_CHECK_ERRORS;
  // enable vertex attribute array for vertex position
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

  GL_CHECK_ERRORS;

  // use spherical coordinates to get the light position
  lightPosOS.x = radius * cos(theta) * sin(phi);
  lightPosOS.y = radius * cos(phi);
  lightPosOS.z = radius * sin(theta) * sin(phi);

  // enable depth test and culling
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);

  // set clear color to corn blue
  glClearColor(0.5, 0.5, 1, 1);
  std::cout << "Initialization successfull" << std::endl;
}

// release all allocated resources
void OnShutdown() {}

// resize event handler
void OnResize(int w, int h) {
  // set the viewport
  glViewport(0, 0, (GLsizei)w, (GLsizei)h);
  // setup the projection matrix
  P = glm::perspective(60.0f, (float)w / h, 0.1f, 1000.0f);
}

// display callback function
void OnRender() {
  GL_CHECK_ERRORS;
  // clear colour and depth buffer
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // set the camera transformation
  glm::mat4 T = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, dist));
  glm::mat4 Rx = glm::rotate(T, rX, glm::vec3(1.0f, 0.0f, 0.0f));
  glm::mat4 MV = glm::rotate(Rx, rY, glm::vec3(0.0f, 0.0f, 1.0f));

  GL_CHECK_ERRORS;

  // bind the mesh vertex array object
  glBindVertexArray(vaoID);
  {
    // bind the mesh rendering shader
    shader.Use();
    // set shader uniforms
    glUniformMatrix4fv(shader("MV"), 1, GL_FALSE, glm::value_ptr(MV));
    glUniformMatrix3fv(shader("N"), 1, GL_FALSE,
                       glm::value_ptr(glm::inverseTranspose(glm::mat3(MV))));
    glUniformMatrix4fv(shader("P"), 1, GL_FALSE, glm::value_ptr(P));
    glUniform3fv(shader("light_position"), 1, &(lightPosOS.x));

    // if we have a single material, we render the whole mesh in a single call
    if (materials.size() == 1) {
      GLint whichID[1];
      glGetIntegerv(GL_TEXTURE_BINDING_2D, whichID);
      // if the material has texturemaps and it is not bound already
      // bind it and set the hasTexture variable to 1
      if (textureMaps.size() > 0) {
        if (static_cast<uint32_t>(whichID[0]) != textureMaps[materials[0]->textureMaps[0]->filename]) {
          glBindTexture(GL_TEXTURE_2D,
                        textureMaps[materials[0]->textureMaps[0]->filename]);
          glUniform1f(shader("hasTexture"), 1.0);
        }
      } else {
        // otherwise set hasTexture as 0 and use the material diffuse colour
        glUniform1f(shader("hasTexture"), 0.0);
        glUniform3fv(shader("diffuse_color"), 1, materials[0]->diffuse);
      }
      // draw mesh triangles in a single call
      glDrawElements(GL_TRIANGLES, meshes[0]->faces.size() * 3,
                     GL_UNSIGNED_SHORT, 0);
    } else {
      // otherwise we render the submeshes by material
      for (size_t i = 0; i < materials.size(); i++) {
        GLint whichID[1];
        glGetIntegerv(GL_TEXTURE_BINDING_2D, whichID);
        // if the material has texturemaps and it is not bound already
        // bind it and set the hasTexture variable to 1
        if (materials[i]->textureMaps.size() > 0) {
          if (static_cast<uint32_t>(whichID[0]) !=
              textureMaps[materials[i]->textureMaps[0]->filename]) {
            glBindTexture(GL_TEXTURE_2D,
                          textureMaps[materials[i]->textureMaps[0]->filename]);
          }
          glUniform1f(shader("hasTexture"), 1.0);
        } else {
          // otherwise set hasTexture as 0
          glUniform1f(shader("hasTexture"), 0.0);
        }
        // pass the diffuse colour uniform to the material's diffuse color
        glUniform3fv(shader("diffuse_color"), 1, materials[i]->diffuse);
        // draw triangles using the submesh indices
        glDrawElements(GL_TRIANGLES, materials[i]->sub_indices.size(),
                       GL_UNSIGNED_SHORT, &(materials[i]->sub_indices[0]));
      }
    }
    // unbind shader
    shader.UnUse();
  }

  // disable depth testing
  glDisable(GL_DEPTH_TEST);

  // draw light gizmo
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
  // enable depth test
  glEnable(GL_DEPTH_TEST);

  // swap front and back buffers to show the rendered result
  glutSwapBuffers();
}

int main(int argc, char *argv[]) {
  // freeglut initialization
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
  glutInitContextVersion(3, 3);
  glutInitContextFlags(GLUT_CORE_PROFILE | GLUT_DEBUG);
  glutInitWindowSize(WIDTH, HEIGHT);
  glutCreateWindow("3DS Viewer - OpenGL 3.3");

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

  // call main loop
  glutMainLoop();

  return 0;
}
