// This is an open source non-commercial project. Dear PVS-Studio, please check
// it. PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include <GL/glew.h>

#include <GL/freeglut.h>

#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "GLSLShader.hpp"
#include "Grid.hpp"

#define GL_CHECK_ERRORS assert(glGetError() == GL_NO_ERROR)

struct Common {
  // set screen dimensions
  static constexpr int WIDTH = 1280;
  static constexpr int HEIGHT = 960;

  // total number of depth peeling passes
  static constexpr int NUM_PASSES = 6;

  // grid object
  CGrid *m_pGrid = nullptr;

  GLuint mQueryId;
  GLuint mFBO[2];
  GLuint mTexID[2];
  GLuint mDepthTexID[2];

  GLuint mColorBlenderTexID;
  GLuint mColorBlenderFBOID;

  GLuint mQuadVAOID;
  GLuint mQuadVBOID;
  GLuint mQuadIndicesID;

  GLuint mCubeVAOID;
  GLuint mCubeVBOID;
  GLuint mCubeIndicesID;

  GLSLShader mCubeShader;
  GLSLShader mFrontPeelShader;
  GLSLShader mBlendShader;
  GLSLShader mFinalShader;

  // camera transform variables
  int mState = 0;
  int mOldX = 0;
  int mOldY = 0;
  float rX = 0.f;
  float rY = 300.f;
  float mDist = -10.f;

  // constants for box colours
  glm::vec4 mBox_colors[3] = {glm::vec4(1, 0, 0, 0.5), glm::vec4(0, 1, 0, 0.5),
                              glm::vec4(0, 0, 1, 0.5)};

  // modelview projection and rotation matrices
  glm::mat4 mMV, mP, mR;
  bool m_bShowDepthPeeling;
  bool m_bUseOQ;

  glm::vec4 bg = glm::vec4(0, 0, 0, 0);
  float mAngle = 0.f;
};
static Common *g_pCommon = nullptr;

// FBO initialization function
void initFBO() {
  // generate 2 FBO
  glGenFramebuffers(2, g_pCommon->mFBO);
  // The FBO has two colour attachments
  glGenTextures(2, g_pCommon->mTexID);
  // The FBO has two depth attachments
  glGenTextures(2, g_pCommon->mDepthTexID);

  // for each attachment
  for (int i = 0; i < 2; i++) {
    // first initialize the depth texture
    glBindTexture(GL_TEXTURE_RECTANGLE, g_pCommon->mDepthTexID[i]);
    glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_DEPTH_COMPONENT32F, Common::WIDTH,
                 Common::HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);

    // second initialize the colour attachment
    glBindTexture(GL_TEXTURE_RECTANGLE, g_pCommon->mTexID[i]);
    glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_RGBA, Common::WIDTH,
                 Common::HEIGHT, 0, GL_RGBA, GL_FLOAT, nullptr);

    // bind FBO and attach the depth and colour attachments
    glBindFramebuffer(GL_FRAMEBUFFER, g_pCommon->mFBO[i]);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                           GL_TEXTURE_RECTANGLE, g_pCommon->mDepthTexID[i], 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_RECTANGLE, g_pCommon->mTexID[i], 0);
  }

  // Now setup the colour attachment for colour blend FBO
  glGenTextures(1, &g_pCommon->mColorBlenderTexID);
  glBindTexture(GL_TEXTURE_RECTANGLE, g_pCommon->mColorBlenderTexID);
  glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_T, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_RGBA, Common::WIDTH, Common::HEIGHT,
               0, GL_RGBA, GL_FLOAT, nullptr);

  // generate the colour blend FBO ID
  glGenFramebuffers(1, &g_pCommon->mColorBlenderFBOID);
  glBindFramebuffer(GL_FRAMEBUFFER, g_pCommon->mColorBlenderFBOID);

  // set the depth attachment of previous FBO as depth attachment for this FBO
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                         GL_TEXTURE_RECTANGLE, g_pCommon->mDepthTexID[0], 0);
  // set the colour blender texture as the FBO colour attachment
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                         GL_TEXTURE_RECTANGLE, g_pCommon->mColorBlenderTexID,
                         0);

  // check the FBO completeness status
  GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
  if (status == GL_FRAMEBUFFER_COMPLETE) {
    printf("FBO setup successful !!! \n");
  } else {
    printf("Problem with FBO setup");
  }

  // unbind FBO
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

// OpenGL initialization function
void OnInit() {
  // Initialize FBO
  initFBO();

  // Generate hardwre query
  glGenQueries(1, &g_pCommon->mQueryId);

  // Create a uniform grid of size 20x20 in XZ plane
  g_pCommon->m_pGrid = new CGrid(20, 20);
  GL_CHECK_ERRORS;

  // generate the quad vertices
  glm::vec2 quadVerts[4];
  quadVerts[0] = glm::vec2(0, 0);
  quadVerts[1] = glm::vec2(1, 0);
  quadVerts[2] = glm::vec2(1, 1);
  quadVerts[3] = glm::vec2(0, 1);

  // generate quad indices
  GLushort quadIndices[] = {0, 1, 2, 0, 2, 3};

  // generate quad  vertex array and vertex buffer objects
  glGenVertexArrays(1, &g_pCommon->mQuadVAOID);
  glGenBuffers(1, &g_pCommon->mQuadVBOID);
  glGenBuffers(1, &g_pCommon->mQuadIndicesID);

  glBindVertexArray(g_pCommon->mQuadVAOID);
  glBindBuffer(GL_ARRAY_BUFFER, g_pCommon->mQuadVBOID);
  // pass quad vertices to buffer object memory
  glBufferData(GL_ARRAY_BUFFER, sizeof(quadVerts), &quadVerts[0],
               GL_STATIC_DRAW);
  GL_CHECK_ERRORS;

  // enable vertex attribute array for position
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

  // pass the quad indices to the element array buffer
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_pCommon->mQuadIndicesID);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(quadIndices), &quadIndices[0],
               GL_STATIC_DRAW);

  // setup unit cube vertex array and vertex buffer objects
  glGenVertexArrays(1, &g_pCommon->mCubeVAOID);
  glGenBuffers(1, &g_pCommon->mCubeVBOID);
  glGenBuffers(1, &g_pCommon->mCubeIndicesID);

  // unit cube vertices
  glm::vec3 vertices[8] = {
      glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(0.5f, -0.5f, -0.5f),
      glm::vec3(0.5f, 0.5f, -0.5f),   glm::vec3(-0.5f, 0.5f, -0.5f),
      glm::vec3(-0.5f, -0.5f, 0.5f),  glm::vec3(0.5f, -0.5f, 0.5f),
      glm::vec3(0.5f, 0.5f, 0.5f),    glm::vec3(-0.5f, 0.5f, 0.5f)};

  // unit cube indices
  GLushort cubeIndices[36] = {0, 5, 4, 5, 0, 1, 3, 7, 6, 3, 6, 2,
                              7, 4, 6, 6, 4, 5, 2, 1, 3, 3, 1, 0,
                              3, 0, 7, 7, 0, 4, 6, 5, 2, 2, 5, 1};

  glBindVertexArray(g_pCommon->mCubeVAOID);
  glBindBuffer(GL_ARRAY_BUFFER, g_pCommon->mCubeVBOID);
  // pass cube vertices to buffer object memory
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &(vertices[0].x),
               GL_STATIC_DRAW);
  GL_CHECK_ERRORS;

  // enable vertex attributre array for position
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

  // pass cube indices to element array  buffer
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_pCommon->mCubeIndicesID);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cubeIndices), &cubeIndices[0],
               GL_STATIC_DRAW);
  glBindVertexArray(0);

  // Load the cube shader
  g_pCommon->mCubeShader.LoadFromFile(GL_VERTEX_SHADER,
                                      "shaders/cube_shader.vert");
  g_pCommon->mCubeShader.LoadFromFile(GL_FRAGMENT_SHADER,
                                      "shaders/cube_shader.frag");

  // compile and link the shader
  g_pCommon->mCubeShader.CreateAndLinkProgram();
  g_pCommon->mCubeShader.Use();
  // add attributes and uniforms
  g_pCommon->mCubeShader.AddAttribute("vVertex");
  g_pCommon->mCubeShader.AddUniform("MVP");
  g_pCommon->mCubeShader.AddUniform("vColor");
  g_pCommon->mCubeShader.UnUse();

  // Load the front to back peeling shader
  g_pCommon->mFrontPeelShader.LoadFromFile(GL_VERTEX_SHADER,
                                           "shaders/front_peel.vert");
  g_pCommon->mFrontPeelShader.LoadFromFile(GL_FRAGMENT_SHADER,
                                           "shaders/front_peel.frag");
  // compile and link the shader
  g_pCommon->mFrontPeelShader.CreateAndLinkProgram();
  g_pCommon->mFrontPeelShader.Use();
  // add attributes and uniforms
  g_pCommon->mFrontPeelShader.AddAttribute("vVertex");
  g_pCommon->mFrontPeelShader.AddUniform("MVP");
  g_pCommon->mFrontPeelShader.AddUniform("vColor");
  g_pCommon->mFrontPeelShader.AddUniform("depthTexture");
  // pass constant uniforms at initialization
  glUniform1i(g_pCommon->mFrontPeelShader("depthTexture"), 0);
  g_pCommon->mFrontPeelShader.UnUse();

  // Load the blending shader
  g_pCommon->mBlendShader.LoadFromFile(GL_VERTEX_SHADER, "shaders/blend.vert");
  g_pCommon->mBlendShader.LoadFromFile(GL_FRAGMENT_SHADER,
                                       "shaders/blend.frag");
  // compile and link the shader
  g_pCommon->mBlendShader.CreateAndLinkProgram();
  g_pCommon->mBlendShader.Use();
  // add attributes and uniforms
  g_pCommon->mBlendShader.AddAttribute("vVertex");
  g_pCommon->mBlendShader.AddUniform("tempTexture");
  // pass constant uniforms at initialization
  glUniform1i(g_pCommon->mBlendShader("tempTexture"), 0);
  g_pCommon->mBlendShader.UnUse();

  // Load the final shader
  g_pCommon->mFinalShader.LoadFromFile(GL_VERTEX_SHADER, "shaders/blend.vert");
  g_pCommon->mFinalShader.LoadFromFile(GL_FRAGMENT_SHADER,
                                       "shaders/final.frag");
  // compile and link the shader
  g_pCommon->mFinalShader.CreateAndLinkProgram();
  g_pCommon->mFinalShader.Use();
  // add attributes and uniforms
  g_pCommon->mFinalShader.AddAttribute("vVertex");
  g_pCommon->mFinalShader.AddUniform("colorTexture");
  g_pCommon->mFinalShader.AddUniform("vBackgroundColor");
  // pass constant uniforms at initialization
  glUniform1i(g_pCommon->mFinalShader("colorTexture"), 0);
  g_pCommon->mFinalShader.UnUse();
  std::cout << "Initialization successfull" << std::endl;
}

// Delete all FBO related resources
void shutdownFBO() {
  glDeleteFramebuffers(2, g_pCommon->mFBO);
  glDeleteTextures(2, g_pCommon->mTexID);
  glDeleteTextures(2, g_pCommon->mDepthTexID);
  glDeleteFramebuffers(1, &g_pCommon->mColorBlenderFBOID);
  glDeleteTextures(1, &g_pCommon->mColorBlenderTexID);
}

// Release all allocated resources
void OnShutdown() {
  g_pCommon->mCubeShader.DeleteShaderProgram();
  g_pCommon->mFrontPeelShader.DeleteShaderProgram();
  g_pCommon->mBlendShader.DeleteShaderProgram();
  g_pCommon->mFinalShader.DeleteShaderProgram();

  shutdownFBO();
  glDeleteQueries(1, &g_pCommon->mQueryId);

  glDeleteVertexArrays(1, &g_pCommon->mQuadVAOID);
  glDeleteBuffers(1, &g_pCommon->mQuadVBOID);
  glDeleteBuffers(1, &g_pCommon->mQuadIndicesID);

  glDeleteVertexArrays(1, &g_pCommon->mCubeVAOID);
  glDeleteBuffers(1, &g_pCommon->mCubeVBOID);
  glDeleteBuffers(1, &g_pCommon->mCubeIndicesID);

  delete g_pCommon->m_pGrid;
  g_pCommon->m_pGrid = nullptr;
  std::cout << "Shutdown successfull" << std::endl;
}

// function to draw a fullscreen quad
void DrawFullScreenQuad() {
  // bind the quad vertex array object
  glBindVertexArray(g_pCommon->mQuadVAOID);

  // draw 2 triangles
  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr);
}

// function to render scene given the combined modelview projection matrix
// and a shader
void DrawScene(const glm::mat4 &MVP, GLSLShader &shader) {
  // enable alpha blending with over compositing
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  // bind the cube vertex array object
  glBindVertexArray(g_pCommon->mCubeVAOID);
  // bind the shader
  shader.Use();
  // for all cubes
  for (int k = -1; k <= 1; k++) {
    for (int j = -1; j <= 1; j++) {
      int index = 0;
      for (int i = -1; i <= 1; i++) {
        GL_CHECK_ERRORS;
        // set the modelling transformation and shader uniforms
        glm::mat4 T =
            glm::translate(glm::mat4(1), glm::vec3(i * 2, j * 2, k * 2));
        glUniform4fv(shader("vColor"), 1, &(g_pCommon->mBox_colors[index++].x));
        glUniformMatrix4fv(shader("MVP"), 1, GL_FALSE,
                           glm::value_ptr(MVP * g_pCommon->mR * T));
        // draw the cube
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, nullptr);
        GL_CHECK_ERRORS;
      }
    }
  }
  // unbind shader
  shader.UnUse();
  // unbind vertex array object
  glBindVertexArray(0);
}

// Display callback function
void OnRender() {
  GL_CHECK_ERRORS;

  // camera transformation
  auto Tr =
      glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, g_pCommon->mDist));
  auto Rx = glm::rotate(Tr, g_pCommon->rX, glm::vec3(1.0f, 0.0f, 0.0f));
  auto MV = glm::rotate(Rx, g_pCommon->rY, glm::vec3(0.0f, 1.0f, 0.0f));

  // clear colour and depth buffer
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // get the combined modelview projection matrix
  glm::mat4 MVP = g_pCommon->mP * MV;

  // if we want to use depth peeling
  if (g_pCommon->m_bShowDepthPeeling) {
    // bind the colour blending FBO
    glBindFramebuffer(GL_FRAMEBUFFER, g_pCommon->mColorBlenderFBOID);
    // set the first colour attachment as the draw buffer
    glDrawBuffer(GL_COLOR_ATTACHMENT0);
    // clear the colour and depth buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // 1. In the first pass, we render normally with depth test enabled to get
    // the nearest surface
    glEnable(GL_DEPTH_TEST);
    DrawScene(MVP, g_pCommon->mCubeShader);

    // 2. Depth peeling + blending pass
    int numLayers = (Common::NUM_PASSES - 1) * 2;

    // for each pass
    for (int layer = 1; g_pCommon->m_bUseOQ || layer < numLayers; layer++) {
      int currId = layer % 2;
      int prevId = 1 - currId;

      // bind the current FBO
      glBindFramebuffer(GL_FRAMEBUFFER, g_pCommon->mFBO[currId]);
      // set the first colour attachment as draw buffer
      glDrawBuffer(GL_COLOR_ATTACHMENT0);

      // set clear colour to black
      glClearColor(0, 0, 0, 0);
      // clear the colour and depth buffers
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      // disbale blending and depth testing
      glDisable(GL_BLEND);
      glEnable(GL_DEPTH_TEST);

      // if we want to use occlusion query, we initiate it
      if (g_pCommon->m_bUseOQ) {
        glBeginQuery(GL_SAMPLES_PASSED_ARB, g_pCommon->mQueryId);
      }
      GL_CHECK_ERRORS;

      // bind the depth texture from the previous step
      glBindTexture(GL_TEXTURE_RECTANGLE, g_pCommon->mDepthTexID[prevId]);

      // render scene with the front to back peeling shader
      DrawScene(MVP, g_pCommon->mFrontPeelShader);

      // if we initiated the occlusion query, we end it
      if (g_pCommon->m_bUseOQ) {
        glEndQuery(GL_SAMPLES_PASSED_ARB);
      }
      GL_CHECK_ERRORS;

      // bind the colour blender FBO
      glBindFramebuffer(GL_FRAMEBUFFER, g_pCommon->mColorBlenderFBOID);
      // render to its first colour attachment
      glDrawBuffer(GL_COLOR_ATTACHMENT0);

      // enable blending but disable depth testing
      glDisable(GL_DEPTH_TEST);
      glEnable(GL_BLEND);

      // change the blending equation to add
      glBlendEquation(GL_FUNC_ADD);
      // use separate blending function
      glBlendFuncSeparate(GL_DST_ALPHA, GL_ONE, GL_ZERO,
                          GL_ONE_MINUS_SRC_ALPHA);

      // bind the result from the previous iteration as texture
      glBindTexture(GL_TEXTURE_RECTANGLE, g_pCommon->mTexID[currId]);
      // bind the blend shader and then draw a fullscreen quad
      g_pCommon->mBlendShader.Use();
      DrawFullScreenQuad();
      g_pCommon->mBlendShader.UnUse();

      // disable blending
      glDisable(GL_BLEND);
      GL_CHECK_ERRORS;

      // if we initiated the occlusion query, we get the query result
      // that is the total number of samples
      if (g_pCommon->m_bUseOQ) {
        GLuint sample_count;
        glGetQueryObjectuiv(g_pCommon->mQueryId, GL_QUERY_RESULT,
                            &sample_count);
        if (sample_count == 0) {
          break;
        }
      }
    }
    GL_CHECK_ERRORS;

    // 3. Final render pass
    // remove the FBO
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    // restore the default back buffer
    glDrawBuffer(GL_BACK_LEFT);
    // disable depth testing and blending
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);

    // bind the colour blender texture
    glBindTexture(GL_TEXTURE_RECTANGLE, g_pCommon->mColorBlenderTexID);
    // bind the final shader
    g_pCommon->mFinalShader.Use();
    // set shader uniforms
    glUniform4fv(g_pCommon->mFinalShader("vBackgroundColor"), 1,
                 &g_pCommon->bg.x);
    // draw full screen quad
    DrawFullScreenQuad();
    g_pCommon->mFinalShader.UnUse();
  } else {
    // no depth peeling, render scene with default alpha blending
    glEnable(GL_DEPTH_TEST);
    DrawScene(MVP, g_pCommon->mCubeShader);
  }

  // render grid
  g_pCommon->m_pGrid->Render(glm::value_ptr(MVP));

  // swap front and back buffers to show the rendered result
  glutSwapBuffers();
}

// Mouse down event handler
void OnMouseDown(int button, int s, int x, int y) {
  if (s == GLUT_DOWN) {
    g_pCommon->mOldX = x;
    g_pCommon->mOldY = y;
  }

  if (button == GLUT_MIDDLE_BUTTON) {
    g_pCommon->mState = 0;
  } else {
    g_pCommon->mState = 1;
  }
}

// Resize event handler
void OnResize(int w, int h) {
  // set the viewport
  glViewport(0, 0, static_cast<GLsizei>(w), static_cast<GLsizei>(h));

  // setup the projection matrix
  g_pCommon->mP =
      glm::perspective(45.0f, static_cast<float>(w) / h, 0.1f, 1000.0f);
}

// Keyboard event handler to toggle the depth peeling usage
void OnKey(unsigned char key, int /*x*/, int /*y*/) {
  switch (key) {
  case ' ':
    g_pCommon->m_bShowDepthPeeling = !g_pCommon->m_bShowDepthPeeling;
    break;
  }

  if (g_pCommon->m_bShowDepthPeeling) {
    glutSetWindowTitle("Front-to-back Depth Peeling: On");
  } else {
    glutSetWindowTitle("Front-to-back Depth Peeling: Off");
  }

  glutPostRedisplay();
}

// Mouse move event handler
void OnMouseMove(int x, int y) {
  if (g_pCommon->mState == 0) {
    g_pCommon->mDist += (y - g_pCommon->mOldY) / 5.0f;
  } else {
    g_pCommon->rX += (y - g_pCommon->mOldY) / 5.0f;
    g_pCommon->rY += (x - g_pCommon->mOldX) / 5.0f;
  }
  g_pCommon->mOldX = x;
  g_pCommon->mOldY = y;

  glutPostRedisplay();
}

void OnIdle() {
  // create a new rotation matrix for rotation on the Y axis
  g_pCommon->mR = glm::rotate(
      glm::mat4(1), glm::radians(g_pCommon->mAngle += 5), glm::vec3(0, 1, 0));
  // recall the display callback
  glutPostRedisplay();
}

int main(int argc, char *argv[]) {
  Common common;
  g_pCommon = &common;
  // Freeglut initialization
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
  glutInitContextVersion(3, 3);
  glutInitContextFlags(GLUT_CORE_PROFILE | GLUT_DEBUG);
  glutInitWindowSize(Common::WIDTH, Common::HEIGHT);
  glutCreateWindow("Front-to-back Depth Peeling - OpenGL 3.3");

  // Initialize glew
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
  // This is to ignore INVALID ENUM error 1282
  err = glGetError();
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
  glutKeyboardFunc(OnKey);
  glutIdleFunc(OnIdle);

  // main loop call
  glutMainLoop();
  return EXIT_SUCCESS;
}
