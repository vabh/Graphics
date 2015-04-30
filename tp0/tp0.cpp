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

/* -------------------------------------------------------- */
/*

TP0 - Description

This first TP is dedicated to glut and basic OpenGL operations.

* Documentation:
---------------

You *will* need to access the documentation.
- OpenGL reference  http://www.mevis.de/opengl/opengl.html (more online => use google)
- glut   reference  http://www.opengl.org/documentation/specs/glut/spec3/spec3.html

* Reference:
------------
- OpenGL web site       http://www.opengl.org
- OpenGL specifications http://www.opengl.org/documentation/specs/


Exercises guidelines:
---------------------

=> For each exercise, markers have been placed into the file.
   For instance, markers [1A] and [1B] mark locations of interest
   for exercise 1.
=> As a generic rule, *do not delete anything*: use '/*' comments
   to deactivate parts of the code.
=> Anytime you wonder what the parameters to a function mean or should
   be: *use the documentation*.

Exercices:
----------

  Ex1) (markers [1A] and [1B])

   - Change the color background when pressing the following keys:
     '1' => red; '2' => green; '3' => blue

 Ex2)  Open files SimpleFragmentShader.fp SimpleVertexShader.vp and try and 
      understand what they do. Summarize their functionality:
      	Answer:... 

        FragmentShader is for the pixels that are to be rendered.
        In this case I was able to change the color of the triangle.
        VertexShader is for operations on each vertex of the polygons that 
        are beign rendered. For example, by chaging the 'w', I can change the
        coordinates of the image.

 Ex3) (marker [3A][3B][3C][3D][3SA])
   - study the lines of the markers A-C -- what are these lines ?
     	Answer: ....
      3A - Colour at each vertex
      3C - Associate shader variables and the corresponding data

   - uncomment marker [3Db] and comment [3Da] 
     what have you done with this change:
     	Answer:...
      Loading a different fragment shader program, The triangle is
      no longer visible. The fragment shader program takes as input fragmentColor which is not provided.

   - uncomment marker [3SA] in SimpleVertexShader.vp 
     what have you done with this change:
	Answer: ...
    The vertices are coloured according to the values
    I specified in GLfloat g_color_buffer_data[]
    SimpleFragmentShader_col.fp takes as input 'fragmentColor' and provides as output 'color'

   - Display a triangle with red on topmost vertex, green on bottom left
     and blue on bottom right. To do this, modify the lines in [3A]

 Ex4) (markers [4A][4B][4C][4D])
   - Uncomment lines [4A] and [4D], comment out line  [4C]. Examine the shader
     SimpleVertexShader_MVP.vp and compare it to SimpleVertexShader.vp.
     What is the difference ? 
     	Answer:...
      We load a different vertexshader program.
      The ShaderProgram gets as input a matrix MVP. 
      The position of the pixels is tranformed according
      to this matrix.

     What do the lines around 4B do ?
     	Answer:...
      MatrixID is linked to the matrix in the shader program.
      It is used to link the MVP matrix to the shader program

   - Change line [4E] so that the triangle is upright and part of it is outside the screen
      I changed the camera postion to (0, 0, 3), so that we look along the z-axis and straight at the x-y plane. This makes the triangle upright. I changed the viewing point of the camera to (2, 0, 0) so that we look to one side of the centre. This makes part of the triangle go outisde the screen.

 Ex5)
   - Show a wireframe triangle using glPolygonMode (see OpenGL documentation)
   - Use the 'w' key to switch the wireframe mode on/off

 Ex6)
   - Modify the code so that we can adjust the view distance from the triangle 
     using the '+' and '-' keys.
     I translate the object along the z-axis to change the view distance.

 Ex7) Please look at the slides for this exercise; also examine Matrix4x4.h to
    understand which constructor to use for Matrix4x4.h

   - Make the triangle automatically rotate around the z axis
     To do this you will need to create a matrix and multiply MVP appropriately.

   - Make the triangle move back and forth on the x axis between [-1,1], 
     while still rotating on itself along the z axis.

     I create the rotation and translation matrix.
     Press 'r', 't' to start/stop rotation/translation.

 Ex8)
   - Modify the code so that we can adjust the view distance from the triangle
     using the mouse left button + movement.

     press left mouse button and move the  mouse upwards to zoom in, downwards to zoom out.

*/
/* -------------------------------------------------------- */

#ifdef _WIN32

#include <windows.h>        // windows API header (required by gl.h)
#include "glew/include/GL/glew.h" // Support for shaders
#include <GL/gl.h>          // OpenGL header
#include <GL/glu.h>         // OpenGL Utilities header
#include <GL/glut.h>        // OpenGL Utility Toolkit header

#elif __APPLE__

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
#include <stdlib.h>
#include <stdio.h>
#include <iostream>

#include "glsl.h" // GLSL Helper functions
#include "Matrix4x4.h" //class to represent matrices
#include "Vertex.h" //class to represent vertices


//path of the shaders
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

GLhandleARB createGLSLProgram(const char *vp_code,const char *fp_code);
const char *loadStringFromFile(const char *fname);

/* --------------------- Main Window ------------------- */

int          g_MainWindow; // glut Window Id
int          g_W=640;      // window width
int          g_H=480;      // window width


/* --------------------- Geometry ------------------- */
//Vertex Array Object
GLuint VertexArrayID;

// An array of 3 vectors which represents 3 vertices
static const GLfloat g_vertex_buffer_data[] = {
	// [2A] 
  
  1.0f, -1.0f, 0.0f,
  -1.0f, -1.0f, 0.0f,
  0.0f,  1.0f, 0.0f,
};

// One color for each vertex.  [3A]
static const GLfloat g_color_buffer_data[] = {
	// add in three lines here for the 3 colors as requested
	// the syntax is as above for the vertex data
      0, 0, 1, 1, // right
      0, 1, 0, 1, // left
      1, 0, 0, 1  // top
};

// This will identify our vertex buffer
GLuint vertexbuffer;

// This will identify our color buffer
GLuint colorbuffer;

/*---------------------- Shaders -------------------*/
GLuint g_glslProgram;

/*---------------------- Camera --------------------*/
m4x4f MVP; //model-view-projection matrix

/* -------------------------------------------------------- */

/*my params*/
void createMatrices();

int colour = 3;
int wireframe = 0;
float view_distance = -2.;

float delta = 0.5f;
int theta = 0;

float posX = 0.f;
float move = 0.01;

int translate = 1;
int rotate = 1;

int mouseX = 0;
int mouseY = 0;

void mainKeyboard(unsigned char key, int x, int y) 
{
	if (key == 'q') {
		exit (0);
	} else if (key == ' ') {
		printf("spacebar pressed\n");
	}  // [1A]
	else if (key == '1'){
		colour = 1;
	}
	else if (key == '2'){
		colour = 2;
	}
	else if (key == '3'){
		colour = 3;
	}
	else if (key == 'w'){
		wireframe = !wireframe;
	}
	else if (key == '+'){
		view_distance += delta;
		// camx += move;
		// createMatrices();
	}
	else if (key == '-'){
		view_distance -= delta;
		// camx -= move;
		// createMatrices();
	}
	else if (key == 'r'){
		rotate = !rotate;
	}
	else if (key == 't'){
		translate = !translate;
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
	printf("Mouse is at %d,%d\n",x,y);

	if(mouseY - y > 0){
		view_distance += delta;
	}
	else{
		view_distance -= delta;
	}

	mouseX = x;
	mouseY = y;
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
	if(colour == 1)
		glClearColor(0.4f, 0.0f, 0.0f, 0.0f); // red
	else if (colour == 2)
		glClearColor(0.0f, 0.4f, 0.0f, 0.0f); // green
	else if (colour == 3)
		glClearColor(0.0f, 0.0f, 0.4f, 0.0f); // blue
	
	// clear screen and depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	// use our shader
	glUseProgramObjectARB(g_glslProgram);

	// Get a handle for our "MVP" uniform (identify the model-view-projection matrix in the shader)
	// [4B]
	GLuint MatrixID = glGetUniformLocation(g_glslProgram, "MVP");
	
	// Send our transformation to the currently bound shader, 
	// in the "MVP" uniform
	glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &(MVP[0]));
	
	// 1rst attribute buffer : vertices
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glVertexAttribPointer(
	  0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
	  3,                  // size, we have 3 components per vertex
	  GL_FLOAT,           // type
	  GL_FALSE,           // normalized?
	  0,                  // stride
	  (void*)0            // array buffer offset
	);

	// 2nd attribute buffer : colors [3C]
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
	glVertexAttribPointer(
	    1,                                // attribute. No particular reason for 1, but must match the layout in the shader.
	    4,                                // size
	    GL_FLOAT,                         // type
	    GL_FALSE,                         // normalized?
	    0,                                // stride
	    (void*)0                          // array buffer offset
	);
  
	// Draw the triangle !
	glDrawArrays(GL_TRIANGLES, 0, 3); // Starting from vertex 0; 3 vertices total -> 1 triangle
 
 	if (wireframe == 1)
 		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); 	
 	else
 		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); 	

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

	if(rotate)
		theta = (theta + 1) % 360;

	if(translate){
		posX += move;
		if(posX >= 2.f)
			move = -move;
		if(posX <= -2.f)
			move = -move;
	}	
		
	createMatrices();
}

void createMatrices()
{
	// Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
	m4x4f Projection = perspectiveMatrix<float>(45.0f, 4.0f / 3.0f, 0.1f, 100.0f);
	
	// Camera matrix
	m4x4f View       = lookatMatrix(V3F(0.,0.,3.), // Camera is at (4,3,3), in World Space [4E]
					V3F(0.,0.,0.), // and looks at the origin
					V3F(0.,1.,0.)); // Head is up (set to 0,-1,0 to look upside-down)
				
	// Model matrix : an identity matrix (model will be at the origin)
	m4x4f Model;
	// Model.eqIdentity();
		
	 
	float PI = 3.1414;
	float cost = cos(theta * PI / 180);
	float sint = sin(theta * PI / 180);

	m4x4f rotation (cost, -sint, 0, 0,
				   sint, cost, 0, 0,
				   0, 0, 1, 0,
				   0, 0, 0, 1);	
				
	m4x4f translation(1, 0, 0, posX,
				   	   0, 1, 0, 0,
				   	   0, 0, 1, view_distance,
				   	   0, 0, 0, 1);	
								
	Model = translation * rotation;	// Our ModelViewProjection : multiplication of our 3 matrices
	Model = Model.transpose();
	MVP = Model*View*Projection; // Remember, matrix multiplication is the other way around	
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

	///
	/// Shaders and geometry
	///
	
	//need to init Glew before anything else
#ifdef __APPLE__
	glewExperimental = GL_TRUE;
#endif
	glewInit();

	//--- Load the shaders
	// const char *vp_code=loadStringFromFile(SRC_PATH "SimpleVertexShader.vp"); // [4C];
	// const char *fp_code=loadStringFromFile(SRC_PATH "SimpleFragmentShader.fp"); // [3Da];

	const char *vp_code=loadStringFromFile(SRC_PATH "SimpleVertexShader_MVP.vp"); // [4D]
	const char *fp_code=loadStringFromFile(SRC_PATH "SimpleFragmentShader_col.fp"); // [3Db];
	g_glslProgram = createGLSLProgram(vp_code,fp_code);
	delete [](fp_code);
	delete [](vp_code);
	
	//--- Send the geometry to OpenGL
	// We need a vertex array object
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);
	// Generate 1 vertex buffer, put the resulting identifier in vertexbuffer
	glGenBuffers(1, &vertexbuffer);
	// Make the buffer active
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	// Give our vertices to OpenGL.
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);
	//same for colors... [3B]
	glGenBuffers(1, &colorbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_color_buffer_data), g_color_buffer_data, GL_STATIC_DRAW);

 	// [4A]	
 	createMatrices();

	// print a small documentation
	printf("[q]     - quit\n");

	// enter glut main loop - this *never* returns
	glutMainLoop();
	
	glDeleteBuffers(1, &vertexbuffer);
	glDeleteVertexArrays(1, &VertexArrayID);
}

/* -------------------------------------------------------- */

