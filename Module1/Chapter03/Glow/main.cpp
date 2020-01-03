// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include <iostream>
#include <sstream>

#include <GL/glew.h>
#include <GL/freeglut.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "FreeCamera.hpp"
#include "GLSLShader.hpp"
#include "Grid.hpp"
#include "UnitCube.hpp"

#define GL_CHECK_ERRORS assert(glGetError() == GL_NO_ERROR);

struct Common {
  // screen size
  static constexpr int WIDTH = 1280;
  static constexpr int HEIGHT = 960;

  // camera tranformation variables
  int mState = 0, mOldX = 0, mOldY = 0;
  float mRX = 0, mRY = 0, mFov = 45;

  // espsilon epsilon2 for accuracy
  static constexpr float EPSILON = 0.001f;
  static constexpr float EPSILON2 = EPSILON * EPSILON;

  // delta time
  float mDt = 0;

  // Free camera instance
  CFreeCamera mCam;

  // output message
  std::stringstream mMessage;

  // Grid object
  CGrid *m_pGrid;

  // Unit Cube object
  CUnitCube *m_pCube;

  // modelview, projection and rotation matrices
  glm::mat4 mMV, mP;
  glm::mat4 mRot;

  // timing related variables
  float mLast_time = 0, mCurrent_time = 0;

  // mouse filtering support variables
  static constexpr float MOUSE_FILTER_WEIGHT       = 0.75f;
  static constexpr int   MOUSE_HISTORY_BUFFER_SIZE = 10;

  // mouse history buffer
  glm::vec2 mMouseHistory[MOUSE_HISTORY_BUFFER_SIZE];

  float mMouseX = 0, mMouseY = 0; // filtered mouse values

  // flag to enable filtering
  bool mUseFiltering = true;

  // distance of the points
  float mRadius = 2.f;

  // particle positions
  glm::vec3 mParticles[8];

  // particle vertex array and vertex buffer objects IDs
  GLuint mParticlesVAO;
  GLuint mParticlesVBO;

  // particle and blurring shader
  GLSLShader mParticleShader;
  GLSLShader mBlurShader;

  // quad vertex array and vertex buffer object IDs
  GLuint mQuadVAOID;
  GLuint mQuadVBOID;
  GLuint mQuadVBOIndicesID;

  // autorotate g_pCommon->mAngle
  float mAngle = 0.f;

  // FBO ID
  GLuint mFboID;
  // FBO colour attachment textures
  GLuint mTexID[2]; // 0 -> glow rendered output
                    // 1 -> blurred output

  // width and height of the FBO colour attachment
  static constexpr int RENDER_TARGET_WIDTH  = Common::WIDTH >> 1;
  static constexpr int RENDER_TARGET_HEIGHT = Common::HEIGHT >> 1;
};
static Common *g_pCommon = nullptr;

// mouse move filtering function
void filterMouseMoves(float dx, float dy) {
  for (int i = Common::MOUSE_HISTORY_BUFFER_SIZE - 1; i > 0; --i) {
    g_pCommon->mMouseHistory[i] = g_pCommon->mMouseHistory[i - 1];
  }

  // Store current mouse entry at front of array.
  g_pCommon->mMouseHistory[0] = glm::vec2(dx, dy);

  float averageX = 0.0f;
  float averageY = 0.0f;
  float averageTotal = 0.0f;
  float currentWeight = 1.0f;

  // Filter the mouse.
  for (int i = 0; i < Common::MOUSE_HISTORY_BUFFER_SIZE; ++i) {
    glm::vec2 tmp = g_pCommon->mMouseHistory[i];
    averageX += tmp.x * currentWeight;
    averageY += tmp.y * currentWeight;
    averageTotal += 1.0f * currentWeight;
    currentWeight *= Common::MOUSE_FILTER_WEIGHT;
  }

  g_pCommon->mMouseX = averageX / averageTotal;
  g_pCommon->mMouseY = averageY / averageTotal;
}

// mouse click handler
void OnMouseDown(int button, int s, int x, int y) {
  if (s == GLUT_DOWN) {
    g_pCommon->mOldX = x;
    g_pCommon->mOldY = y;
  }

  if (button == GLUT_MIDDLE_BUTTON)
    g_pCommon->mState = 0;
  else
    g_pCommon->mState = 1;
}

// mouse move handler
void OnMouseMove(int x, int y) {
  if (g_pCommon->mState == 0) {
    g_pCommon->mFov += (y - g_pCommon->mOldY) / 5.0f;
    g_pCommon->mCam.SetupProjection(g_pCommon->mFov, g_pCommon->mCam.GetAspectRatio());
  } else {
    g_pCommon->mRY += (y - g_pCommon->mOldY) / 5.0f;
    g_pCommon->mRX += (g_pCommon->mOldX - x) / 5.0f;
    if (g_pCommon->mUseFiltering)
      filterMouseMoves(g_pCommon->mRX, g_pCommon->mRY);
    else {
      g_pCommon->mMouseX = g_pCommon->mRX;
      g_pCommon->mMouseY = g_pCommon->mRY;
    }
    g_pCommon->mCam.Rotate(g_pCommon->mMouseX, g_pCommon->mMouseY, 0);
  }
  g_pCommon->mOldX = x;
  g_pCommon->mOldY = y;
  glutPostRedisplay();
}

// initialize OpenGL
void OnInit() {

  // create g_pCommon->m_pGrid object
  g_pCommon->m_pGrid = new CGrid(20, 20);

  // create unit cg_pCommon->m_pCube
  g_pCommon->m_pCube = new CUnitCube();
  // set the cg_pCommon->m_pCube colour as blue
  g_pCommon->m_pCube->color = glm::vec3(0, 0, 1);

  GL_CHECK_ERRORS

  // set the camera position
  glm::vec3 p = glm::vec3(5, 5, 5);
  g_pCommon->mCam.SetPosition(p);

  // orient the camera
  glm::vec3 look = glm::normalize(p);
  float yaw = glm::degrees(atan2f(look.z, look.x) + static_cast<float>(M_PI));
  float pitch = glm::degrees(asinf(look.y));
  g_pCommon->mRX = yaw;
  g_pCommon->mRY = pitch;
  if (g_pCommon->mUseFiltering) {
    for (int i = 0; i < Common::MOUSE_HISTORY_BUFFER_SIZE; ++i) {
      g_pCommon->mMouseHistory[i] = glm::vec2(g_pCommon->mRX, g_pCommon->mRY);
    }
  }
  g_pCommon->mCam.Rotate(g_pCommon->mRX, g_pCommon->mRY, 0);

  // enable depth testing
  glEnable(GL_DEPTH_TEST);

  // set point size
  glPointSize(50);

  // load particle shader
  g_pCommon->mParticleShader.LoadFromFile(GL_VERTEX_SHADER, "shaders/particle.vert");
  g_pCommon->mParticleShader.LoadFromFile(GL_FRAGMENT_SHADER, "shaders/particle.frag");
  // compile and link shader
  g_pCommon->mParticleShader.CreateAndLinkProgram();
  g_pCommon->mParticleShader.Use();
  // set shader attributes
  g_pCommon->mParticleShader.AddAttribute("vVertex");
  g_pCommon->mParticleShader.AddUniform("MVP");
  g_pCommon->mParticleShader.UnUse();

  // set particle positions
  for (int i = 0; i < 8; i++) {
    float theta = static_cast<float>(i / 8.0 * 2 * M_PI);
    g_pCommon->mParticles[i].x = g_pCommon->mRadius * cosf(theta);
    g_pCommon->mParticles[i].y = 0.0f;
    g_pCommon->mParticles[i].z = g_pCommon->mRadius * sinf(theta);
  }

  // set particle vertex array and vertex buffer objects
  glGenVertexArrays(1, &g_pCommon->mParticlesVAO);
  glGenBuffers(1, &g_pCommon->mParticlesVBO);
  glBindVertexArray(g_pCommon->mParticlesVAO);

  glBindBuffer(GL_ARRAY_BUFFER, g_pCommon->mParticlesVBO);
  // pass particle vertices to buffer object
  glBufferData(GL_ARRAY_BUFFER, sizeof(g_pCommon->mParticles), &g_pCommon->mParticles[0],
               GL_STATIC_DRAW);
  GL_CHECK_ERRORS
  // enable vertex attribute array for position
  glEnableVertexAttribArray(g_pCommon->mParticleShader["vVertex"]);
  glVertexAttribPointer(g_pCommon->mParticleShader["vVertex"], 3, GL_FLOAT, GL_FALSE, 0, nullptr);

  GL_CHECK_ERRORS

  // setup FBO and offscreen render target
  glGenFramebuffers(1, &g_pCommon->mFboID);
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, g_pCommon->mFboID);

  // setup two colour attachments
  glGenTextures(2, g_pCommon->mTexID);
  glActiveTexture(GL_TEXTURE0);
  for (int i = 0; i < 2; i++) {
    glBindTexture(GL_TEXTURE_2D, g_pCommon->mTexID[i]);
    // set texture parameters
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // allocate OpenGL texture
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, Common::RENDER_TARGET_WIDTH,
                 Common::RENDER_TARGET_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    GL_CHECK_ERRORS

    // set the current texture as the FBO attachment
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i,
                           GL_TEXTURE_2D, g_pCommon->mTexID[i], 0);
  }

  GL_CHECK_ERRORS

  // check for framebuffer completeness
  GLenum status = glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER);
  if (status != GL_FRAMEBUFFER_COMPLETE) {
    std::cerr << "Frame buffer object setup error." << std::endl;
    exit(EXIT_FAILURE);
  } else {
    std::cerr << "FBO setup successfully." << std::endl;
  }

  // unbind the FBO
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

  GL_CHECK_ERRORS

  // setup fullscreen quad vertices
  glm::vec2 vertices[4];
  vertices[0] = glm::vec2(0, 0);
  vertices[1] = glm::vec2(1, 0);
  vertices[2] = glm::vec2(1, 1);
  vertices[3] = glm::vec2(0, 1);

  GLushort indices[6];

  // fill quad indices array
  GLushort *id = &indices[0];
  *id++ = 0;
  *id++ = 1;
  *id++ = 2;
  *id++ = 0;
  *id++ = 2;
  *id++ = 3;

  // load the blur shader
  g_pCommon->mBlurShader.LoadFromFile(GL_VERTEX_SHADER, "shaders/full_screen_shader.vert");
  g_pCommon->mBlurShader.LoadFromFile(GL_FRAGMENT_SHADER,
                          "shaders/full_screen_shader.frag");
  // compile and link the shader
  g_pCommon->mBlurShader.CreateAndLinkProgram();
  g_pCommon->mBlurShader.Use();
  // add shader attributes and uniforms
  g_pCommon->mBlurShader.AddAttribute("vVertex");
  g_pCommon->mBlurShader.AddUniform("textureMap");
  // set the values of the constant uniforms at initialization
  glUniform1i(g_pCommon->mBlurShader("textureMap"), 0);
  g_pCommon->mBlurShader.UnUse();

  GL_CHECK_ERRORS

  // set up quad vertex array and vertex buffer object
  glGenVertexArrays(1, &g_pCommon->mQuadVAOID);
  glGenBuffers(1, &g_pCommon->mQuadVBOID);
  glGenBuffers(1, &g_pCommon->mQuadVBOIndicesID);

  glBindVertexArray(g_pCommon->mQuadVAOID);

  glBindBuffer(GL_ARRAY_BUFFER, g_pCommon->mQuadVBOID);
  // pass quad vertices
  glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(glm::vec2), &vertices[0],
               GL_STATIC_DRAW);

  // enable vertex attribute array for vertex position
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

  // pass the quad indices to element array buffer
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_pCommon->mQuadVBOIndicesID);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLushort) * 6, &indices[0],
               GL_STATIC_DRAW);

  glBindVertexArray(0);

  GL_CHECK_ERRORS
  std::cout << "Initialization successfull" << std::endl;
}

// release all allocated resources
void OnShutdown() {
  g_pCommon->mParticleShader.DeleteShaderProgram();
  g_pCommon->mBlurShader.DeleteShaderProgram();

  delete g_pCommon->m_pGrid;
  delete g_pCommon->m_pCube;

  glDeleteBuffers(1, &g_pCommon->mParticlesVBO);
  glDeleteBuffers(1, &g_pCommon->mParticlesVAO);

  glDeleteBuffers(1, &g_pCommon->mQuadVBOID);
  glDeleteBuffers(1, &g_pCommon->mQuadVBOIndicesID);
  glDeleteBuffers(1, &g_pCommon->mQuadVAOID);

  glDeleteTextures(2, g_pCommon->mTexID);
  glDeleteFramebuffers(1, &g_pCommon->mFboID);

  std::cout << "Shutdown successfull" << std::endl;
}

// resize event handler
void OnResize(int w, int h) {
  // set the viewport size
  glViewport(0, 0, static_cast<GLsizei>(w), static_cast<GLsizei>(h));
  // set the camera projection settings
  g_pCommon->mCam.SetupProjection(g_pCommon->mFov, static_cast<float>(w) / h);
}

// idle callback function
void OnIdle() {
  // generate a rotation matrix to rotate on the Y axis each idle event
  g_pCommon->mRot = glm::rotate(glm::mat4(1), g_pCommon->mAngle++, glm::vec3(0, 1, 0));

  // call the display function
  glutPostRedisplay();
}

// keyboard event handler to change the output to convolved or normal image
void OnKey(unsigned char key, int /*x*/, int /*y*/) {
  switch (key) {
  case 'w':
    g_pCommon->mCam.Walk(g_pCommon->mDt);
    break;
  case 's':
    g_pCommon->mCam.Walk(-g_pCommon->mDt);
    break;
  case 'a':
    g_pCommon->mCam.Strafe(-g_pCommon->mDt);
    break;
  case 'd':
    g_pCommon->mCam.Strafe(g_pCommon->mDt);
    break;
  case 'q':
    g_pCommon->mCam.Lift(g_pCommon->mDt);
    break;
  case 'z':
    g_pCommon->mCam.Lift(-g_pCommon->mDt);
    break;
  }

  glm::vec3 t = g_pCommon->mCam.GetTranslation();

  if (glm::dot(t, t) > Common::EPSILON2) {
    g_pCommon->mCam.SetTranslation(t * 0.95f);
  }
  // call display function
  glutPostRedisplay();
}

// display callback function
void OnRender() {
  static int total = 0;
  static int offset = 0;
  total++;

  if (total % 50 == 0) {
    offset = 4 - offset;
  }
  GL_CHECK_ERRORS

  // timing related calcualtion
  g_pCommon->mLast_time = g_pCommon->mCurrent_time;
  g_pCommon->mCurrent_time = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
  g_pCommon->mDt = g_pCommon->mCurrent_time - g_pCommon->mLast_time;

  // clear the colour and depth buffers
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // setup the modelview and projection matrices and get the combined modelview
  // projection matrix
  g_pCommon->mMV = g_pCommon->mCam.GetViewMatrix();
  g_pCommon->mP = g_pCommon->mCam.GetProjectionMatrix();
  glm::mat4 MVP = g_pCommon->mP * g_pCommon->mMV;

  // Render scene normally
  // render the g_pCommon->m_pGrid
  g_pCommon->m_pGrid->Render(glm::value_ptr(MVP));

  // render the cg_pCommon->m_pCube
  g_pCommon->m_pCube->Render(glm::value_ptr(MVP));

  // set the particle vertex array object
  glBindVertexArray(g_pCommon->mParticlesVAO);
  // set the particle shader
  g_pCommon->mParticleShader.Use();
  // set the shader uniforms
  glUniformMatrix4fv(g_pCommon->mParticleShader("MVP"), 1, GL_FALSE,
                     glm::value_ptr(MVP * g_pCommon->mRot));
  // draw g_pCommon->mParticles
  glDrawArrays(GL_POINTS, 0, 8);

  GL_CHECK_ERRORS

  // Activate FBO and render the scene elements which need glow
  // in our example, we will apply glow to the first 4 points
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, g_pCommon->mFboID);

  // set the viewport to the size of the offscreen render target
  glViewport(0, 0, Common::RENDER_TARGET_WIDTH, Common::RENDER_TARGET_HEIGHT);

  // set colour attachment 0 as the draw buffer
  glDrawBuffer(GL_COLOR_ATTACHMENT0);
  // clear the colour buffer
  glClear(GL_COLOR_BUFFER_BIT);
  // render 4 points
  glDrawArrays(GL_POINTS, offset, 4);
  // unbind the particle shader
  g_pCommon->mParticleShader.UnUse();
  GL_CHECK_ERRORS

  // set the first colour attachment
  glDrawBuffer(GL_COLOR_ATTACHMENT1);
  // bind the output of the previous step as texture
  glBindTexture(GL_TEXTURE_2D, g_pCommon->mTexID[0]);
  // use the blur shader
  g_pCommon->mBlurShader.Use();
  // bind the fullscreen quad vertex array
  glBindVertexArray(g_pCommon->mQuadVAOID);
  // render fullscreen quad
  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr);

  GL_CHECK_ERRORS

  // unbind the FBO
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
  // restore the default back buffer
  glDrawBuffer(GL_BACK_LEFT);
  // bind the filtered texture from the final step
  glBindTexture(GL_TEXTURE_2D, g_pCommon->mTexID[1]);

  GL_CHECK_ERRORS

  // reset the default viewport
  glViewport(0, 0, Common::WIDTH, Common::HEIGHT);
  GL_CHECK_ERRORS
  // enable additive blending
  glEnable(GL_BLEND);
  glBlendFunc(GL_ONE, GL_ONE);
  // draw fullscreen quad
  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr);
  glBindVertexArray(0);

  // unbind the blur shader
  g_pCommon->mBlurShader.UnUse();

  // disable blending
  glDisable(GL_BLEND);

  GL_CHECK_ERRORS

  // swap front and back buffers to show the rendered result
  glutSwapBuffers();
}

int main(int argc, char **argv) {
  Common common;
  g_pCommon = &common;

  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
  glutInitContextVersion(3, 3);
  glutInitContextFlags(GLUT_CORE_PROFILE | GLUT_DEBUG);
  glutInitWindowSize(Common::WIDTH, Common::HEIGHT);
  glutCreateWindow("Glow - OpenGL 3.3");

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

  GL_CHECK_ERRORS

  // opengl initialization
  OnInit();

  // callback hooks
  glutCloseFunc(OnShutdown);
  glutDisplayFunc(OnRender);
  glutReshapeFunc(OnResize);
  glutMouseFunc(OnMouseDown);
  glutMotionFunc(OnMouseMove);
  glutKeyboardFunc(OnKey);
  glutIdleFunc(OnIdle);

  // call main loop
  glutMainLoop();

  return 0;
}
