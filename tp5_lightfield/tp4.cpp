/* -------------------------------------------------------- */
/* -------------------------------------------------------- */
/*

TP OpenGL - Image Synthesis course

George Drettakis, Adrien Bousseau, Sylvain Lefebvre

mailto: FirstName.LastName@inria.fr

REVES / INRIA

History:
- Adapted to OpenGL 3, 2014-02 (AB)

Documentation:
- OpenGL tutorial, provides detailed instructions but relies on a different representation of matrices and 3D models
http://www.opengl-tutorial.org/beginners-tutorials/
- GLSL reference
https://www.opengl.org/documentation/glsl/


Questions:

Q0
  - Launch the application: what do you see? 
  - Make the window as big as possible to see the little views.

Q1 
  - The lightfield contains 9x9 views. Update the pixel shader to display only the central view.
  - Implement an interactive change of view using the x and y mouse coordinates (show the top-left view when the mouse is in the top left corner). 
    You need to send the mouse coordinates to the shader, and then display the view accordingly.
  
Q2
  - Modify the shader to average all the views of the lightfield. You should now obtain a shallow deopth of field where only part of the scene is sharp while other parts are blurry.
  
Q3
  A lightfield can be synthetically refocused by translating the views with respect to the central one (see refocusing_shift.png, where (i,j) 
  denotes the coordinates of the view in the 9x9 grid). To implement refocusing, you need to:
  - Translate each view (i,j) by alpha*(i-i0, j-j0), where (i0=4, j0=4) is the central view. Alpha controls the position of the 
    plane in focus in the scene. Try varying alpha between -0.0003 and 0.0003
  - Implement an interactive change of focus using the y mouse coordinates to control alpha.
  To focus a region, please click on that region
  alpha can also be changed by pressing '+' or '-'.
*/

#ifdef _WIN32
#include <windows.h>        // windows API header (required by gl.h)
#endif

#include "glew/include/GL/glew.h"	// Support for shaders

#ifdef __APPLE__

#include <gl3.h>          // OpenGL header
#include <gl3ext.h>       // OpenGL header
#include <glut.h>         // OpenGL Utility Toolkit header

#else

#include <GL/gl.h>          // OpenGL header
#include <GL/glu.h>         // OpenGL Utilities header
#include <GL/glut.h>        // OpenGL Utility Toolkit header

#endif

#include <cstdio>
#include <cmath>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>

#include "glsl.h" // GLSL Helper functions
#include "TextureLoader.h"

//path of the shaders
#define SRC_PATH	""

#include <iostream>
using namespace std;

/* --------------------- Main Window ------------------- */

int          g_MainWindow; // glut Window Id
int          g_W=640;      // window width
int          g_H=480;      // window height


/* --------------------- Geometry ------------------- */

//Vertex Array Object
GLuint VertexArrayID;
// This will identify our vertex buffer
GLuint vertexbuffer;
// This will identify our texture coordinate buffer
GLuint uvbuffer;
// This will identify the texture
GLuint textureID;


/*---------------------- Shaders -------------------*/
GLuint g_glslProgram;


/* -------------------------------------------------------- */

int mouseX;
int mouseY;
float alpha = 0.000;
int scroll = 0;
float delta = 0.00001;
void mainKeyboard(unsigned char key, int x, int y) 
{
	if (key == 'q') {
		exit (0);
	} else if (key == ' ') {
		printf("spacebar pressed\n");
	} else if (key == '+') {
		printf("+ pressed\n");
		alpha += delta;
		if(alpha >= 0.0003)
			alpha = -delta;

	} else if (key == '-') {
		printf("- pressed\n");
		alpha -= delta;
		if(alpha <= -0.0003)
			alpha = 0.00003;
	}

	printf("alpha: %f\n", alpha);	

	printf("key '%c' pressed\n",key);
}

/* -------------------------------------------------------- */

void mainMouse(int btn, int state, int x, int y) 
{
	if (state == GLUT_DOWN) {
		if (btn == GLUT_LEFT_BUTTON) {
			printf("Left mouse button pressed at coordinates %d,%d\n",x,y);

			float block_size = g_H / 7.;			
			alpha = ((int)(y / block_size) - 3) * 0.0001;
			printf("%f\n", alpha);

		} else if (btn == GLUT_RIGHT_BUTTON) {
			printf("Right mouse button pressed at coordinates %d,%d\n",x,y);

		}
	} else if (state == GLUT_UP) {
		if (btn == GLUT_LEFT_BUTTON) {
			printf("Left mouse button release at coordinates %d,%d\n",x,y);
		} else if (btn == GLUT_RIGHT_BUTTON) {
			printf("Right mouse button release at coordinates %d,%d\n",x,y);
		}
	}
}

/* -------------------------------------------------------- */

void mainMotion(int x,int y)
{
	
	mouseX = x / (g_W / 9);
	mouseY = y / (g_H / 9);
	printf("Mouse is at %d,%d, %d, %d\n",x, y, mouseX, mouseY);

	if(y - scroll > 0){	
		alpha += delta;
		if(alpha >= 0.0003)
			alpha = -0.0003;
	}
	else{
		alpha -= delta;
		if(alpha <= -0.0003)
			alpha = 0.0003;
	}
	printf("alpha: %f\n", alpha);	
	scroll = y;
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
	// Dark background
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	
	// clear screen and depth buffer
	glClear(GL_COLOR_BUFFER_BIT);
	
	// use our shader
	glUseProgramObjectARB(g_glslProgram);	
	
	//--- Camera ---//
	// No camera, just draw a quad over the entire screen	
	
	GLuint mouseXID = glGetUniformLocation(g_glslProgram, "mouseX");
	glUniform1i(mouseXID, mouseX);

	GLuint mouseYID = glGetUniformLocation(g_glslProgram, "mouseY");
	glUniform1i(mouseYID, mouseY);

	GLuint alphaID = glGetUniformLocation(g_glslProgram, "alpha");
	glUniform1f(alphaID, alpha);

	//--- Texturing: cover the quad with the image
	// Get a handle for our "myTextureSampler" uniform
	GLuint TextureID  = glGetUniformLocation(g_glslProgram, "myTextureSampler");
	// Bind our texture in Texture Unit 0
	glActiveTexture(GL_TEXTURE0);
    //glBindTexture(GL_TEXTURE_2D, treeTexture.texID); //if using TGA image
	glBindTexture(GL_TEXTURE_2D, textureID);
	// Set our "myTextureSampler" sampler to user Texture Unit 0
	glUniform1i(TextureID, 0); 
	
	
	//--- Geometry ---//
	// 1rst attribute buffer : vertices
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glVertexAttribPointer(
	  0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
	  2,                  // size, 2 coordinates per vertex (x,y)
	  GL_FLOAT,           // type
	  GL_FALSE,           // normalized?
	  0,                  // stride
	  (void*)0            // array buffer offset
	);
	
	// 2nd attribute buffer : UVs
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
	glVertexAttribPointer(
	    1,                                // attribute. No particular reason for 1, but must match the layout in the shader.
	    2,                                // size, 2 texture coordinates (u,v)
	    GL_FLOAT,                         // type
	    GL_FALSE,                         // normalized? No
	    0,                                // stride
	    (void*)0                          // array buffer offset
	);
	
	// Draw the quad (2 triangles)
	glDrawArrays(GL_TRIANGLES, 0, 6); // Starting from vertex 0; 3 vertices total -> 2 triangle.
 
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	
	
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

void loadShaders()
{
	const char *fp_code=loadStringFromFile(SRC_PATH "SimpleFragmentShader.fp");
	const char *vp_code=loadStringFromFile(SRC_PATH "SimpleVertexShader.vp");
	g_glslProgram = createGLSLProgram(vp_code,fp_code);
	delete [](fp_code);
	delete [](vp_code);
}

/* -------------------------------------------------------- */

void createGeometry()
{
	// Create a quad made of two triangles. A triangle contains 3 vertices, each vertex has 2 coordinates (screen space)
	// Position
	GLfloat* g_vertex_buffer_data;
	g_vertex_buffer_data = new GLfloat[6*2];
	//bottom right triangle. Note: the OpenGL screen covers [-1;1] x [-1;1]
	g_vertex_buffer_data[0] = -1.0f; g_vertex_buffer_data[1] = -1.0f; 
	g_vertex_buffer_data[2] = 1.0f; g_vertex_buffer_data[3] = -1.0f; 
	g_vertex_buffer_data[4] = 1.0f; g_vertex_buffer_data[5] = 1.0f; 
	//top left triangle
	g_vertex_buffer_data[6] = -1.0f; g_vertex_buffer_data[7] = -1.0f; 
	g_vertex_buffer_data[8] = 1.0f; g_vertex_buffer_data[9] = 1.0f; 
	g_vertex_buffer_data[10] = -1.0f; g_vertex_buffer_data[11] = 1.0f; 
	
	// UVs
	GLfloat* g_uv_buffer_data;
	g_uv_buffer_data = new GLfloat[6*2];
	//bottom right triangle. Set the coordinates so that the quad covers [0;1] x [0;1]
	g_uv_buffer_data[0] = 0.0f; g_uv_buffer_data[1] = 1.0f;
	g_uv_buffer_data[2] = 1.0f; g_uv_buffer_data[3] = 1.0f;
	g_uv_buffer_data[4] = 1.0f; g_uv_buffer_data[5] = 0.0f;
	//top left triangle
	g_uv_buffer_data[6] = 0.0f; g_uv_buffer_data[7] = 1.0f;
	g_uv_buffer_data[8] = 1.0f; g_uv_buffer_data[9] = 0.0f;
	g_uv_buffer_data[10] = 0.0f; g_uv_buffer_data[11] = 0.0f;
	

	//--- Send the geometry to OpenGL
	// We need a vertex array object
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);
	// Generate 1 vertex buffer, put the resulting identifier in vertexbuffer
	glGenBuffers(1, &vertexbuffer);
	// Make the buffer active
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	// Give our vertices to OpenGL.
	glBufferData(GL_ARRAY_BUFFER, 12*sizeof(GLfloat), g_vertex_buffer_data, GL_STATIC_DRAW); 
	//same for UVs...
	glGenBuffers(1, &uvbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
	glBufferData(GL_ARRAY_BUFFER, 12*sizeof(GLfloat), g_uv_buffer_data, GL_STATIC_DRAW);
	
	
	//clean up
	delete [] g_vertex_buffer_data;
	delete [] g_uv_buffer_data;
}

void loadTexture()
{
    
    unsigned int width, height;
    unsigned char * data = loadBMPRaw("images/lego_9x9.bmp", width, height);
    // unsigned char * data = loadBMPRaw("images/ball_9x9.bmp", width, height);
    // Create one OpenGL texture
    glGenTextures(1, &textureID);
    
    // "Bind" the newly created texture : all future texture functions will modify this texture
    glBindTexture(GL_TEXTURE_2D, textureID);
    // Give the image to OpenGL
    glTexImage2D(GL_TEXTURE_2D, 0,GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, data);
    // Poor filtering
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    
    delete[] data;
}

/* -------------------------------------------------------- */

void cleanUp()
{
	glDeleteBuffers(1, &vertexbuffer);
	glDeleteBuffers(1, &uvbuffer);
	glDeleteVertexArrays(1, &VertexArrayID);
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
	g_MainWindow=glutCreateWindow("TP2");
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

	///
	/// Shaders, geometry, texture and camera
	///
	
	//need to init Glew before anything else
#ifdef __APPLE__
    glewExperimental = GL_TRUE;
#endif
    glewInit();
	
	loadShaders();
	
	//Send the geometry to OpenGL
	createGeometry();
    
    loadTexture();

	// print a small documentation
	printf("[q]     - quit\n");

	// enter glut main loop - this *never* returns
	glutMainLoop();
	
	cleanUp();
}
/* -------------------------------------------------------- */