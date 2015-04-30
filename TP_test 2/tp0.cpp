/* -------------------------------------------------------- */
/* -------------------------------------------------------- */
/*

TP OpenGL - Image Synthesis course

	George Drettakis, Adrien Bousseau, Sylvain Lefebvre

mailto: FirstName.LastName@inria.fr

REVES / INRIA

History:
- Adapted to OpenGL 3, 2014-02 (AB)

*/

#define printOpenGLError() printOglError(__FILE__, __LINE__)
extern int printOglError(char*, int);

#ifdef NODEF
#ifdef _WIN32

#include <windows.h>        // windows API header (required by gl.h)
#include "glew/include/GL/glew.h" // Support for shaders
//#include <glew.h>          // 
#include <GL/gl.h>          // OpenGL header
#include <GL/glu.h>         // OpenGL Utilities header
#include <GL/glut.h>        // OpenGL Utility Toolkit header


#elif __APPLE__

/*
#include <glew.h>        // OpenGL Utility Toolkit header
//#include "glew/include/GL/glew.h"		// Support for shaders
//#include <gl.h>          // OpenGL header
//#include <gl3.h>          // OpenGL header
#include <glu.h>         // OpenGL Utilities header
#include <glut.h>        // OpenGL Utility Toolkit header
*/
#include <stdlib.h>

#else


#include "glew/include/GL/glew.h"		// Support for shaders
#include <GL/gl.h>          // OpenGL header
#include <GL/glu.h>         // OpenGL Utilities header
#include <GL/glut.h>        // OpenGL Utility Toolkit header
#include <stdlib.h>

#endif

#include <cstdio>
#include <cmath>


//path of the shaders
#endif
#ifdef _WIN32
#define SRC_PATH	""
#else
#define SRC_PATH	""
#endif


#include "glew/include/GL/glew.h"		// Support for shaders
#ifdef __APPLE__
#include <gl3.h>          // OpenGL header
#include <gl3ext.h>          // OpenGL header
#include <glut.h>        // OpenGL Utility Toolkit header
#else
#include <GL/gl.h>          // OpenGL header
#include <GL/glut.h>        // OpenGL Utility Toolkit header
#endif
#include <stdlib.h>
#include <stdio.h>
#include <iostream>

GLhandleARB createGLSLProgram(const char *vp_code,const char *fp_code);
const char *loadStringFromFile(const char *fname);

/* --------------------- Main Window ------------------- */

int          g_MainWindow; // glut Window Id
int          g_W=640;      // window width
int          g_H=480;      // window width


/* --------------------- Geometry ------------------- */
//Vertex Array Object
//unsigned int VertexArrayID;

// An array of 3 vectors which represents 3 vertices
static const float g_vertex_buffer_data[] = {
  -1.0f, -1.0f, 0.0f,
  1.0f, -1.0f, 0.0f,
  0.0f,  1.0f, 0.0f,
};

// This will identify our vertex buffer
//unsigned int 
GLuint vertexbuffer;
GLuint elementbuffer;

/*---------------------- Shaders -------------------*/
unsigned int g_glslProgram;


/* -------------------------------------------------------- */

void mainKeyboard(unsigned char key, int x, int y) 
{
	if (key == 'q') {
		exit (0);
	} else if (key == ' ') {
		printf("spacebar pressed\n");
	} 

	printf("key '%c' pressed\n",key);
}

/* -------------------------------------------------------- */

void mainMouse(int btn, int state, int x, int y) 
{
	if (state == GLUT_DOWN) {
		if (btn == GLUT_LEFT_BUTTON) {
			printf("Left mouse button pressed at coordinates %d,%d\n",x,y);
		} else if (btn == GLUT_RIGHT_BUTTON) {
			printf("Right mouse button pressed at coordinates %d,%d\n",x,y);
		}
	} else if (state == GLUT_UP) {
		if        (btn == GLUT_LEFT_BUTTON) {
			printf("Left mouse button release at coordinates %d,%d\n",x,y);
		} else if (btn == GLUT_RIGHT_BUTTON) {
			printf("Right mouse button release at coordinates %d,%d\n",x,y);
		}
	}
}

/* -------------------------------------------------------- */

void mainMotion(int x,int y)
{
	printf("Mouse is at %d,%d\n",x,y);
}

/* -------------------------------------------------------- */

void mainReshape(int w,int h)
{
	printf("Resizing window to %d,%d\n",w,h);
	g_W=w;
	g_H=h;
	// set viewport to the entire window
	glViewport(0,0,g_W,g_H);
}


/* -------------------------------------------------------- */

void mainRender()
{
	// Dark blue background
	glClearColor(0.0f, 0.0f, 0.4f, 0.0f);
	
	// clear screen and depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	printOpenGLError();
	
	
	// use our shader
	glUseProgram(g_glslProgram);
	printOpenGLError();
	// 1rst attribute buffer : vertices
	glEnableVertexAttribArray(0);
	printOpenGLError();
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	printOpenGLError();
	glVertexAttribPointer(
	  0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
	  3,                  // size, we have 3 vertices
	  GL_FLOAT,           // type
	  GL_FALSE,           // normalized?
	  0,                  // stride
	  (void*)0            // array buffer offset
	);
 
	// Draw the triangle !
	printOpenGLError();
	glDrawArrays(GL_TRIANGLES, 0, 3); // Starting from vertex 0; 3 vertices total -> 1 triangle
	printOpenGLError();
 
	glDisableVertexAttribArray(0);
	
	
	// swap - this call exchanges the back and front buffer
	// swap is synchronized on the screen vertical sync
	glutSwapBuffers();
}

/* -------------------------------------------------------- */

void idle( void )
{
	// whenever the application has free time, ask for a screen refresh
	glutPostRedisplay();
}

/* -------------------------------------------------------- */

int
main(int argc, char **argv) 
{
	///
	///  glut Window
	///
	// main glut init
	glutInit(&argc, argv);
	// initial window size
	glutInitWindowSize(g_W,g_H); 
	// init display mode
#ifdef __APPLE__
	glutInitDisplayMode( GLUT_3_2_CORE_PROFILE |  GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
#else
	glutInitDisplayMode( GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
#endif
	// create main window
	g_MainWindow=glutCreateWindow("TP0");


	// set main window as current window
	glutSetWindow(g_MainWindow);
	/// setup glut callbacks
	// mouse (whenever a button is pressed)
	glutMouseFunc(mainMouse);
	// motion (whenever the mouse is moved *while* a button is down)
	glutMotionFunc(mainMotion);
	// keyboard (whenever a character key is pressed)
	glutKeyboardFunc(mainKeyboard);
	// display  (whenerver the screen needs to be painted)
	glutDisplayFunc(mainRender);
	// reshape (whenever the window size changes)
	glutReshapeFunc(mainReshape);
	// idle (whenever the application as some free time)
	glutIdleFunc(idle);

	// 
	printf("OpenGL version %s\n", glGetString(GL_VERSION));
	printOpenGLError();
	///
	/// Shaders and geometry
	///
	
	//need to init Glew before anything else
	printOpenGLError();
	glewExperimental = GL_TRUE;
	glewInit();
		GLuint vao[10];
		glGenVertexArrays(1, vao);
		glBindVertexArray(vao[0]);

	//--- Load the shaders
	const char *fp_code=loadStringFromFile(SRC_PATH "SimpleFragmentShader.fp");
	const char *vp_code=loadStringFromFile(SRC_PATH "SimpleVertexShader.vp");

	g_glslProgram = createGLSLProgram(vp_code,fp_code);

//	delete [](fp_code);
//	delete [](vp_code);
	
	//--- Send the geometry to OpenGL
	// We need a vertex array object
	// Generate 1 vertex buffer, put the resulting identifier in vertexbuffer
	glGenBuffers(1, &vertexbuffer);

	// Make the buffer active
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);

	// Give our vertices to OpenGL.
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);


	short I[] = {0,1,2};
	glGenBuffers(1,&elementbuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(I), I, GL_STATIC_DRAW);


	// print a small documentation
	printf("[q]     - quit\n");

	// enter glut main loop - this *never* returns
	glutMainLoop();
	
	glDeleteBuffers(1, &vertexbuffer);
}

/* -------------------------------------------------------- */

