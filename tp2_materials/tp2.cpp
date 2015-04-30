/* -------------------------------------------------------- */
/* -------------------------------------------------------- */
/*

TP OpenGL - Image Synthesis course

George Drettakis, Adrien Bousseau

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

* Q1:   The 'loadIFS' function loads an Indexed Face Set (see ifs.pdf) in the following variables:

    unsigned int    g_NumVerticies  = 0;      // number of verticies
    t_point        *g_Verticies     = NULL;   // verticies
    unsigned int    g_NumIndices    = 0;      // number of indices (= 3 * number of triangles)
    unsigned short *g_Indices       = NULL;   // indices, 3 per triangles

  - Add code to send the IFS data to OpenGL (markers [Q1a] and [Q1b])
  - Rotate the model with the mouse. Uncomment [Q1c] and test the difference. What is happening?
    Answer: ....
    If we comment the depth_test, fragments are rendered even if they are behind soem fragments. As a result
    we 'see-through' the object. Commenting this line disables the working of the z-buffer.

  - What are the colors over the object? (see SimpleFragmentShader.fp)
    Answer: ....
	The colours are a funcion of the direction of the normal of each fragment.
	The colour indicates the orientation of the object with respect to the camera.

  - In SimpleVertexShader.fp, uncomment [Q1d] and see how it changes the colors of the object. What is the difference?
    Answer: ....
    The coulrs now stay depend on the object's shape and not its orientation in the view space.
    The colours do not change if we change the object's postion.

* Q2. - Program a Phong model in the fragment shader
   
   Reminder:
      The Phong model computes the final color as:
         (-L.N)*diffuse_color + ( (R.L)^specular_exponent ) * specular_color
       where L is the incoming light direction, N the surface normals and R the
       reflected *view* direction. L,N and R are normalized (unit length).
       Use the GLSL function `reflect' to compute R in the fragment shader.

* Q3.

 - Provide the specular exponent as a parameter of your shader. 
 - Use the keyboard callback function to increase ('+') or decrease ('-') the exponent. Use values in [1,2,4,8,16,32,64,128]
  
* Q5.
  - Modify the Phong shader to render the object in a cartoon style. To do so, you need to quantize the diffuse term to (0.5,1)
  and the specular term to (0,1)
  
  - Draw the contours of the object in black. Note: the contour lies where the surface normal is perpendicular to the view direction

  Press 'c' to enable cartoon mode.
  
* Q4.
  - Animate the light to orbit along a circle.
  
  - Fix the light to the camera position.

  Press 'r' to stop the light orbit. 

*/


#ifdef _WIN32
#include <windows.h>        // windows API header (required by gl.h)
#endif

#include "glew/include/GL/glew.h"	// Support for shaders

#ifdef __APPLE__

// #include <gl3.h>          // OpenGL header
// #include <gl3ext.h>       // OpenGL header
#include <GLUT/glut.h>         // OpenGL Utility Toolkit header

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
#include "Matrix4x4.h" //class to represent matrices
#include "Vertex.h" //class to represent vertices
#include "Trackball.h" // trackball

//path of the shaders
#define SRC_PATH	""

#include <iostream>
using namespace std;


/* --------------------- Main Window ------------------- */

int          g_MainWindow; // glut Window Id
int          g_W=640;      // window width
int          g_H=480;      // window width


/* --------------------- Geometry ------------------- */

//Vertex Array Object
GLuint VertexArrayID;
// This will identify our vertex buffer
GLuint vertexbuffer;
// This will identify our normal buffer
GLuint normalbuffer;

GLuint uvbuffer;
GLuint texture;

//---- IFS representation ---
typedef struct s_point
{
  float p[3]; //position
  float n[3]; //normal
  float uv[2]; //texture coordinate
} t_point;

t_point        *g_Verticies     = NULL; //list of vertices
unsigned short *g_Indices       = NULL; //list of faces, 3 indices per face
unsigned int    g_NumVerticies  = 0; //number of vertices
unsigned int    g_NumIndices    = 0; //number of indices, i.e. 3 times the number of faces


/*---------------------- Shaders -------------------*/
GLuint g_glslProgram;


/* -------------------------------------------------------- */

int theta = 0;
int delta = 1;
int light_rotate = 0;

int spec_value[] = {1,2,4,8,16,32,64,128};
int spec_index = 0;
int specularE = 1;

int cartoon_mode = 0;

void mainKeyboard(unsigned char key, int x, int y) 
{
	if (key == 'q') {
		exit (0);

	} else if (key == ' ') {
		printf("spacebar pressed\n");
	}
	else if (key == '+'){
		spec_index = (spec_index + 1) % 8;
		specularE = spec_value[spec_index];
	}
	else if (key == '-'){
		spec_index--;
		if(spec_index == -1)
			spec_index = 7;

		specularE = spec_value[spec_index];
	}
	else if (key == 'c'){
		cartoon_mode = !cartoon_mode;
	}
	else if(key == 'r'){
		light_rotate = !light_rotate;
	}

	printf("key '%c' pressed\n",key);
}

/* -------------------------------------------------------- */

void mainMouse(int btn, int state, int x, int y) 
{
	if (state == GLUT_DOWN) {
		trackballButtonPressed(btn,x,y);
	} else if (state == GLUT_UP) {
		trackballButtonReleased(btn);
	}
}

/* -------------------------------------------------------- */

void mainMotion(int x,int y)
{
	trackballUpdate(x,y);
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
	glClearColor(0.4f, 0.4f, 0.4f, 0.0f);
	
	// clear screen and depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	// Enable depth test
	glEnable(GL_DEPTH_TEST); //[Q1c]
	
	// use our shader
	glUseProgramObjectARB(g_glslProgram);
	

	
	//--- Camera ---//
	// View: get trackball transform
	m4x4f View = trackballMatrix();
	// Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
	m4x4f Projection = perspectiveMatrix<float>(45.0f, 4.0f / 3.0f, 0.1f, 100.0f);
	// Model matrix : an identity matrix (model will be at the origin)
	m4x4f Model;
	Model.eqIdentity();
	
	// Our ModelViewProjection : multiplication of our 3 matrices
	m4x4f MVP = Projection*View*Model; // Remember, matrix multiplication is the other way around
	MVP = MVP.transpose();
	
	// ModelView
	m4x4f MV = View*Model;

	//chaging the light direction
	float PI = 3.1414;
	float cost = cos(theta * PI / 180);
	float sint = sin(theta * PI / 180);

	m4x4f rotation (cost, 0, -sint, 0,
					0, 1, 0, 0,
				   -sint, 0, cost, 0,				   
				   0, 0, 0, 1);
	

	MV = MV.transpose();
	
	// Get a handle for our "MVP" uniform (identify the model-view-projection matrix in the shader)
	GLuint MatrixID = glGetUniformLocation(g_glslProgram, "MVP");
	// Send our transformation to the currently bound shader, in the "MVP" uniform
	glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &(MVP[0]));
	
	// Get a handle for our "MV" uniform (identify the view matrix in the shader)
	GLuint MatrixViewID = glGetUniformLocation(g_glslProgram, "MV");
	// Send our transformation to the currently bound shader, in the "MV" uniform
	glUniformMatrix4fv(MatrixViewID, 1, GL_FALSE, &(MV[0]));
		

	// Light direction, expressed in *view* space
	v3f lightDirection = (View * rotation).mulVector(V3F(0, 0, -1));		
	GLuint lightDirectionID = glGetUniformLocation(g_glslProgram, "lightDirection");
	glUniform3fv(lightDirectionID, 1, &(lightDirection[0]));

	//specular exponent
	GLuint specularExponentID = glGetUniformLocation(g_glslProgram, "specularE");
	glUniform1i(specularExponentID, specularE);	

	GLuint cartoon_modeID = glGetUniformLocation(g_glslProgram, "cartoon_mode");
	glUniform1i(cartoon_modeID, cartoon_mode);


	//--- Texturing, [Q2b]
	// Get a handle for our "myTextureSampler" uniform
	GLuint TextureID  = glGetUniformLocation(g_glslProgram, "myTextureSampler");
	// Bind our texture in Texture Unit 0
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);
	// Set our "myTextureSampler" sampler to user Texture Unit 0
	glUniform1i(TextureID, 0); 


 
	//--- Geometry ---//
	// 1rst attribute buffer : vertices
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glVertexAttribPointer(
	  0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
	  3,                  // size, 3 coordinates per vertex (x,y,z)
	  GL_FLOAT,           // type
	  GL_FALSE,           // normalized?
	  0,                  // stride
	  (void*)0            // array buffer offset
	);
	
	// 2nd attribute buffer : normals
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
	glVertexAttribPointer(
	    1,                                // attribute 1. No particular reason for 1, but must match the layout in the shader.
	    3,                                // size, 3 coordinate per normal
	    GL_FLOAT,                         // type
	    GL_FALSE,                         // normalized? No, it will be normalized in the shader
	    0,                                // stride
	    (void*)0                          // array buffer offset
	);
  
	//3rd attribute : texture
  	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
	glVertexAttribPointer(
	    2,                                // attribute 2. No particular reason for 1, but must match the layout in the shader.
	    2,                                // size, 2 coordinate per uv_texture
	    GL_FLOAT,                         // type
	    GL_FALSE,                         // normalized? No, it will be normalized in the shader
	    0,                                // stride
	    (void*)0                          // array buffer offset
	);
	// Draw the triangle !
	glDrawArrays(GL_TRIANGLES, 0, 3*g_NumIndices); // Starting from vertex 0; 3 vertices total -> 1 triangle. [Q1b] Update to draw all vertices of the loaded mesh
	 	 	
	// glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); 

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);
	
	// swap - this call exchanges the back and front buffer
	// swap is synchronized on the screen vertical sync
	glutSwapBuffers();
}

/* -------------------------------------------------------- */

void idle( void )
{
	// whenever the application has free time, ask for a screen refresh	
	//rotate light
	if(light_rotate)
		theta = (theta + delta) % 360;	

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

// Load an IFS model
void loadIFS(const char *filename)
{
  // open file
  FILE *f=fopen(filename,"rb");
  if (f == NULL) {
    // error?
    cerr << "[loadIFS] Cannot load " << filename << endl;
  }
  // read vertices
  fread(&g_NumVerticies,sizeof(unsigned int),1,f);
  g_Verticies = new t_point[g_NumVerticies];
  fread(g_Verticies,sizeof(t_point),g_NumVerticies,f);
  // read indices
  fread(&g_NumIndices,sizeof(unsigned int),1,f);
  g_Indices = new unsigned short[g_NumIndices];
  fread(g_Indices,sizeof(unsigned short),g_NumIndices,f);
  // close file
  fclose(f);
  // print mesh info
  cerr << g_NumVerticies << " points " << g_NumIndices/3 << " triangles" << endl;
  // done.
}

/* -------------------------------------------------------- */

void createGeometry()
{
	loadIFS("test.mesh");
	
	//----- [Q1a] : the following piece of code sends a simple triangle to OpenGL. Modify the code to send the IFS mesh instead.
	// Your code should loop over all faces of the mesh, and create one triangle per face by sending the 3 coordinates of each vertex.
  
	// Create a simple triangle. A triangle contains 3 vertices, each vertex has 3 coordinates
	// Position


	GLfloat* g_vertex_buffer_data;
	// g_vertex_buffer_data = new GLfloat[3*3];
	g_vertex_buffer_data = new GLfloat[3*g_NumIndices];

	GLfloat* g_normal_buffer_data;
	// g_normal_buffer_data = new GLfloat[3*3];
	g_normal_buffer_data = new GLfloat[3*g_NumIndices];

	GLfloat* g_uv_buffer_data;
	g_uv_buffer_data = new GLfloat[2*g_NumIndices];

	for(int i = 0 ;i < g_NumIndices; i++){
		g_vertex_buffer_data[i*3 + 0] = g_Verticies[g_Indices[i]].p[0];
		g_vertex_buffer_data[i*3 + 1] = g_Verticies[g_Indices[i]].p[1];
		g_vertex_buffer_data[i*3 + 2] = g_Verticies[g_Indices[i]].p[2];
		
		// Normals		
		g_normal_buffer_data[i*3 + 0] = g_Verticies[g_Indices[i]].n[0];
		g_normal_buffer_data[i*3 + 1] = g_Verticies[g_Indices[i]].n[1];
		g_normal_buffer_data[i*3 + 2] = g_Verticies[g_Indices[i]].n[2];

		//uv texture data
		g_uv_buffer_data[i*2 + 0] = g_Verticies[g_Indices[i]].uv[0];
		g_uv_buffer_data[i*2 + 1] = g_Verticies[g_Indices[i]].uv[1];
	}

	//--- Send the geometry to OpenGL
	// We need a vertex array object
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);
	// Generate 1 vertex buffer, put the resulting identifier in vertexbuffer
	glGenBuffers(1, &vertexbuffer);
	// Make the buffer active
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	// Give our vertices to OpenGL.
	glBufferData(GL_ARRAY_BUFFER, 3*g_NumIndices*sizeof(GLfloat), g_vertex_buffer_data, GL_STATIC_DRAW); // [Q1a] Don't forget to send all your vertices
	//same for colors...
	glGenBuffers(1, &normalbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
	glBufferData(GL_ARRAY_BUFFER, 3*g_NumIndices*sizeof(GLfloat), g_normal_buffer_data, GL_STATIC_DRAW); // [Q1a] Don't forget to send all your normals
	
	glGenBuffers(1, &uvbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
	glBufferData(GL_ARRAY_BUFFER, 2*g_NumIndices*sizeof(GLfloat), g_uv_buffer_data, GL_STATIC_DRAW);
	
	//clean up
	delete [] g_vertex_buffer_data;
	delete [] g_normal_buffer_data;
	delete [] g_uv_buffer_data;
}

/* -------------------------------------------------------- */

void cleanUp()
{
	glDeleteBuffers(1, &vertexbuffer);
	glDeleteBuffers(1, &normalbuffer);
	glDeleteBuffers(1, &uvbuffer);
	glDeleteVertexArrays(1, &VertexArrayID);
}

/* -------------------------------------------------------- */

GLuint createCheckerBoardTexture()
{
  static int res=16;
  GLuint id = 0;
  glGenTextures(1,&id);
  glBindTexture(GL_TEXTURE_2D,id);
  unsigned char *data = new unsigned char[res*res*4];
  for (int j = 0;j < res;j++) {
    for (int i = 0;i < res;i++) {
      data[(i + j*res) * 4 + 0] = ((i + j) & 1) ? 255 : 0;
      data[(i + j*res) * 4 + 1] = ((i + j) & 1) ? 255 : 0;
      data[(i + j*res) * 4 + 2] = ((i + j) & 1) ? 255 : 0;
      data[(i + j*res) * 4 + 3] = 255;

      cout << "i: "<< j << " j: " << i << "data: " << (i + j*res)*4 << "dsad:" << ((i + j) & 1) <<endl;
    }
    cout << endl;
  }

  glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,res,res,0,GL_RGBA,GL_UNSIGNED_BYTE,data);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST); //[Q3c]
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP);
  delete [](data);
  return (id);
}

GLuint makeStripeImage(void)
{
	static int res=16;
	GLuint id = 0;
	glGenTextures(1,&id);
  	glBindTexture(GL_TEXTURE_2D,id);
	unsigned char *data = new unsigned char[res*res*4];
	for (int j = 0; j < res*res; j++) {
		data[4*j] =  j % 2 == 0 ? 255 : 0;
		data[4*j+1] =  j % 2 == 0 ? 255 : 0;
		data[4*j+2] =  j % 2 == 0 ? 255 : 0;
		data[4*j+3] =  255;
	}

	glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,res,res,0,GL_RGBA,GL_UNSIGNED_BYTE,data);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST); //[Q3c]
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP);
	delete [](data);
	return (id);	
}


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
	g_MainWindow=glutCreateWindow("TP3");
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
	/// Shaders, geometry and camera
	///
	
	//need to init Glew before anything else
#ifdef __APPLE__
	glewExperimental = GL_TRUE;
#endif
	glewInit();
	
	loadShaders();
	
	//Send the geometry to OpenGL
	createGeometry();

	// Trackball init (controls the camera)
	trackballInit(g_W,g_H);
	trackballTranslation() = V3F(0.f,0.f,-3.f);


	texture = createCheckerBoardTexture(); //[Q2a]
	// texture = makeStripeImage(); 

	// print a small documentation
	printf("[q]     - quit\n");

	// enter glut main loop - this *never* returns
	glutMainLoop();
	
	cleanUp();
}
/* -------------------------------------------------------- */

