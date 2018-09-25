#include <GL/glew.h>
#include <GL/freeglut.h>
#include <iostream>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Grid.hpp"
#include "UnitCube.hpp"
#include "GLSLShader.hpp"

#define GL_CHECK_ERRORS assert(glGetError() == GL_NO_ERROR);

// vertex struct for storing per-vertex position and normal
struct Vertex {
  glm::vec3 pos, normal;
};

struct Common {
  // screen resolution
  static constexpr int WIDTH  = 1024;
  static constexpr int HEIGHT = 768;

  // cubemap size
  static constexpr int CUBEMAP_SIZE = 1024;

  // shaders for rendering and cubemap generation
  GLSLShader mShader, mCubemapShader;

  // sphere vertex array and vertex buffer objects
  GLuint mSphereVAOID;
  GLuint mSphereVerticesVBO;
  GLuint mSphereIndicesVBO;

  // distance to radially displace the spheres
  float m_fRadius = 2;

  // projection, cubemap, modelview and rotation matrices
  glm::mat4 mP        = glm::mat4(1);
  glm::mat4 mPcubemap = glm::mat4(1);
  glm::mat4 mMV       = glm::mat4(1);
  glm::mat4 mRot;

  // camera transformation variables
  int mState = 0, mOldX = 0, mOldY = 0;
  float mRX = 25, mRY = -40, mDist = -10;

  // dynamic cubemap texture ID
  GLuint mDynamicCubeMapID;

  // FBO and RBO IDs
  GLuint mFboID, mRboID;

  // grid object
  CGrid *m_pGrid = nullptr;

  // unit cube object
  CUnitCube *m_pCube = nullptr;

  // eye position
  glm::vec3 mEyePos;

  // vertice and indices of geometry
  std::vector<Vertex>   m_vVertices;
  std::vector<GLushort> m_vIndices;

  // autorotate angle
  float m_fAngle = 0;

  float dx = -0.1f; // direction

  // constant colours array
  const glm::vec3 colors[8] = {
    glm::vec3(1, 0, 0), glm::vec3(0, 1, 0),
    glm::vec3(0, 0, 1), glm::vec3(1, 1, 0),
    glm::vec3(1, 0, 1), glm::vec3(0, 1, 1),
    glm::vec3(1, 1, 1), glm::vec3(0.5, 0.5, 0.5)};
};
static Common *g_pCommon = nullptr;

// adds the given sphere indices to the indices vector
inline void push_indices(int sectors, int r, int s) {
  int curRow  = r * sectors;
  int nextRow = (r + 1) * sectors;

  g_pCommon->m_vIndices.push_back(static_cast<ushort>(curRow + s));
  g_pCommon->m_vIndices.push_back(static_cast<ushort>(nextRow + s));
  g_pCommon->m_vIndices.push_back(static_cast<ushort>(nextRow + (s + 1)));

  g_pCommon->m_vIndices.push_back(static_cast<ushort>(curRow + s));
  g_pCommon->m_vIndices.push_back(static_cast<ushort>(nextRow + (s + 1)));
  g_pCommon->m_vIndices.push_back(static_cast<ushort>(curRow + (s + 1)));
}

// generates a sphere primitive with the given radius, slices and stacks
void createSphere(float radius, unsigned int slices, unsigned int stacks) {
  float const R = 1.0f / static_cast<float>(slices - 1);
  float const S = 1.0f / static_cast<float>(stacks - 1);

  for (size_t r = 0; r < slices; ++r) {
    for (size_t s = 0; s < stacks; ++s) {
      float const y = static_cast<float>(sin(-M_PI_2 + M_PI * r * static_cast<double>(R)));
      float const x = static_cast<float>(cos(2 * M_PI * s * static_cast<double>(S)) * sin(M_PI * r * static_cast<double>(R)));
      float const z = static_cast<float>(sin(2 * M_PI * s * static_cast<double>(S)) * sin(M_PI * r * static_cast<double>(R)));

      Vertex v;
      v.pos = glm::vec3(x, y, z) * radius;
      v.normal = glm::normalize(v.pos);
      g_pCommon->m_vVertices.push_back(v);
      push_indices(static_cast<int>(stacks),
                   static_cast<int>(r),
                   static_cast<int>(s));
    }
  }
}

// mouse click handler
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

// mouse move handler
void OnMouseMove(int x, int y) {
  if (g_pCommon->mState == 0) {
    g_pCommon->mDist *= (1 + (y - g_pCommon->mOldY) / 60.0f);
  } else {
    g_pCommon->mRY += (x - g_pCommon->mOldX) / 5.0f;
    g_pCommon->mRX += (y - g_pCommon->mOldY) / 5.0f;
  }
  g_pCommon->mOldX = x;
  g_pCommon->mOldY = y;

  glutPostRedisplay();
}

// OpenGL initialization
void OnInit() {
  // load the cubemap shader
  g_pCommon->mCubemapShader.LoadFromFile(GL_VERTEX_SHADER, "shaders/cubemap.vert");
  g_pCommon->mCubemapShader.LoadFromFile(GL_FRAGMENT_SHADER, "shaders/cubemap.frag");
  // compile and link shader
  g_pCommon->mCubemapShader.CreateAndLinkProgram();
  g_pCommon->mCubemapShader.Use();
  // add shader attribute and uniforms
  g_pCommon->mCubemapShader.AddAttribute("vVertex");
  g_pCommon->mCubemapShader.AddAttribute("vNormal");
  g_pCommon->mCubemapShader.AddUniform("MVP");
  g_pCommon->mCubemapShader.AddUniform("eyePosition");
  g_pCommon->mCubemapShader.AddUniform("cubeMap");
  // set values of constant uniforms at initialization
  glUniform1i(g_pCommon->mCubemapShader("cubeMap"), 1);
  g_pCommon->mCubemapShader.UnUse();

  GL_CHECK_ERRORS

  // setup sphere geometry
  createSphere(1, 10, 10);

  GL_CHECK_ERRORS

  // setup sphere vao and vbo stuff
  glGenVertexArrays(1, &g_pCommon->mSphereVAOID);
  glGenBuffers(1,      &g_pCommon->mSphereVerticesVBO);
  glGenBuffers(1,      &g_pCommon->mSphereIndicesVBO);
  glBindVertexArray(g_pCommon->mSphereVAOID);

  glBindBuffer(GL_ARRAY_BUFFER, g_pCommon->mSphereVerticesVBO);
  // pass vertices to the buffer object
  glBufferData(GL_ARRAY_BUFFER, g_pCommon->m_vVertices.size() * sizeof(Vertex),
              &g_pCommon->m_vVertices[0],
               GL_STATIC_DRAW);
  GL_CHECK_ERRORS
  // enable vertex attribute array for position
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), nullptr);
  GL_CHECK_ERRORS
  // enable vertex attribute array for normal
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                        reinterpret_cast<const GLvoid *>(offsetof(Vertex, normal)));
  GL_CHECK_ERRORS
  // pass sphere indices to element array buffer
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_pCommon->mSphereIndicesVBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, g_pCommon->m_vIndices.size() * sizeof(GLushort),
               &g_pCommon->m_vIndices[0], GL_STATIC_DRAW);

  // generate the dynamic cubemap texture and bind to texture unit 1
  glGenTextures(1, &g_pCommon->mDynamicCubeMapID);
  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_CUBE_MAP, g_pCommon->mDynamicCubeMapID);
  // set texture parameters
  glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
  // for all 6 cubemap faces
  for (int face = 0; face < 6; face++) {
    // allocate a different texture for each face and assign to the cubemap
    // texture target
    glTexImage2D(static_cast<GLenum>(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face),
                 0, GL_RGBA, Common::CUBEMAP_SIZE, Common::CUBEMAP_SIZE, 0,
                 GL_RGBA, GL_FLOAT, nullptr);
  }

  GL_CHECK_ERRORS

  // setup FBO
  glGenFramebuffers(1, &g_pCommon->mFboID);
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, g_pCommon->mFboID);

  // setup render buffer object (RBO)
  glGenRenderbuffers(1, &g_pCommon->mRboID);
  glBindRenderbuffer(GL_RENDERBUFFER, g_pCommon->mRboID);

  // set the renderbuffer storage to have the same dimensions as the cubemap
  // texture also set the renderbuffer as the depth attachment of the FBO
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, Common::CUBEMAP_SIZE,
                        Common::CUBEMAP_SIZE);
  glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                            GL_RENDERBUFFER, g_pCommon->mFboID);

  // set the dynamic cubemap texture as the colour attachment of FBO
  glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                         GL_TEXTURE_CUBE_MAP_POSITIVE_X, g_pCommon->mDynamicCubeMapID, 0);

  // check the framebuffer completeness status
  GLenum status = glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER);
  if (status != GL_FRAMEBUFFER_COMPLETE) {
    std::cerr << "Frame buffer object setup error." << std::endl;
    exit(EXIT_FAILURE);
  } else {
    std::cerr << "FBO setup successfully." << std::endl;
  }
  // unbind FBO
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
  // unbind renderbuffer
  glBindRenderbuffer(GL_RENDERBUFFER, 0);

  GL_CHECK_ERRORS

  // create a grid object
  g_pCommon->m_pGrid = new CGrid();

  // create a unit cube object
  g_pCommon->m_pCube = new CUnitCube(glm::vec3(1, 0, 0));

  GL_CHECK_ERRORS

  // enable depth testing and back face culling
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);

  std::cout << "Initialization successfull" << std::endl;
}

// release all allocated resources
void OnShutdown() {
  // Destroy shader
  g_pCommon->mShader.DeleteShaderProgram();
  g_pCommon->mCubemapShader.DeleteShaderProgram();

  // Destroy vao and vbo
  glDeleteBuffers(1,      &g_pCommon->mSphereVerticesVBO);
  glDeleteBuffers(1,      &g_pCommon->mSphereIndicesVBO);
  glDeleteVertexArrays(1, &g_pCommon->mSphereVAOID);

  delete g_pCommon->m_pGrid;
  delete g_pCommon->m_pCube;

  glDeleteTextures(1, &g_pCommon->mDynamicCubeMapID);

  glDeleteFramebuffers(1,  &g_pCommon->mFboID);
  glDeleteRenderbuffers(1, &g_pCommon->mRboID);
  std::cout << "Shutdown successfull" << std::endl;
}

// resize event handler
void OnResize(int w, int h) {
  // set the viewport size
  glViewport(0, 0, static_cast<GLsizei>(w), static_cast<GLsizei>(h));
  // setup the projection matrix
  g_pCommon->mP = glm::perspective(45.0f, static_cast<GLfloat>(w)/ h, 0.1f, 1000.f);
  // setup the cube map projection matrix
  g_pCommon->mPcubemap = glm::perspective(90.0f, 1.0f, 0.1f, 1000.0f);
}

// idle event callback
void OnIdle() {
  // generate a new Y rotation matrix
  g_pCommon->mRot = glm::rotate(glm::mat4(1), g_pCommon->m_fAngle++, glm::vec3(0, 1, 0));

  // call display function
  glutPostRedisplay();
}

// scene rendering function
void DrawScene(glm::mat4 MView, glm::mat4 Proj) {
  // for each cube
  for (int i = 0; i < 8; i++) {
    // determine the cube's transform
    float angle = static_cast<float>(i / 8.0 * 2.0 * M_PI);
    glm::mat4 T = glm::translate(
        glm::mat4(1), glm::vec3(g_pCommon->m_fRadius * cosf(angle),
          0.5, g_pCommon->m_fRadius * sinf(angle)));

    // get the combined modelview projection matrix
    glm::mat4 MVP = Proj * MView * g_pCommon->mRot * T;

    // set the cube's colour
    g_pCommon->m_pCube->color = g_pCommon->colors[i];

    // render the cube
    g_pCommon->m_pCube->Render(glm::value_ptr(MVP));
  }

  // render the grid object
  g_pCommon->m_pGrid->Render(glm::value_ptr(Proj * MView));
}

// display callback function
void OnRender() {
  // increment the radius
  g_pCommon->m_fRadius += g_pCommon->dx;

  // if radius is beyond limits, invert the movement direction
  if (g_pCommon->m_fRadius < 1 || g_pCommon->m_fRadius > 5) {
    g_pCommon->dx = -g_pCommon->dx;
  }

  GL_CHECK_ERRORS

  // clear colour buffer
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // set the camera transform
  glm::mat4 T  = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, g_pCommon->mDist));
  const glm::mat4 Rx = glm::rotate(T,  g_pCommon->mRX, glm::vec3(1.0f, 0.0f, 0.0f));
  const glm::mat4 MV = glm::rotate(Rx, g_pCommon->mRY, glm::vec3(0.0f, 1.0f, 0.0f));

  // get the eye position
  g_pCommon->mEyePos.x = -(MV[0][0] * MV[3][0] + MV[0][1] * MV[3][1] + MV[0][2] * MV[3][2]);
  g_pCommon->mEyePos.y = -(MV[1][0] * MV[3][0] + MV[1][1] * MV[3][1] + MV[1][2] * MV[3][2]);
  g_pCommon->mEyePos.z = -(MV[2][0] * MV[3][0] + MV[2][1] * MV[3][1] + MV[2][2] * MV[3][2]);

  // p is to translate the sphere to bring it to the ground level
  const glm::vec3 p = glm::vec3(0, 1, 0);

  // when rendering to the CUBEMAP texture, we move all of cubes by opposite
  // amount so that the projection is clearly visible
  T = glm::translate(glm::mat4(1), -p);

  // set the viewport to the size of the cube map texture
  glViewport(0, 0, Common::CUBEMAP_SIZE, Common::CUBEMAP_SIZE);

  // bind the FBO
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, g_pCommon->mFboID);

  // set the GL_TEXTURE_CUBE_MAP_POSITIVE_X to the colour attachment of FBO
  glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                         GL_TEXTURE_CUBE_MAP_POSITIVE_X, g_pCommon->mDynamicCubeMapID, 0);
  // clear the colour and depth buffers
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  // set the virtual viewrer at the reflective object center and render the
  // scene using the cube map projection matrix and appropriate viewing settings
  glm::mat4 MV1 =
      glm::lookAt(glm::vec3(0), glm::vec3(1, 0, 0), glm::vec3(0, -1, 0));
  DrawScene(MV1 * T, g_pCommon->mPcubemap);

  // set the GL_TEXTURE_CUBE_MAP_NEGATIVE_X to the colour attachment of FBO
  glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                         GL_TEXTURE_CUBE_MAP_NEGATIVE_X, g_pCommon->mDynamicCubeMapID, 0);
  // clear the colour and depth buffers
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  // set the virtual viewrer at the reflective object center and render the
  // scene using the cube map projection matrix and appropriate viewing settings
  glm::mat4 MV2 =
      glm::lookAt(glm::vec3(0), glm::vec3(-1, 0, 0), glm::vec3(0, -1, 0));
  DrawScene(MV2 * T, g_pCommon->mPcubemap);

  // set the GL_TEXTURE_CUBE_MAP_POSITIVE_Y to the colour attachment of FBO
  glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                         GL_TEXTURE_CUBE_MAP_POSITIVE_Y, g_pCommon->mDynamicCubeMapID, 0);
  // clear the colour and depth buffers
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  // set the virtual viewrer at the reflective object center and render the
  // scene using the cube map projection matrix and appropriate viewing settings
  glm::mat4 MV3 =
      glm::lookAt(glm::vec3(0), glm::vec3(0, 1, 0), glm::vec3(1, 0, 0));
  DrawScene(MV3 * T, g_pCommon->mPcubemap);

  // set the GL_TEXTURE_CUBE_MAP_NEGATIVE_Y to the colour attachment of FBO
  glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                         GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, g_pCommon->mDynamicCubeMapID, 0);
  // clear the colour and depth buffers
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  // set the virtual viewrer at the reflective object center and render the
  // scene using the cube map projection matrix and appropriate viewing settings
  glm::mat4 MV4 =
      glm::lookAt(glm::vec3(0), glm::vec3(0, -1, 0), glm::vec3(1, 0, 0));
  DrawScene(MV4 * T, g_pCommon->mPcubemap);

  // set the GL_TEXTURE_CUBE_MAP_POSITIVE_Z to the colour attachment of FBO
  glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                         GL_TEXTURE_CUBE_MAP_POSITIVE_Z, g_pCommon->mDynamicCubeMapID, 0);
  // clear the colour and depth buffers
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  // set the virtual viewrer at the reflective object center and render the
  // scene using the cube map projection matrix and appropriate viewing settings
  glm::mat4 MV5 =
      glm::lookAt(glm::vec3(0), glm::vec3(0, 0, 1), glm::vec3(0, -1, 0));
  DrawScene(MV5 * T, g_pCommon->mPcubemap);

  // set the GL_TEXTURE_CUBE_MAP_NEGATIVE_Z to the colour attachment of FBO
  glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                         GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, g_pCommon->mDynamicCubeMapID, 0);
  // clear the colour and depth buffers
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  // set the virtual viewrer at the reflective object center and render the
  // scene using the cube map projection matrix and appropriate viewing settings
  glm::mat4 MV6 =
      glm::lookAt(glm::vec3(0), glm::vec3(0, 0, -1), glm::vec3(0, -1, 0));
  DrawScene(MV6 * T, g_pCommon->mPcubemap);

  // unbind the FBO
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

  // reset the default viewport
  glViewport(0, 0, Common::WIDTH, Common::HEIGHT);

  // render scene from the camera point of view and projection matrix
  DrawScene(MV, g_pCommon->mP);

  // bind the sphere vertex array object
  glBindVertexArray(g_pCommon->mSphereVAOID);

  // use the cubemap shader to render the reflective sphere
  g_pCommon->mCubemapShader.Use();
  // set the sphere transform
  T = glm::translate(glm::mat4(1), p);
  // set the shader uniforms
  glUniformMatrix4fv(g_pCommon->mCubemapShader("MVP"), 1, GL_FALSE,
                     glm::value_ptr(g_pCommon->mP * (MV * T)));
  glUniform3fv(g_pCommon->mCubemapShader("eyePosition"), 1, glm::value_ptr(g_pCommon->mEyePos));
  // draw the sphere triangles
  glDrawElements(GL_TRIANGLES, static_cast<int>(g_pCommon->m_vIndices.size()),
                 GL_UNSIGNED_SHORT, nullptr);

  // unbind shader
  g_pCommon->mCubemapShader.UnUse();

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
  glutCreateWindow("Dynamic Cubemapping - OpenGL 3.3");

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

  // initialization of OpenGL
  OnInit();

  // callback hooks
  glutCloseFunc(OnShutdown);
  glutDisplayFunc(OnRender);
  glutReshapeFunc(OnResize);
  glutMouseFunc(OnMouseDown);
  glutMotionFunc(OnMouseMove);
  glutIdleFunc(OnIdle);

  // main loop call
  glutMainLoop();

  return 0;
}
