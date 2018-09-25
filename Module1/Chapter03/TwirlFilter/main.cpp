// This is an open source non-commercial project. Dear PVS-Studio, please check
// it. PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include <iostream>

#include <GL/glew.h>
#include <GL/freeglut.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <SOIL/SOIL.h>

#include "GLSLShader.hpp"

#define GL_CHECK_ERRORS assert(glGetError()== GL_NO_ERROR);

struct Common {
  // screen size
  static constexpr int WIDTH = 1280;
  static constexpr int HEIGHT = 960;

  //shader for recipe
  GLSLShader shader;

  //vertex array and vertex buffer object IDs
  GLuint vaoID;
  GLuint vboVerticesID;
  GLuint vboIndicesID;

  //texture image ID
  GLuint textureID;

  //vertices and indices arrays for fullscreen quad
  glm::vec2 vertices[4];
  GLushort indices[6];

  //texture image filename
  const std::string filename = "media/Lenna.png";

  //amount of twirl
  float twirl_amount = 0;
};
static Common *g_pCommon = nullptr;

//OpenGL initialization
void OnInit() {
	GL_CHECK_ERRORS
	//load shader
	g_pCommon->shader.LoadFromFile(GL_VERTEX_SHADER,   "shaders/Twirl.vert");
	g_pCommon->shader.LoadFromFile(GL_FRAGMENT_SHADER, "shaders/Twirl.frag");
	//compile and link shader
	g_pCommon->shader.CreateAndLinkProgram();
	g_pCommon->shader.Use();
		//add attributes and uniforms
		g_pCommon->shader.AddAttribute("vVertex");
		g_pCommon->shader.AddUniform("textureMap");
		g_pCommon->shader.AddUniform("twirl_amount");
		//pass values of constant uniforms at initialization
		glUniform1i(g_pCommon->shader("textureMap"), 0);
		glUniform1f(g_pCommon->shader("twirl_amount"), g_pCommon->twirl_amount);
	g_pCommon->shader.UnUse();

	GL_CHECK_ERRORS

	//setup quad geometry
	//setup quad vertices
	g_pCommon->vertices[0] = glm::vec2(0.0,0.0);
	g_pCommon->vertices[1] = glm::vec2(1.0,0.0);
	g_pCommon->vertices[2] = glm::vec2(1.0,1.0);
	g_pCommon->vertices[3] = glm::vec2(0.0,1.0);

	//fill quad indices array
	GLushort* id=&g_pCommon->indices[0];
	*id++ =0;
	*id++ =1;
	*id++ =2;
	*id++ =0;
	*id++ =2;
	*id++ =3;

	GL_CHECK_ERRORS

	//setup quad vao and vbo stuff
	glGenVertexArrays(1, &g_pCommon->vaoID);
	glGenBuffers(1, &g_pCommon->vboVerticesID);
	glGenBuffers(1, &g_pCommon->vboIndicesID);

	glBindVertexArray(g_pCommon->vaoID);
		glBindBuffer (GL_ARRAY_BUFFER, g_pCommon->vboVerticesID);
		//pass quad vertices to buffer object
		glBufferData (GL_ARRAY_BUFFER, sizeof(Common::vertices), &g_pCommon->vertices[0], GL_STATIC_DRAW);
		GL_CHECK_ERRORS
		//enable vertex attribute array for position
		glEnableVertexAttribArray(g_pCommon->shader["vVertex"]);
		glVertexAttribPointer(g_pCommon->shader["vVertex"], 2, GL_FLOAT, GL_FALSE, 0, nullptr);
		GL_CHECK_ERRORS
		//pass quad indices to element array buffer
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_pCommon->vboIndicesID);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(g_pCommon->indices), &g_pCommon->indices[0], GL_STATIC_DRAW);
		GL_CHECK_ERRORS


	//load image using SOIL
	int texture_width = 0, texture_height = 0, channels=0;
	GLubyte* pData = SOIL_load_image(g_pCommon->filename.c_str(), &texture_width, &texture_height, &channels, SOIL_LOAD_AUTO);

	//vertically flip the heightmap image on Y axis since it is inverted
	int i,j;
	for( j = 0; j*2 < texture_height; ++j ) {
		int index1 = j * texture_width * channels;
		int index2 = (texture_height - 1 - j) * texture_width * channels;
		for( i = texture_width * channels; i > 0; --i ) {
			GLubyte temp = pData[index1];
			pData[index1] = pData[index2];
			pData[index2] = temp;
			++index1;
			++index2;
		}
	}

	//setup OpenGL texture and bind to texture unit 0
	glGenTextures(1, &g_pCommon->textureID);
		glActiveTexture(GL_TEXTURE0);		
		glBindTexture(GL_TEXTURE_2D, g_pCommon->textureID);

		//set texture parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

		//allocate texture 
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texture_width, texture_height, 0, GL_RGB, GL_UNSIGNED_BYTE, pData);

	//free SOIL image data
	SOIL_free_image_data(pData);

	GL_CHECK_ERRORS

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
	glDeleteTextures(1, &g_pCommon->textureID);
  std::cout << "Shutdown successfull" << std::endl;
}

//resize event handler
void OnResize(int w, int h) {
	//set the viewport
  glViewport(0, 0, static_cast<GLsizei>(w), static_cast<GLsizei>(h));
}

//display function
void OnRender() {
	//clear the colour and depth buffers
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	
	//bind shader
	g_pCommon->shader.Use();
		//set shader uniform
		glUniform1f(g_pCommon->shader("twirl_amount"), g_pCommon->twirl_amount);
			//draw the full screen quad
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr);
	//unbind shader
	g_pCommon->shader.UnUse();
	
	//swap front and back buffers to show the rendered result
	glutSwapBuffers();
}

//keyboard event handler to change the twirl amount
void OnKey(unsigned char key, int /*x*/, int /*y*/) {
	switch(key) {
		case '-': g_pCommon->twirl_amount -= 0.1f; break;
		case '+': g_pCommon->twirl_amount += 0.1f; break;
	}
	//call display function
	glutPostRedisplay();
}

int main(int argc, char** argv) {
  Common common;
  g_pCommon = &common;

	//freeglut initialization calls
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitContextVersion (3, 3);
	glutInitContextFlags (GLUT_CORE_PROFILE | GLUT_DEBUG);
  glutInitWindowSize(Common::WIDTH, Common::HEIGHT);
	glutCreateWindow("Twirl filter - OpenGL 3.3");

	//glew initialization
	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
	if (GLEW_OK != err)	{
    std::cerr << "Error: "<<glewGetErrorString(err) << std::endl;
	} else {
		if (GLEW_VERSION_3_3) {
      std::cout << "Driver supports OpenGL 3.3\nDetails:" << std::endl;
		}
	}
	err = glGetError(); //this is to ignore INVALID ENUM error 1282
	GL_CHECK_ERRORS

	//print information on screen
  std::cout << "\tUsing GLEW " << glewGetString(GLEW_VERSION)              << std::endl;
  std::cout << "\tVendor: "    << glGetString(GL_VENDOR)                   << std::endl;
  std::cout << "\tRenderer: "  << glGetString(GL_RENDERER)                 << std::endl;
  std::cout << "\tVersion: "   << glGetString(GL_VERSION)                  << std::endl;
  std::cout << "\tGLSL: "      << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;

  std::cout << "Press '-' key to reduce twirl amount\n      '+' key to increase twirl amount\n";
	GL_CHECK_ERRORS

	//initialization of OpenGL
	OnInit();

	//callback hooks
	glutCloseFunc(OnShutdown);
	glutDisplayFunc(OnRender);
	glutReshapeFunc(OnResize);
	glutKeyboardFunc(OnKey);
			
	//main loop call
	glutMainLoop();

	return 0;
}
