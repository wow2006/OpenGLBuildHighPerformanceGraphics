// This is an open source non-commercial project. Dear PVS-Studio, please check
// it. PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include <iostream>
#include <cstring>
#include <sstream>

#include <GL/glew.h>
#include <GL/freeglut.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "GLSLShader.hpp"
#include "FreeCamera.hpp"
#include "TexturedPlane.hpp"

#ifndef MAX_PATH
#define MAX_PATH 260
#endif //!MAX_PATH

#define GL_CHECK_ERRORS assert(glGetError() == GL_NO_ERROR);

struct Common {
  //screen size
  static constexpr int WIDTH  = 1280;
  static constexpr int HEIGHT = 960;

  // shaders for this recipe
  GLSLShader shader;
  // point rendering shader
  GLSLShader pointShader;

  // vertex array and vertex buffer objects
  GLuint vaoID;
  GLuint vboVerticesID;
  GLuint vboIndicesID;

  // view frustum vertex array and vertex buffer object
  GLuint vaoFrustumID;
  GLuint vboFrustumVerticesID;
  GLuint vboFrustumIndicesID;

  static constexpr int NUM_X = 40; // total quads on X axis
  static constexpr int NUM_Z = 40; // total quads on Z axis

  static constexpr float SIZE_X = 100; // size of plane in world space
  static constexpr float SIZE_Z = 100;
  static constexpr float HALF_SIZE_X = SIZE_X / 2.0f;
  static constexpr float HALF_SIZE_Z = SIZE_Z / 2.0f;

  // total vertices and indices
  glm::vec3 vertices[(NUM_X + 1) * (NUM_Z + 1)];
  static constexpr int TOTAL_INDICES = NUM_X * NUM_Z * 2 * 3;
  GLushort indices[TOTAL_INDICES];

  // for floating point imprecision
  static constexpr float EPSILON = 0.001f;
  static constexpr float EPSILON2 = EPSILON * EPSILON;

  // camera tranformation variables
  int state = 0, oldX = 0, oldY = 0;
  float rX = -135, rY = 45, fov = 45;

  // delta time
  float dt = 0;

  // timing related variables
  float last_time = 0, current_time = 0;

  // 2 FreeCamera instances and a current pointer
  CFreeCamera cam;
  CFreeCamera world;
  CFreeCamera *pCurrentCam;

  // mouse filtering support variables
  static constexpr float MOUSE_FILTER_WEIGHT = 0.75f;
  static constexpr int   MOUSE_HISTORY_BUFFER_SIZE = 10;

  // mouse history buffer
  glm::vec2 mouseHistory[MOUSE_HISTORY_BUFFER_SIZE];

  float mouseX = 0, mouseY = 0; // filtered mouse values

  // flag to enable filtering
  bool useFiltering = true;

  // view frustum vertices
  glm::vec3 frustum_vertices[8];

  // constant colours
  GLfloat white[4] = {1, 1, 1, 1};
  GLfloat red[4] = {1, 0, 0, 0.5};
  GLfloat cyan[4] = {0, 1, 1, 0.5};

  // points
  static constexpr int PX = 100;
  static constexpr int PZ = 100;
  static constexpr int MAX_POINTS = PX * PZ;

  // point vertices, vertex array and vertex buffer objects
  glm::vec3 pointVertices[MAX_POINTS];
  GLuint pointVAOID, pointVBOID;

  // number of points visible
  int total_visible = 0;

  // hardware query
  GLuint query;

  // FPS related variables
  float start_time = 0;
  float fps = 0;
  int total_frames;
  float last_time_fps = 0;

  // string buffer for message display
  char buffer[MAX_PATH] = {'\0'};
};
static Common *g_pCommon = nullptr;


// mouse move filtering function
void filterMouseMoves(float dx, float dy) {
  for (int i = Common::MOUSE_HISTORY_BUFFER_SIZE - 1; i > 0; --i) {
    g_pCommon->mouseHistory[i] = g_pCommon->mouseHistory[i - 1];
  }

  // Store current mouse entry at front of array.
  g_pCommon->mouseHistory[0] = glm::vec2(dx, dy);

  float averageX      = 0.0f;
  float averageY      = 0.0f;
  float averageTotal  = 0.0f;
  float currentWeight = 1.0f;

  // Filter the mouse.
  for (int i = 0; i < Common::MOUSE_HISTORY_BUFFER_SIZE; ++i) {
    glm::vec2 tmp = g_pCommon->mouseHistory[i];
    averageX += tmp.x * currentWeight;
    averageY += tmp.y * currentWeight;
    averageTotal += 1.0f * currentWeight;
    currentWeight *= Common::MOUSE_FILTER_WEIGHT;
  }

  g_pCommon->mouseX = averageX / averageTotal;
  g_pCommon->mouseY = averageY / averageTotal;
}

// mouse click handler
void OnMouseDown(int button, int s, int x, int y) {
  if (s == GLUT_DOWN) {
    g_pCommon->oldX = x;
    g_pCommon->oldY = y;
  }

  if (button == GLUT_MIDDLE_BUTTON)
    g_pCommon->state = 0;
  else if (button == GLUT_RIGHT_BUTTON)
    g_pCommon->state = 2;
  else
    g_pCommon->state = 1;
}

// mouse move handler
void OnMouseMove(int x, int y) {
  bool changed = false;
  if (g_pCommon->state == 0) {
    g_pCommon->fov += (y - g_pCommon->oldY) / 5.0f;
    g_pCommon->pCurrentCam->SetFOV(g_pCommon->fov);
    changed = true;
  } else if (g_pCommon->state == 1) {
    g_pCommon->rY += (y - g_pCommon->oldY) / 5.0f;
    g_pCommon->rX += (g_pCommon->oldX - x) / 5.0f;
    if (g_pCommon->useFiltering)
      filterMouseMoves(g_pCommon->rX, g_pCommon->rY);
    else {
      g_pCommon->mouseX = g_pCommon->rX;
      g_pCommon->mouseY = g_pCommon->rY;
    }
    if (g_pCommon->pCurrentCam == &g_pCommon->world) {
      g_pCommon->cam.Rotate(g_pCommon->mouseX, g_pCommon->mouseY, 0);
      g_pCommon->cam.CalcFrustumPlanes();
    } else {
      g_pCommon->pCurrentCam->Rotate(g_pCommon->mouseX, g_pCommon->mouseY, 0);
    }
    changed = true;
  }
  g_pCommon->oldX = x;
  g_pCommon->oldY = y;

  if (changed) {
    g_pCommon->pCurrentCam->CalcFrustumPlanes();
    g_pCommon->frustum_vertices[0] = g_pCommon->cam.farPts[0];
    g_pCommon->frustum_vertices[1] = g_pCommon->cam.farPts[1];
    g_pCommon->frustum_vertices[2] = g_pCommon->cam.farPts[2];
    g_pCommon->frustum_vertices[3] = g_pCommon->cam.farPts[3];

    g_pCommon->frustum_vertices[4] = g_pCommon->cam.nearPts[0];
    g_pCommon->frustum_vertices[5] = g_pCommon->cam.nearPts[1];
    g_pCommon->frustum_vertices[6] = g_pCommon->cam.nearPts[2];
    g_pCommon->frustum_vertices[7] = g_pCommon->cam.nearPts[3];

    // update the frustum vertices on the GPU
    glBindVertexArray(g_pCommon->vaoFrustumID);
    glBindBuffer(GL_ARRAY_BUFFER, g_pCommon->vboFrustumVerticesID);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(g_pCommon->frustum_vertices),
                    &g_pCommon->frustum_vertices[0]);
    glBindVertexArray(0);
  }

  glutPostRedisplay();
}

void OnInit() {
  // generate hardware query
  glGenQueries(1, &g_pCommon->query);

  // enable polygin line drawing mode
  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  // enable depth testing
  glEnable(GL_DEPTH_TEST);

  // set point size
  glPointSize(10);

  GL_CHECK_ERRORS

  // load the shader
  g_pCommon->shader.LoadFromFile(GL_VERTEX_SHADER, "shaders/view.vert");
  g_pCommon->shader.LoadFromFile(GL_FRAGMENT_SHADER, "shaders/view.frag");
  // compile and link shader
  g_pCommon->shader.CreateAndLinkProgram();
  g_pCommon->shader.Use();
  // add attributes and uniforms
  g_pCommon->shader.AddAttribute("vVertex");
  g_pCommon->shader.AddUniform("MVP");
  g_pCommon->shader.AddUniform("color");
  // set values of constant uniforms at initialization
  glUniform4fv(g_pCommon->shader("color"), 1, g_pCommon->white);
  g_pCommon->shader.UnUse();

  GL_CHECK_ERRORS

  // setup point rendering shader
  g_pCommon->pointShader.LoadFromFile(GL_VERTEX_SHADER,   "shaders/points.vert");
  g_pCommon->pointShader.LoadFromFile(GL_GEOMETRY_SHADER, "shaders/points.geom");
  g_pCommon->pointShader.LoadFromFile(GL_FRAGMENT_SHADER, "shaders/points.frag");
  // compile and link shader
  g_pCommon->pointShader.CreateAndLinkProgram();
  g_pCommon->pointShader.Use();
  // add attributes and uniforms
  g_pCommon->pointShader.AddAttribute("vVertex");
  g_pCommon->pointShader.AddUniform("MVP");
  g_pCommon->pointShader.AddUniform("t");
  g_pCommon->pointShader.AddUniform("FrustumPlanes");
  g_pCommon->pointShader.UnUse();

  GL_CHECK_ERRORS

  {
    // setup ground plane geometry
    // setup vertices
    int count = 0;
    int i = 0, j = 0;
    for (j = 0; j <= Common::NUM_Z; j++) {
      for (i = 0; i <= Common::NUM_X; i++) {
        g_pCommon->vertices[count++] =
            glm::vec3(((float(i) / (Common::NUM_X - 1)) * 2 - 1) * Common::HALF_SIZE_X, 0,
                      ((float(j) / (Common::NUM_Z - 1)) * 2 - 1) * Common::HALF_SIZE_Z);
      }
    }

    // fill indices array
    GLushort *id = &g_pCommon->indices[0];
    for (i = 0; i < Common::NUM_Z; i++) {
      for (j = 0; j < Common::NUM_X; j++) {
        int i0 = i * (Common::NUM_X + 1) + j;
        int i1 = i0 + 1;
        int i2 = i0 + (Common::NUM_X + 1);
        int i3 = i2 + 1;
        if ((j + i) % 2) {
          *id++ = static_cast<GLushort>(i0);
          *id++ = static_cast<GLushort>(i2);
          *id++ = static_cast<GLushort>(i1);
          *id++ = static_cast<GLushort>(i1);
          *id++ = static_cast<GLushort>(i2);
          *id++ = static_cast<GLushort>(i3);
        } else {
          *id++ = static_cast<GLushort>(i0);
          *id++ = static_cast<GLushort>(i2);
          *id++ = static_cast<GLushort>(i3);
          *id++ = static_cast<GLushort>(i0);
          *id++ = static_cast<GLushort>(i3);
          *id++ = static_cast<GLushort>(i1);
        }
      }
    }
  }

  GL_CHECK_ERRORS

  // setup ground plane vao and vbo stuff
  glGenVertexArrays(1, &g_pCommon->vaoID);
  glGenBuffers(1, &g_pCommon->vboVerticesID);
  glGenBuffers(1, &g_pCommon->vboIndicesID);

  glBindVertexArray(g_pCommon->vaoID);

  glBindBuffer(GL_ARRAY_BUFFER, g_pCommon->vboVerticesID);
  // pass vertices to buffer object
  glBufferData(GL_ARRAY_BUFFER, sizeof(g_pCommon->vertices), &g_pCommon->vertices[0], GL_STATIC_DRAW);
  GL_CHECK_ERRORS
  // enabel vertex attribute array for position
  glEnableVertexAttribArray(g_pCommon->shader["vVertex"]);
  glVertexAttribPointer(g_pCommon->shader["vVertex"], 3, GL_FLOAT, GL_FALSE, 0, nullptr);
  GL_CHECK_ERRORS
  // set indices to element arrary buffer
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_pCommon->vboIndicesID);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(g_pCommon->indices), &g_pCommon->indices[0],
               GL_STATIC_DRAW);
  GL_CHECK_ERRORS

  // setup camera position and rotate it
  g_pCommon->cam.SetPosition(glm::vec3(2, 2, 2));
  g_pCommon->cam.Rotate(g_pCommon->rX, g_pCommon->rY, 0);
  // setup camera projection settings and update the matrices
  g_pCommon->cam.SetupProjection(g_pCommon->fov, (static_cast<GLfloat>(Common::WIDTH) / Common::HEIGHT), 1.f, 10);
  g_pCommon->cam.Update();

  // calculate the camera view furstum planes
  g_pCommon->cam.CalcFrustumPlanes();

  // set the world camera position and direction
  g_pCommon->world.SetPosition(glm::vec3(10, 10, 10));
  g_pCommon->world.Rotate(g_pCommon->rX, g_pCommon->rY, 0);
  // set the projection and update the camera settings
  g_pCommon->world.SetupProjection(g_pCommon->fov, (static_cast<GLfloat>(Common::WIDTH) / Common::HEIGHT), 0.1f, 100.0f);
  g_pCommon->world.Update();

  // assign the object cam as the current camera
  g_pCommon->pCurrentCam = &g_pCommon->cam;

  // setup Frustum geometry
  glGenVertexArrays(1, &g_pCommon->vaoFrustumID);
  glGenBuffers(1, &g_pCommon->vboFrustumVerticesID);
  glGenBuffers(1, &g_pCommon->vboFrustumIndicesID);

  // store the view frustum vertices
  g_pCommon->frustum_vertices[0] = g_pCommon->cam.farPts[0];
  g_pCommon->frustum_vertices[1] = g_pCommon->cam.farPts[1];
  g_pCommon->frustum_vertices[2] = g_pCommon->cam.farPts[2];
  g_pCommon->frustum_vertices[3] = g_pCommon->cam.farPts[3];

  g_pCommon->frustum_vertices[4] = g_pCommon->cam.nearPts[0];
  g_pCommon->frustum_vertices[5] = g_pCommon->cam.nearPts[1];
  g_pCommon->frustum_vertices[6] = g_pCommon->cam.nearPts[2];
  g_pCommon->frustum_vertices[7] = g_pCommon->cam.nearPts[3];

  GLushort frustum_indices[36] = {
      0, 4, 3, 3, 4, 7, // top
      6, 5, 1, 6, 1, 2, // bottom
      0, 1, 4, 4, 1, 5, // left
      7, 6, 3, 3, 6, 2, // right
      4, 5, 6, 4, 6, 7, // near
      3, 2, 0, 0, 2, 1, // far
  };
  glBindVertexArray(g_pCommon->vaoFrustumID);

  glBindBuffer(GL_ARRAY_BUFFER, g_pCommon->vboFrustumVerticesID);
  // pass frustum vertices to buffer object
  glBufferData(GL_ARRAY_BUFFER, sizeof(g_pCommon->frustum_vertices), &g_pCommon->frustum_vertices[0],
               GL_DYNAMIC_DRAW);
  GL_CHECK_ERRORS
  // enable vertex attribute array for position
  glEnableVertexAttribArray(g_pCommon->shader["vVertex"]);
  glVertexAttribPointer(g_pCommon->shader["vVertex"], 3, GL_FLOAT, GL_FALSE, 0, nullptr);
  GL_CHECK_ERRORS
  // pass indices to element array buffer
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_pCommon->vboFrustumIndicesID);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(frustum_indices),
               &frustum_indices[0], GL_STATIC_DRAW);
  GL_CHECK_ERRORS

  // setup a set of points
  for (int j = 0; j < Common::PZ; j++) {
    for (int i = 0; i < Common::PX; i++) {
      float x = i / (Common::PX - 1.0f);
      float z = j / (Common::PZ - 1.0f);
      g_pCommon->pointVertices[j * Common::PX + i] = glm::vec3(x, 0, z);
    }
  }
  // setup point vertex array and verex buffer objects
  glGenVertexArrays(1, &g_pCommon->pointVAOID);
  glGenBuffers(1, &g_pCommon->pointVBOID);
  glBindVertexArray(g_pCommon->pointVAOID);
  glBindBuffer(GL_ARRAY_BUFFER, g_pCommon->pointVBOID);
  // pass vertices to buffer object
  glBufferData(GL_ARRAY_BUFFER, sizeof(g_pCommon->pointVertices), g_pCommon->pointVertices,
               GL_STATIC_DRAW);

  // enable vertex attrib array for position
  glEnableVertexAttribArray(g_pCommon->pointShader["vVertex"]);
  glVertexAttribPointer(g_pCommon->pointShader["vVertex"], 3, GL_FLOAT, GL_FALSE, 0, nullptr);

  // get the camera look direction to determine the yaw and pitch amount
  glm::vec3 look = glm::normalize(g_pCommon->cam.GetPosition());
  float yaw = glm::degrees(static_cast<float>(atan2f(look.z, look.x) + static_cast<float>(M_PI)));
  float pitch = glm::degrees(asinf(look.y));
  g_pCommon->rX = yaw;
  g_pCommon->rY = pitch;

  // if filtering is enabled, fill the mouse history buffer
  if (g_pCommon->useFiltering) {
    for (int i = 0; i < Common::MOUSE_HISTORY_BUFFER_SIZE; ++i) {
      g_pCommon->mouseHistory[i] = glm::vec2(g_pCommon->rX, g_pCommon->rY);
    }
  }

  std::cout << "Initialization successfull" << std::endl;

  // get the initial time
  g_pCommon->start_time = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
}

// delete all allocated objects
void OnShutdown() {
  glDeleteQueries(1, &g_pCommon->query);

  // Destroy shader
  g_pCommon->shader.DeleteShaderProgram();
  g_pCommon->pointShader.DeleteShaderProgram();

  // Destroy vao and vbos
  glDeleteBuffers(1, &g_pCommon->vboVerticesID);
  glDeleteBuffers(1, &g_pCommon->vboIndicesID);
  glDeleteVertexArrays(1, &g_pCommon->vaoID);

  // Delete frustum vao and vbos
  glDeleteVertexArrays(1, &g_pCommon->vaoFrustumID);
  glDeleteBuffers(1, &g_pCommon->vboFrustumVerticesID);
  glDeleteBuffers(1, &g_pCommon->vboFrustumIndicesID);

  // Delete point vao/vbo
  glDeleteVertexArrays(1, &g_pCommon->pointVAOID);
  glDeleteBuffers(1, &g_pCommon->pointVBOID);

  std::cout << "Shutdown successfull" << std::endl;
}

// resize event handler
void OnResize(int w, int h) {
  // set the viewport
  glViewport(0, 0, static_cast<GLsizei>(w), static_cast<GLsizei>(h));
}

// idle event processing
void OnIdle() {
	glm::vec3 t = g_pCommon->pCurrentCam->GetTranslation(); 

	if(glm::dot(t,t)>Common::EPSILON2) {
		g_pCommon->pCurrentCam->SetTranslation(t*0.95f);
	}

	//update camera frustum
	if(g_pCommon->pCurrentCam == &g_pCommon->cam) {
		//get updated camera frustum vertices
		g_pCommon->pCurrentCam->CalcFrustumPlanes();
		g_pCommon->frustum_vertices[0] = g_pCommon->cam.farPts[0];
		g_pCommon->frustum_vertices[1] = g_pCommon->cam.farPts[1];
		g_pCommon->frustum_vertices[2] = g_pCommon->cam.farPts[2];
		g_pCommon->frustum_vertices[3] = g_pCommon->cam.farPts[3];

		g_pCommon->frustum_vertices[4] = g_pCommon->cam.nearPts[0];
		g_pCommon->frustum_vertices[5] = g_pCommon->cam.nearPts[1];
		g_pCommon->frustum_vertices[6] = g_pCommon->cam.nearPts[2];
		g_pCommon->frustum_vertices[7] = g_pCommon->cam.nearPts[3];

		//update the frustum vertices on the GPU
		glBindVertexArray(g_pCommon->vaoFrustumID);
			glBindBuffer (GL_ARRAY_BUFFER, g_pCommon->vboFrustumVerticesID);
			glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(g_pCommon->frustum_vertices), &g_pCommon->frustum_vertices[0]);
		glBindVertexArray(0);
	}

  // call the display function
  glutPostRedisplay();
}

// display callback function
void OnRender() {
  // FPS related calcualtion
  ++g_pCommon->total_frames;
  g_pCommon->last_time = g_pCommon->current_time;
  g_pCommon->current_time = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
  g_pCommon->dt = g_pCommon->current_time - g_pCommon->last_time;
  if ((g_pCommon->current_time - g_pCommon->last_time_fps) > 1) {
    g_pCommon->fps = g_pCommon->total_frames / (g_pCommon->current_time - g_pCommon->last_time_fps);
    g_pCommon->last_time_fps = g_pCommon->current_time;
    g_pCommon->total_frames = 0;
  }

  // clear color buffer and depth buffer
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // set the camera transformation
  glm::mat4 MV = g_pCommon->pCurrentCam->GetViewMatrix();
  glm::mat4 P = g_pCommon->pCurrentCam->GetProjectionMatrix();
  glm::mat4 MVP = P * MV;

  // get the frustum planes
  glm::vec4 p[6];
  g_pCommon->pCurrentCam->GetFrustumPlanes(p);

  // begin hardware query
  glBeginQuery(GL_PRIMITIVES_GENERATED, g_pCommon->query);

  // bind point shader
  g_pCommon->pointShader.Use();
  // set shader uniforms
  glUniform1f(g_pCommon->pointShader("t"), g_pCommon->current_time);
  glUniformMatrix4fv(g_pCommon->pointShader("MVP"), 1, GL_FALSE, glm::value_ptr(MVP));
  glUniform4fv(g_pCommon->pointShader("FrustumPlanes"), 6, glm::value_ptr(p[0]));

  // bind the point vertex array object
  glBindVertexArray(g_pCommon->pointVAOID);
  // draw points
  glDrawArrays(GL_POINTS, 0, Common::MAX_POINTS);

  // unbind point shader
  g_pCommon->pointShader.UnUse();

  // end hardware query
  glEndQuery(GL_PRIMITIVES_GENERATED);

  // check the query result to get the total number of visible points
  GLuint res;
  glGetQueryObjectuiv(g_pCommon->query, GL_QUERY_RESULT, &res);
  sprintf(g_pCommon->buffer, "FPS: %3.3f :: Total visible points: %3d", static_cast<double>(g_pCommon->fps), res);
  glutSetWindowTitle(g_pCommon->buffer);

  // set the normal shader
  g_pCommon->shader.Use();
  // bind the vetex array object for ground plane
  glBindVertexArray(g_pCommon->vaoID);
  // set shader uniforms
  glUniformMatrix4fv(g_pCommon->shader("MVP"), 1, GL_FALSE, glm::value_ptr(MVP));
  glUniform4fv(g_pCommon->shader("color"), 1, g_pCommon->white);
  // draw triangles
  glDrawElements(GL_TRIANGLES, Common::TOTAL_INDICES, GL_UNSIGNED_SHORT, nullptr);

  // draw the local cam frustum when world cam is on
  if (g_pCommon->pCurrentCam == &g_pCommon->world) {
    // set shader uniforms
    glUniform4fv(g_pCommon->shader("color"), 1, g_pCommon->red);

    // bind frustum vertex array object
    glBindVertexArray(g_pCommon->vaoFrustumID);

    // enable alpha blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // set polygon mode as fill
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    // draw triangles
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, nullptr);
    // reset the line drawing mode
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // disable blending
    glDisable(GL_BLEND);
  }

  // unbind shader
  g_pCommon->shader.UnUse();

  // swap front and back buffers to show the rendered result
  glutSwapBuffers();
}

// Keyboard event handler to toggle the mouse filtering using spacebar key
void OnKey(unsigned char key, int /*x*/, int /*y*/) {
  switch (key) {
		case '1':
			g_pCommon->pCurrentCam = &g_pCommon->cam;
		break;

		case '2':
			g_pCommon->pCurrentCam = &g_pCommon->world;
		break;
  case ' ':
    g_pCommon->useFiltering = !g_pCommon->useFiltering;
    break;
  case 'w':
    g_pCommon->cam.Walk(g_pCommon->dt);
    break;
  case 's':
    g_pCommon->cam.Walk(-g_pCommon->dt);
    break;
  case 'a':
    g_pCommon->cam.Strafe(-g_pCommon->dt);
    break;
  case 'd':
    g_pCommon->cam.Strafe(g_pCommon->dt);
    break;
  case 'q':
    g_pCommon->cam.Lift(g_pCommon->dt);
    break;
  case 'z':
    g_pCommon->cam.Lift(-g_pCommon->dt);
    break;
  }
  glutPostRedisplay();
}

int main(int argc, char **argv) {
  Common common;
  g_pCommon = &common;

  // freeglut initialization calls
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
  glutInitContextVersion(3, 3);
  glutInitContextFlags(GLUT_CORE_PROFILE | GLUT_DEBUG);
  glutInitWindowSize(Common::WIDTH, Common::HEIGHT);
  glutCreateWindow("Free Camera - OpenGL 3.3");

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
  std::cout << "\tUsing GLEW " << glewGetString(GLEW_VERSION)              << std::endl;
  std::cout << "\tVendor: "    << glGetString(GL_VENDOR)                   << std::endl;
  std::cout << "\tRenderer: "  << glGetString(GL_RENDERER)                 << std::endl;
  std::cout << "\tVersion: "   << glGetString(GL_VERSION)                  << std::endl;
  std::cout << "\tGLSL: "      << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;

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
