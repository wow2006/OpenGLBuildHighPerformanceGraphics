// STL
#include <cassert>
#include <cstdlib>
#include <iostream>
// GLEW
#include <GL/glew.h>
// GLUT
#include <GL/freeglut.h>
// GLM
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/matrix_transform.hpp>
// SOIL
#include <SOIL/SOIL.h>
// Internal
#include "Ezm.hpp"
#include "GLSLShader.hpp"

#define GL_CHECK_ERRORS assert(glGetError() == GL_NO_ERROR)

// screen size
const int WIDTH = 1280;
const int HEIGHT = 960;

// shaders for mesh drawing and light gizmo
GLSLShader shader, flatShader;

static const std::string mesh_filename = "media/dudeMesh.ezm";

glm::vec3 center;

// EZMesh loader instance
static EzmLoader ezm;
std::vector<SubMesh> submeshes;            // all submeshes in the EZMesh file
std::map<std::string, GLuint> materialMap; // material name, texture id map
std::map<std::string, std::string> material2ImageMap; // material name, image name map
typedef std::map<std::string, std::string>::iterator iter; // material2map iterator

// mesh vertices and indices
std::vector<Vertex> vertices;
std::vector<unsigned short> indices;

// All material names in the EZMesh model file in a linear list
std::vector<std::string> materialNames;

// light gizmo vertex arrary and buffer object
GLuint lightVAOID;
GLuint lightVerticesVBO;

// objectspace light position
glm::vec3 lightPosOS = glm::vec3(0, 2, 0); // objectspace light position

// spherical coordinates for rotating the light source
float theta = 1.1f;
float phi = 0.85f;
float radius = 70;

// camera transformation variables
int state = 0, oldX = 0, oldY = 0;
float rX = 0, rY = 0, dist = -120;

// vertex arrray and buffer object ids
GLuint vaoID;
GLuint vboVerticesID;
GLuint vboIndicesID;

// OpenGL initialization
void OnInit();

auto main(int argc, char *argv[]) -> int {
  // freeglut initialization calls
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
  glutInitContextVersion(3, 3);
  glutInitContextFlags(GLUT_CORE_PROFILE | GLUT_DEBUG);
  glutInitWindowSize(WIDTH, HEIGHT);
  glutCreateWindow("EZMesh Skeletal Mesh Viewer - OpenGL 3.3");

  // glew initialization
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

  // print information on screen
  std::cout << "\tUsing GLEW " << glewGetString(GLEW_VERSION) << std::endl;
  std::cout << "\tVendor: " << glGetString(GL_VENDOR) << std::endl;
  std::cout << "\tRenderer: " << glGetString(GL_RENDERER) << std::endl;
  std::cout << "\tVersion: " << glGetString(GL_VERSION) << std::endl;
  std::cout << "\tGLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION)
            << std::endl;
  GL_CHECK_ERRORS;

  // initialization of OpenGL
  OnInit();

  // callback hooks
  //glutCloseFunc(OnShutdown);
  //glutDisplayFunc(OnRender);
  //glutReshapeFunc(OnResize);
  //glutMouseFunc(OnMouseDown);
  //glutMotionFunc(OnMouseMove);
  //glutMouseWheelFunc(OnMouseWheel);

  // main loop call
  glutMainLoop();

  return EXIT_SUCCESS;
}

void OnInit() {
  // get mesh path
  std::string mesh_path =
      mesh_filename.substr(0, mesh_filename.find_last_of("//") + 1);

  glm::vec3 min, max;
  // load the EZmesh file
  if (!ezm.Load(mesh_filename.c_str(), submeshes, vertices, indices,
                material2ImageMap, min, max)) {
    std::cout << "Cannot load the 3ds mesh" << std::endl;
    exit(EXIT_FAILURE);
  }
  GL_CHECK_ERRORS;

  // store the loaded material names into a vector
  for (iter i = material2ImageMap.begin(); i != material2ImageMap.end(); ++i) {
    materialNames.push_back(i->second);
  }

  // calculate the distance the camera has to be moved to properly view the
  // EZMesh model
  center = (max + min) * 0.5f;
  glm::vec3 diagonal = (max - min);
  radius = glm::length(center - diagonal * 0.5f);
  dist = -glm::length(diagonal);

  // generate OpenGL textures from the loaded material names
  for (size_t k = 0; k < materialNames.size(); k++) {
    if (materialNames[k].length() == 0)
      continue;

    // generate OpenGL texture object
    GLuint id = 0;
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    int texture_width = 0, texture_height = 0, channels = 0;

    // get the full image name
    const std::string &filename = materialNames[k];
    std::string full_filename = mesh_path;
    full_filename.append(filename);

    // use SOIL to load the image
    GLubyte *pData =
        SOIL_load_image(full_filename.c_str(), &texture_width, &texture_height,
                        &channels, SOIL_LOAD_AUTO);
    if (pData == NULL) {
      std::cerr << "Cannot load image: " << full_filename.c_str() << std::endl;
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

    // determine the image format
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

    // delete the SOIL image data
    SOIL_free_image_data(pData);

    // store the texture id into the material map. Refer to the texture by name
    // will give us its OpenGL texture id
    materialMap[filename] = id;
  }
  GL_CHECK_ERRORS;

  // load flat shader
  flatShader.LoadFromFile(GL_VERTEX_SHADER, "shaders/flat.vert");
  flatShader.LoadFromFile(GL_FRAGMENT_SHADER, "shaders/flat.frag");
  // compile and link flat shader
  flatShader.CreateAndLinkProgram();
  flatShader.Use();
  // add shader attributes and uniforms
  flatShader.AddAttribute("vVertex");
  flatShader.AddUniform("MVP");
  flatShader.UnUse();

  shader.LoadFromFile(GL_VERTEX_SHADER, "shaders/shader.vert");
  shader.LoadFromFile(GL_FRAGMENT_SHADER, "shaders/shader.frag");
  // compile and link flat shader
  shader.CreateAndLinkProgram();
  shader.Use();
  // add shader attributes and uniforms
  shader.AddAttribute("vVertex");
  shader.AddAttribute("vNormal");
  shader.AddAttribute("vUV");
  shader.AddUniform("MV");
  shader.AddUniform("N");
  shader.AddUniform("P");
  shader.AddUniform("textureMap");
  shader.AddUniform("useDefault");
  shader.AddUniform("light_position");
  shader.AddUniform("diffuse_color");
  // set values of constant shader uniforms at initialization
  glUniform1i(shader("textureMap"), 0);
  shader.UnUse();

  GL_CHECK_ERRORS;

  // setup geometry
  // setup vao and vbo stuff
  glGenVertexArrays(1, &vaoID);
  glGenBuffers(1, &vboVerticesID);
  glGenBuffers(1, &vboIndicesID);

  glBindVertexArray(vaoID);
  glBindBuffer(GL_ARRAY_BUFFER, vboVerticesID);
  // pass vertices to buffer object memory
  glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertices.size(),
               &(vertices[0].pos.x), GL_DYNAMIC_DRAW);
  GL_CHECK_ERRORS;
  // enable vertex attribute for position
  glEnableVertexAttribArray(shader["vVertex"]);
  glVertexAttribPointer(shader["vVertex"], 3, GL_FLOAT, GL_FALSE,
                        sizeof(Vertex), 0);
  GL_CHECK_ERRORS;

  // enable vertex attribute for normals
  glEnableVertexAttribArray(shader["vNormal"]);
  glVertexAttribPointer(shader["vNormal"], 3, GL_FLOAT, GL_FALSE,
                        sizeof(Vertex),
                        (const GLvoid *)(offsetof(Vertex, normal)));

  GL_CHECK_ERRORS;
  // enable vertex attribute array for texture coordinate
  glEnableVertexAttribArray(shader["vUV"]);
  glVertexAttribPointer(shader["vUV"], 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                        (const GLvoid *)(offsetof(Vertex, uv)));
  GL_CHECK_ERRORS;

  // glBindVertexArray(0);

  // setup vao and vbo stuff for the light position crosshair
  glm::vec3 crossHairVertices[6];
  crossHairVertices[0] = glm::vec3(-0.5f, 0, 0);
  crossHairVertices[1] = glm::vec3(0.5f, 0, 0);
  crossHairVertices[2] = glm::vec3(0, -0.5f, 0);
  crossHairVertices[3] = glm::vec3(0, 0.5f, 0);
  crossHairVertices[4] = glm::vec3(0, 0, -0.5f);
  crossHairVertices[5] = glm::vec3(0, 0, 0.5f);

  // generate light vertex array and buffer object
  glGenVertexArrays(1, &lightVAOID);
  glGenBuffers(1, &lightVerticesVBO);
  glBindVertexArray(lightVAOID);
  glBindBuffer(GL_ARRAY_BUFFER, lightVerticesVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(crossHairVertices),
               &(crossHairVertices[0].x), GL_STATIC_DRAW);
  GL_CHECK_ERRORS;

  // enable vertex attribute array
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
  GL_CHECK_ERRORS;

  // get the light position using the center and the spherical coordinates
  lightPosOS.x = center.x + radius * cos(theta) * sin(phi);
  lightPosOS.y = center.y + radius * cos(phi);
  lightPosOS.z = center.z + radius * sin(theta) * sin(phi);

  // enable depth test and culling
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);

  // set clear color to corn blue
  glClearColor(0.5, 0.5, 1, 1);
  std::cout << "Initialization successfull" << std::endl;
}
