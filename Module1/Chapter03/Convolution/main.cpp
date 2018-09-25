#include <string>
#include <iostream>

#include <GL/glew.h>
#include <GL/freeglut.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <SOIL/SOIL.h>

#include "GLSLShader.hpp"

#define GL_CHECK_ERRORS assert(glGetError() == GL_NO_ERROR);

struct Common {
  // screen resolution
  static constexpr int WIDTH = 512;
  static constexpr int HEIGHT = 512;

  // g_pCommon->mShader for rendering of image and convolution
  GLSLShader mShader;
  GLSLShader mConvolutionShader;
  GLSLShader *m_pCurrent_shader;

  // vertex array and vertex buffer object IDs
  GLuint mVaoID;
  GLuint mVboVerticesID;
  GLuint mVboIndicesID;

  // texture image ID
  GLuint mTextureID;

  // vertices and indices arrays for fullscreen quad
  glm::vec2 m_vVertices[4];
  GLushort  m_vIndices[6];

  // texture image filename
  const std::string mFilename = "media/Lenna.png";
};
static Common *g_pCommon = nullptr;

void OnInit() {
  GL_CHECK_ERRORS
  // load g_pCommon->mShader
  g_pCommon->mShader.LoadFromFile(GL_VERTEX_SHADER, "shaders/shader.vert");
  g_pCommon->mShader.LoadFromFile(GL_FRAGMENT_SHADER, "shaders/shader.frag");
  // compile and link g_pCommon->mShader
  g_pCommon->mShader.CreateAndLinkProgram();
  g_pCommon->mShader.Use();
  // add attributes and uniforms
  g_pCommon->mShader.AddAttribute("vVertex");
  g_pCommon->mShader.AddUniform("textureMap");
  // pass values of constant uniforms at initialization
  glUniform1i(g_pCommon->mShader("textureMap"), 0);
  g_pCommon->mShader.UnUse();

  g_pCommon->m_pCurrent_shader = &g_pCommon->mShader;

  GL_CHECK_ERRORS

  // load g_pCommon->mShader
  g_pCommon->mConvolutionShader.LoadFromFile(GL_VERTEX_SHADER, "shaders/shader.vert");
  g_pCommon->mConvolutionShader.LoadFromFile(GL_FRAGMENT_SHADER,
                                  "shaders/shader_convolution.frag");
  // compile and link g_pCommon->mShader
  g_pCommon->mConvolutionShader.CreateAndLinkProgram();
  g_pCommon->mConvolutionShader.Use();
  // add attributes and uniforms
  g_pCommon->mConvolutionShader.AddAttribute("vVertex");
  g_pCommon->mConvolutionShader.AddUniform("textureMap");
  // pass values of constant uniforms at initialization
  glUniform1i(g_pCommon->mConvolutionShader("textureMap"), 0);
  g_pCommon->mConvolutionShader.UnUse();

  GL_CHECK_ERRORS

  // setup quad geometry
  // setup quad g_pCommon->m_vVertices
  g_pCommon->m_vVertices[0] = glm::vec2(0.0, 0.0);
  g_pCommon->m_vVertices[1] = glm::vec2(1.0, 0.0);
  g_pCommon->m_vVertices[2] = glm::vec2(1.0, 1.0);
  g_pCommon->m_vVertices[3] = glm::vec2(0.0, 1.0);

  // fill quad g_pCommon->m_vIndices array
  GLushort *id = &g_pCommon->m_vIndices[0];
  *id++ = 0;
  *id++ = 1;
  *id++ = 2;
  *id++ = 0;
  *id++ = 2;
  *id++ = 3;

  GL_CHECK_ERRORS

  // setup quad vao and vbo stuff
  glGenVertexArrays(1, &g_pCommon->mVaoID);
  glGenBuffers(1, &g_pCommon->mVboVerticesID);
  glGenBuffers(1, &g_pCommon->mVboIndicesID);

  glBindVertexArray(g_pCommon->mVaoID);

  glBindBuffer(GL_ARRAY_BUFFER, g_pCommon->mVboVerticesID);
  // pass quad g_pCommon->m_vVertices to buffer object
  glBufferData(GL_ARRAY_BUFFER, sizeof(g_pCommon->m_vVertices), &g_pCommon->m_vVertices[0], GL_STATIC_DRAW);
  GL_CHECK_ERRORS
  // enable vertex attribute array for position
  glEnableVertexAttribArray(g_pCommon->mShader["vVertex"]);
  glVertexAttribPointer(g_pCommon->mShader["vVertex"], 2, GL_FLOAT, GL_FALSE, 0, nullptr);
  GL_CHECK_ERRORS
  // pass quad g_pCommon->m_vIndices to element array buffer
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_pCommon->mVboIndicesID);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(g_pCommon->m_vIndices), &g_pCommon->m_vIndices[0],
               GL_STATIC_DRAW);
  GL_CHECK_ERRORS

  // load the image using SOIL
  int texture_width = 0, texture_height = 0, channels = 0;
  GLubyte *pData = SOIL_load_image(g_pCommon->mFilename.c_str(), &texture_width,
                                   &texture_height, &channels, SOIL_LOAD_AUTO);

  // vertically flip the image on Y axis since it is inverted
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

  // setup OpenGL texture and bind to texture unit 0
  glGenTextures(1, &g_pCommon->mTextureID);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, g_pCommon->mTextureID);
  // set texture parameters
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
  // allocate texture
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texture_width, texture_height, 0,
               GL_RGB, GL_UNSIGNED_BYTE, pData);

  // free SOIL image data
  SOIL_free_image_data(pData);

  GL_CHECK_ERRORS

  std::cout << "Initialization successfull" << std::endl;
}

// release all allocated resources
void OnShutdown() {
  g_pCommon->m_pCurrent_shader = nullptr;

  // Destroy g_pCommon->mShader
  g_pCommon->mShader.DeleteShaderProgram();
  g_pCommon->mConvolutionShader.DeleteShaderProgram();

  // Destroy vao and vbo
  glDeleteBuffers(1, &g_pCommon->mVboVerticesID);
  glDeleteBuffers(1, &g_pCommon->mVboIndicesID);
  glDeleteVertexArrays(1, &g_pCommon->mVaoID);

  // Delete textures
  glDeleteTextures(1, &g_pCommon->mTextureID);
  std::cout << "Shutdown successfull" << std::endl;
}

// resize event handler
void OnResize(int w, int h) {
  // set the viewport
  glViewport(0, 0, static_cast<GLsizei>(w),
                   static_cast<GLsizei>(h));
}

// display function
void OnRender() {
  // clear the colour and depth buffers
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // bind current g_pCommon->mShader
  g_pCommon->m_pCurrent_shader->Use();
  // draw fullscreen quad
  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr);
  // unbind g_pCommon->mShader
  g_pCommon->m_pCurrent_shader->UnUse();

  // swap front and back buffers to show the rendered result
  glutSwapBuffers();
}

// keyboard event handler to change the output to convolved or normal image
void OnKey(unsigned char key, int /*x*/, int /*y*/) {
  switch (key) {
  case ' ': {
    if (g_pCommon->m_pCurrent_shader == &g_pCommon->mShader) {
      g_pCommon->m_pCurrent_shader = &g_pCommon->mConvolutionShader;
      glutSetWindowTitle("Filtered image");
    } else {
      g_pCommon->m_pCurrent_shader = &g_pCommon->mShader;
      glutSetWindowTitle("Normal image");
    }
  } break;
  }
  // call display function
  glutPostRedisplay();
}

int main(int argc, char **argv) {
  Common common;
  g_pCommon = &common;

  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
  glutInitContextVersion(3, 3);
  glutInitContextFlags(GLUT_CORE_PROFILE | GLUT_DEBUG);
  glutInitWindowSize(Common::WIDTH, Common::HEIGHT);
  glutCreateWindow("Convolution - OpenGL 3.3");

  // glew initialization
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
  GL_CHECK_ERRORS

  // print information on screen
  std::cout << "\tUsing GLEW " << glewGetString(GLEW_VERSION) << std::endl;
  std::cout << "\tVendor: " << glGetString(GL_VENDOR) << std::endl;
  std::cout << "\tRenderer: " << glGetString(GL_RENDERER) << std::endl;
  std::cout << "\tVersion: " << glGetString(GL_VERSION) << std::endl;
  std::cout << "\tGLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;

  std::cout << "Press ' ' key to filter/unfilter\n";
  GL_CHECK_ERRORS

  // initialization of OpenGL
  OnInit();

  // callback hooks
  glutCloseFunc(OnShutdown);
  glutDisplayFunc(OnRender);
  glutReshapeFunc(OnResize);
  glutKeyboardFunc(OnKey);

  // main loop call
  glutMainLoop();

  return 0;
}
