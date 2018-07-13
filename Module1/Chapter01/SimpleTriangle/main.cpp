#include <iostream>

#include <GL/glew.h>
#include <GL/freeglut.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "GLSLShader.hpp"

#define GL_CHECK_ERRORS assert(glGetError()== GL_NO_ERROR);

struct Common {
  //screen size
  const int WIDTH  = 1280;
  const int HEIGHT = 960;

  //shader reference
  GLSLShader shader;

  //vertex array and vertex buffer object IDs
  GLuint vaoID;
  GLuint vboVerticesID;
  GLuint vboIndicesID;

  //out vertex struct for interleaved attributes
  struct Vertex {
    glm::vec3 position;
    glm::vec3 color;
  };

  //triangle vertices and indices
  Vertex vertices[3];
  GLushort indices[3];

  //projection and modelview matrices
  glm::mat4  P = glm::mat4(1);
  glm::mat4 MV = glm::mat4(1);
};
static Common* g_pCommon = nullptr;

//OpenGL initialization
void OnInit() {
	GL_CHECK_ERRORS
	//load the shader
	g_pCommon->shader.LoadFromFile(GL_VERTEX_SHADER, "shaders/shader.vert");
	g_pCommon->shader.LoadFromFile(GL_FRAGMENT_SHADER, "shaders/shader.frag");
	//compile and link shader
	g_pCommon->shader.CreateAndLinkProgram();
	g_pCommon->shader.Use();
		//add attributes and uniforms
		g_pCommon->shader.AddAttribute("vVertex");
		g_pCommon->shader.AddAttribute("vColor");
		g_pCommon->shader.AddUniform("MVP");
	g_pCommon->shader.UnUse();

	GL_CHECK_ERRORS

	//setup triangle geometry
	//setup triangle vertices
	g_pCommon->vertices[0].color=glm::vec3(1,0,0);
	g_pCommon->vertices[1].color=glm::vec3(0,1,0);
	g_pCommon->vertices[2].color=glm::vec3(0,0,1);

	g_pCommon->vertices[0].position=glm::vec3(-1,-1,0);
	g_pCommon->vertices[1].position=glm::vec3(0,1,0);
	g_pCommon->vertices[2].position=glm::vec3(1,-1,0);

	//setup triangle indices
	g_pCommon->indices[0] = 0;
	g_pCommon->indices[1] = 1;
	g_pCommon->indices[2] = 2;

	GL_CHECK_ERRORS

	//setup triangle vao and vbo stuff
	glGenVertexArrays(1, &g_pCommon->vaoID);
	glGenBuffers(1,      &g_pCommon->vboVerticesID);
	glGenBuffers(1,      &g_pCommon->vboIndicesID);
	GLsizei stride = sizeof(Common::Vertex);

	glBindVertexArray(g_pCommon->vaoID);

		glBindBuffer (GL_ARRAY_BUFFER, g_pCommon->vboVerticesID);
		//pass triangle verteices to buffer object
		glBufferData (GL_ARRAY_BUFFER, sizeof(g_pCommon->vertices), &g_pCommon->vertices[0], GL_STATIC_DRAW);
		GL_CHECK_ERRORS
		//enable vertex attribute array for position
		glEnableVertexAttribArray(g_pCommon->shader["vVertex"]);
		glVertexAttribPointer(g_pCommon->shader["vVertex"], 3, GL_FLOAT, GL_FALSE, stride, nullptr);
		GL_CHECK_ERRORS
		//enable vertex attribute array for colour
		glEnableVertexAttribArray(g_pCommon->shader["vColor"]);
		glVertexAttribPointer(g_pCommon->shader["vColor"], 3, GL_FLOAT, GL_FALSE,stride, reinterpret_cast<const GLvoid*>(offsetof(Common::Vertex, color)));
		GL_CHECK_ERRORS
		//pass indices to element array buffer
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_pCommon->vboIndicesID);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(g_pCommon->indices), &g_pCommon->indices[0], GL_STATIC_DRAW);
		GL_CHECK_ERRORS 

	std::cout << "Initialization successfull" << std::endl;
}

//release all allocated resources
void OnShutdown() {
	//Destroy shader
	g_pCommon->shader.DeleteShaderProgram();

	//Destroy vao and vbo
	glDeleteBuffers(1,      &g_pCommon->vboVerticesID);
	glDeleteBuffers(1,      &g_pCommon->vboIndicesID);
	glDeleteVertexArrays(1, &g_pCommon->vaoID);

  std::cout << "Shutdown successfull" << std::endl;
}

//resize event handler
void OnResize(int w, int h) {
	//set the viewport size
	glViewport (0, 0, static_cast<GLsizei>(w), static_cast<GLsizei>(h));
	//setup the projection matrix
	g_pCommon->P = glm::ortho(-1,1,-1,1);
}

//display callback function
void OnRender() {
	//clear the colour and depth buffer
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	//bind the shader
	g_pCommon->shader.Use();
		//pass the shader uniform
		glUniformMatrix4fv(g_pCommon->shader("MVP"), 1, GL_FALSE, glm::value_ptr(g_pCommon->P*g_pCommon->MV));
			//drwa triangle
			glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_SHORT, nullptr);
	//unbind the shader
	g_pCommon->shader.UnUse();
	 
	//swap front and back buffers to show the rendered result
	glutSwapBuffers();
}

int main(int argc, char** argv) {
  Common common;
  g_pCommon = &common;

	//freeglut initialization calls
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitContextVersion (3, 3);
	glutInitContextFlags (GLUT_CORE_PROFILE | GLUT_DEBUG);
	glutInitWindowSize(g_pCommon->WIDTH, g_pCommon->HEIGHT);
	glutCreateWindow("Simple triangle - OpenGL 3.3");

	//glew initialization
	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
	if (GLEW_OK != err)	{
    std::cerr << "Error: " << glewGetErrorString(err) << std::endl;
	} else {
		if (GLEW_VERSION_3_3)
		{
      std::cout << "Driver supports OpenGL 3.3\nDetails:" << std::endl;
		}
	}
	err = glGetError(); //this is to ignore INVALID ENUM error 1282
	GL_CHECK_ERRORS

	//print information on screen
	std::cout << "\tUsing GLEW " <<glewGetString(GLEW_VERSION)              << std::endl;
	std::cout << "\tVendor: "    <<glGetString(GL_VENDOR)                   << std::endl;
	std::cout << "\tRenderer: "  <<glGetString(GL_RENDERER)                 << std::endl;
	std::cout << "\tVersion: "   <<glGetString(GL_VERSION)                  << std::endl;
	std::cout << "\tGLSL: "      <<glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;

	GL_CHECK_ERRORS

	//opengl initialization
	OnInit();

	//callback hooks
	glutCloseFunc(OnShutdown);
	glutDisplayFunc(OnRender);
	glutReshapeFunc(OnResize);

	//main loop call
	glutMainLoop();

	return 0;
}
