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

Q1: - Read the code of "createFrameBuffer" and "mainRender". Explain how the different parts of the image are obtained.

	In createFrameBuffer() we have the buffers in which we use to store the shadow map as a texture.
	We also have a buffer to strore the depth map.
	We display the texture in the 2DQuad, which is in the bottom left part of the screen.

Q2: - Modify FragmentShaderDepth.fp to render the depth of the fragment (gl_FragCoord.z) instead of a diffuse shading

Q3: - Modify the 3DMesh shaders to render the shadow. The main steps are:
        - Send the model-view-projection-matrix of the light source to the 3DMesh vertex shader
        - In the vertex shader, project the vertex to light space (in addition to the standard projection to screen space)
        - Transmit the light coordinates of the vertex to the fragment shader
        - Send the rendered depth texture to the fragment shader
        - In the fragment shader, use the interpolated light coordinates to read the depth from the depth texture. Be careful: the light coordinates are in [-1,1][-1,1], while the texture coordinates are in [0,1][0,1]
        - Compare the depth of the vertex in light space with the depth value stored in one of the 3 channels of the texture, turn pixels black if they are occluded from the light source
        - Add a small bias in the depth test to avoid "shadow acne"
 
 This tutorial describes all these steps, but implemented a bit differently: http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-16-shadow-mapping/

Q4: - Make the light rotate around the dragon
 
Q5: - Implement the Poisson Sampling technique in your fragment shader
 
Q6: - Implement the Poisson Stratified Sampling technique in your fragment shader

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
#include "Matrix4x4.h" //class to represent matrices
#include "Vertex.h" //class to represent vertices
#include "Trackball.h" // trackball

//path of the shaders
#define SRC_PATH ""

#include <iostream>
using namespace std;

/* --------------------- Main Window ------------------- */

int          g_MainWindow; // glut Window Id
int          g_W=640;      // window width
int          g_H=480;      // window width


/* --------------------- Geometry ------------------- */

//Vertex Array Object
GLuint VertexArrayID;
// This will identify our vertex buffer for the 3D mesh
GLuint vertexbuffer3DMesh;
// This will identify our normal buffer for the 3D mesh
GLuint normalbuffer3DMesh;

// This will identify our vertex buffer for displaying the shadow map in bottom left corner (debug)
GLuint vertexbuffer2DQuad;
GLuint uvbuffer2DQuad;

// This will identify our framebuffer to render the shadow map
GLuint framebufferID;
// This will identify the texture and depthbuffer we render to
GLuint renderedTexture;
GLuint depthrenderbuffer;


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
GLuint g_glslProgram3DMesh;
GLuint g_glslProgram2DQuad;
GLuint g_glslProgramDepth;

/* -------------------------------------------------------- */

int theta = 0;
int light_rotate = 0;
int delta = 1;
void createFrameBuffer();

void mainKeyboard(unsigned char key, int x, int y) 
{
	if (key == 'q') {
		exit (0);
	} else if (key == ' ') {
		printf("spacebar pressed\n");
	} else if(key == 'r'){
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
	m4x4f View, Projection, Model, MVP, MV, DepthMVP, DepthMV;
	GLuint MatrixID;
  
	//----------------------------------------------------//
	//--      Render in texture, as seen from light     --//
	//----------------------------------------------------//

	float PI = 3.1414;
	float cost = cos(theta * PI / 180);
	float sint = sin(theta * PI / 180);

	m4x4f rotation (cost, 0, sint, 0,
					0, 1, 0, 0,
				   -sint, 0, cost, 0,				   
				   0, 0, 0, 1);

	// m4x4f rotation (cost, -sint, 0, 0,
	// 				sint, cost, 0, 0,
	// 			   0, 0, 1, 0,				   
	// 			   0, 0, 0, 1);
	
	//bind the framebuffer to render in texture
	glBindFramebuffer(GL_FRAMEBUFFER, framebufferID);
	glViewport(0,0,800,800); 
	
	// Dark blue background
	glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
	
	// clear screen and depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	
	// use our shader to render the depth map
	glUseProgramObjectARB(g_glslProgramDepth);
	
	
	//--- Camera from the light source ---//
	// Camera matrix
	View       = lookatMatrix(V3F(-3.,-3.,-2.), // light position, in World Space
					V3F(0.,0.,0.), // looks at the origin
					V3F(0.,1.,0.)); // Head is up 
	Projection = orthographicMatrix<float>(-1.,1.,-1.,1.,-6.,1.);
	Model;
	Model.eqIdentity();
	
	// Our ModelViewProjection : multiplication of our 3 matrices
	DepthMVP = Projection*View*Model * rotation; // Remember, matrix multiplication is the other way around
	DepthMVP = DepthMVP.transpose();
	DepthMV = View * Model;
	
	// Get a handle for our "depthMVP" uniform (identify the mode-view-projection matrix in the shader)
	MatrixID = glGetUniformLocation(g_glslProgramDepth, "DepthMVP");
	// Send our transformation to the currently bound shader, in the "depthMVP" uniform
	glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &(DepthMVP[0]));

	v3f lightDirection = (View * rotation).mulVector(V3F(0, 3, -2));		
	GLuint lightDirectionID = glGetUniformLocation(g_glslProgramDepth, "lightDirection");
	glUniform3fv(lightDirectionID, 1, &(lightDirection[0]));
	
	
	//--- Geometry: 3D Mesh ---//
	// 1rst attribute buffer : vertices
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer3DMesh);
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
	glBindBuffer(GL_ARRAY_BUFFER, normalbuffer3DMesh);
	glVertexAttribPointer(
	    1,                                // attribute. No particular reason for 1, but must match the layout in the shader.
	    3,                                // size, 3 coordinate per normal
	    GL_FLOAT,                         // type
	    GL_FALSE,                         // normalized? No, it will be normalized in the shader
	    0,                                // stride
	    (void*)0                          // array buffer offset
	);
  
	// Draw the triangles !
	glDrawArrays(GL_TRIANGLES, 0, g_NumIndices + 6); // Starting from vertex 0; g_NumIndices+6 vertices total 
 
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	
	
	
	//---------------------------------------------------//
	//--    Render scene on screen, seen from camera   --//
	//---------------------------------------------------//
	
	//render on screen
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0,0,g_W,g_H);
	
	// Dark blue background
	glClearColor(0.0f, 0.0f, 0.4f, 0.0f);
	
	// clear screen and depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	
	// use our shader to render the 3D mesh
	glUseProgramObjectARB(g_glslProgram3DMesh);
	
	//--- Camera ---//
	// View: get trackball transform
	View = trackballMatrix();
	// Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
	Projection = perspectiveMatrix<float>(45.0f, 4.0f / 3.0f, 0.1f, 100.0f);
	// Model matrix : an identity matrix (model will be at the origin)
	Model;
	Model.eqIdentity();
	
	// Our ModelViewProjection : multiplication of our 3 matrices
	MVP = Projection*View*Model; // Remember, matrix multiplication is the other way around
	MVP = MVP.transpose();
	
	MV = View*Model;

	// Get a handle for our "MVP" uniform (identify the model-view-projection matrix in the shader)
	MatrixID = glGetUniformLocation(g_glslProgram3DMesh, "MVP");
	// Send our transformation to the currently bound shader, in the "MVP" uniform
	glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &(MVP[0]));

	MatrixID = glGetUniformLocation(g_glslProgram3DMesh, "DepthMVP");
	// Send our transformation to the currently bound shader, in the "MV" uniform
	glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &(DepthMVP[0]));

	

	// v3f lightDirection = (View * rotation).mulVector(V3F(-3, -3, -2));		
	lightDirectionID = glGetUniformLocation(g_glslProgram3DMesh, "lightDirection");
	glUniform3fv(lightDirectionID, 1, &(lightDirection[0]));


	GLuint TextureID3d  = glGetUniformLocation(g_glslProgram3DMesh, "shadowTexture");
	// Bind our rendered texture in Texture Unit 0
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, renderedTexture);
	// Set our "myTextureSampler" sampler to user Texture Unit 0
	glUniform1i(TextureID3d, 0);
	
	//--- Geometry: 3D Mesh ---//
	// 1rst attribute buffer : vertices
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer3DMesh);
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
	glBindBuffer(GL_ARRAY_BUFFER, normalbuffer3DMesh);
	glVertexAttribPointer(
	    1,                                // attribute. No particular reason for 1, but must match the layout in the shader.
	    3,                                // size, 3 coordinate per normal
	    GL_FLOAT,                         // type
	    GL_FALSE,                         // normalized? No, it will be normalized in the shader
	    0,                                // stride
	    (void*)0                          // array buffer offset
	);
  
	// Draw the triangles !
	glDrawArrays(GL_TRIANGLES, 0, g_NumIndices + 6); // Starting from vertex 0; g_NumIndices+6 vertices total 
 
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	
	
	
	//--------------------------------//
	//--      2D Quad for debug     --//
	//--------------------------------//
	
	// Disable depth test
	glDisable(GL_DEPTH_TEST);
	
	// use our shader to render the 2D quad
	glUseProgramObjectARB(g_glslProgram2DQuad);
	
	//--- Camera ---//
	// No camera, just draw a quad over the screen in 2D space
	
	//--- Texture ---//
	// Get a handle for our "myTextureSampler" uniform
	GLuint TextureID  = glGetUniformLocation(g_glslProgram2DQuad, "myTextureSampler");
	// Bind our rendered texture in Texture Unit 0
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, renderedTexture);
	// Set our "myTextureSampler" sampler to user Texture Unit 0
	glUniform1i(TextureID, 0);
	
	//--- Geometry ---//
	// 1rst attribute buffer : vertices
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer2DQuad);
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
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer2DQuad);
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
	if(light_rotate)
		theta = (theta + delta) % 360;	

	glutPostRedisplay();
}

/* -------------------------------------------------------- */

void createFrameBuffer()
{
	// The framebuffer, which regroups 0, 1, or more textures, and 0 or 1 depth buffer.
	glGenFramebuffers(1, &framebufferID);
	glBindFramebuffer(GL_FRAMEBUFFER, framebufferID);

	// The texture we're going to render to
	glGenTextures(1, &renderedTexture);
	
	// "Bind" the newly created texture : all future texture functions will modify this texture
	glBindTexture(GL_TEXTURE_2D, renderedTexture);

    
      //----- Texture to store the rendering -------//
	// Give an empty image to OpenGL ( the last "0" means "empty" ). GL_RGB means that we want to store color
	glTexImage2D(GL_TEXTURE_2D, 0,GL_RGB, 800, 800, 0,GL_RGB, GL_UNSIGNED_BYTE, 0);

	// Poor filtering
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	// Add a depth buffer for depth test
	glGenRenderbuffers(1, &depthrenderbuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, depthrenderbuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, 800, 800);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthrenderbuffer);
	
	// Set "renderedTexture" as our COLOR attachement
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, renderedTexture, 0);
	
	// Set the list of draw buffers.
	GLenum DrawBuffers[1] = {GL_COLOR_ATTACHMENT0};
	glDrawBuffers(1, DrawBuffers); // "1" is the size of DrawBuffers
    
}

/* -------------------------------------------------------- */

void loadShaders()
{
	const char *fp_code;
	const char *vp_code;
	
	fp_code=loadStringFromFile(SRC_PATH "FragmentShader3DMesh.fp");
	vp_code=loadStringFromFile(SRC_PATH "VertexShader3DMesh.vp");
	g_glslProgram3DMesh = createGLSLProgram(vp_code,fp_code);
	delete [](fp_code);
	delete [](vp_code);
	
	fp_code=loadStringFromFile(SRC_PATH "FragmentShader2DQuad.fp");
	vp_code=loadStringFromFile(SRC_PATH "VertexShader2DQuad.vp");
	g_glslProgram2DQuad = createGLSLProgram(vp_code,fp_code);
	delete [](fp_code);
	delete [](vp_code);
	
	fp_code=loadStringFromFile(SRC_PATH "FragmentShaderDepth.fp");
	vp_code=loadStringFromFile(SRC_PATH "VertexShaderDepth.vp");
	g_glslProgramDepth = createGLSLProgram(vp_code,fp_code);
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
	GLfloat* g_vertex_buffer_data;
	GLfloat* g_normal_buffer_data;
	GLfloat* g_uv_buffer_data;
	
	// We need a vertex array object
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);
	
	//---------------------------//
	//--        3D Scene       --//
	//---------------------------//
	loadIFS("test.mesh");
	
	//----- Position
	g_vertex_buffer_data = new GLfloat[3*g_NumIndices+3*6];
	for(int i=0; i<g_NumIndices; i++){
	    g_vertex_buffer_data[i*3] = g_Verticies[g_Indices[i]].p[0];
	    g_vertex_buffer_data[i*3+1] = g_Verticies[g_Indices[i]].p[1];
	    g_vertex_buffer_data[i*3+2] = g_Verticies[g_Indices[i]].p[2];
	}
	// Add a ground plane
	g_vertex_buffer_data[3*g_NumIndices] = 0.01;
	g_vertex_buffer_data[3*g_NumIndices+1] = -1.;
	g_vertex_buffer_data[3*g_NumIndices+2] = -1.;
	
	g_vertex_buffer_data[3*g_NumIndices+3] = 0.01;
	g_vertex_buffer_data[3*g_NumIndices+4] = -1.;
	g_vertex_buffer_data[3*g_NumIndices+5] = 2.;
	
	g_vertex_buffer_data[3*g_NumIndices+6] = 0.01;
	g_vertex_buffer_data[3*g_NumIndices+7] = 2.;
	g_vertex_buffer_data[3*g_NumIndices+8] = 2.;
	
	g_vertex_buffer_data[3*g_NumIndices+9] = 0.01;
	g_vertex_buffer_data[3*g_NumIndices+10] = -1.;
	g_vertex_buffer_data[3*g_NumIndices+11] = -1.;
	
	g_vertex_buffer_data[3*g_NumIndices+12] = 0.01;
	g_vertex_buffer_data[3*g_NumIndices+13] = 2.;
	g_vertex_buffer_data[3*g_NumIndices+14] = 2.;
	
	g_vertex_buffer_data[3*g_NumIndices+15] = 0.01;
	g_vertex_buffer_data[3*g_NumIndices+16] = 2.;
	g_vertex_buffer_data[3*g_NumIndices+17] = -1.;
	
	
	//----- Normals
	g_normal_buffer_data = new GLfloat[3*g_NumIndices+3*6];
	for(int i=0; i<g_NumIndices; i++){
	    g_normal_buffer_data[i*3] = g_Verticies[g_Indices[i]].n[0];
	    g_normal_buffer_data[i*3+1] = g_Verticies[g_Indices[i]].n[1];
	    g_normal_buffer_data[i*3+2] = g_Verticies[g_Indices[i]].n[2];
	}
	// Add a ground plane
	g_normal_buffer_data[3*g_NumIndices] = 1.;
	g_normal_buffer_data[3*g_NumIndices+1] = 0.;
	g_normal_buffer_data[3*g_NumIndices+2] = 0.;
	
	g_normal_buffer_data[3*g_NumIndices+3] = 1.;
	g_normal_buffer_data[3*g_NumIndices+4] = 0.;
	g_normal_buffer_data[3*g_NumIndices+5] = 0.;
	
	g_normal_buffer_data[3*g_NumIndices+6] = 1.;
	g_normal_buffer_data[3*g_NumIndices+7] = 0.;
	g_normal_buffer_data[3*g_NumIndices+8] = 0.;
	
	g_normal_buffer_data[3*g_NumIndices+9] = 1.;
	g_normal_buffer_data[3*g_NumIndices+10] = 0.;
	g_normal_buffer_data[3*g_NumIndices+11] = 0.;
	
	g_normal_buffer_data[3*g_NumIndices+12] = 1.;
	g_normal_buffer_data[3*g_NumIndices+13] = 0.;
	g_normal_buffer_data[3*g_NumIndices+14] = 0.;
	
	g_normal_buffer_data[3*g_NumIndices+15] = 1.;
	g_normal_buffer_data[3*g_NumIndices+16] = 0.;
	g_normal_buffer_data[3*g_NumIndices+17] = 0.;
	
	
	//--- Send the geometry to OpenGL
	// Generate 1 vertex buffer, put the resulting identifier in vertexbuffer3DMesh
	glGenBuffers(1, &vertexbuffer3DMesh);
	// Make the buffer active
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer3DMesh);
	// Give our vertices to OpenGL.
	glBufferData(GL_ARRAY_BUFFER, (3*g_NumIndices+3*6)*sizeof(GLfloat), g_vertex_buffer_data, GL_STATIC_DRAW);
	//same for normals...
	glGenBuffers(1, &normalbuffer3DMesh);
	glBindBuffer(GL_ARRAY_BUFFER, normalbuffer3DMesh);
	glBufferData(GL_ARRAY_BUFFER, (3*g_NumIndices+3*6)*sizeof(GLfloat), g_normal_buffer_data, GL_STATIC_DRAW);
	
	
	//clean up
	delete [] g_vertex_buffer_data;
	delete [] g_normal_buffer_data;
	
	
	//--------------------------------//
	//--   Quad for debug display   --//
	//--------------------------------//
	
	g_vertex_buffer_data = new GLfloat[12];
	//bottom right triangle. Note: the OpenGL screen covers [-1;1] x [-1;1]
	g_vertex_buffer_data[0] = -1.0f; g_vertex_buffer_data[1] = -1.0f; 
	g_vertex_buffer_data[2] = -0.5f; g_vertex_buffer_data[3] = -1.0f; 
	g_vertex_buffer_data[4] = -0.5f; g_vertex_buffer_data[5] = -0.5f; 
	//top left triangle
	g_vertex_buffer_data[6] = -1.0f; g_vertex_buffer_data[7] = -1.0f; 
	g_vertex_buffer_data[8] = -0.5f; g_vertex_buffer_data[9] = -0.5f; 
	g_vertex_buffer_data[10] = -1.0f; g_vertex_buffer_data[11] = -0.5f; 
	
	// UVs
	g_uv_buffer_data = new GLfloat[12];
	//bottom right triangle. Set the coordinates so that the quad covers [0;1] x [0;1]
	g_uv_buffer_data[0] = 0.0f; g_uv_buffer_data[1] = 0.0f; 
	g_uv_buffer_data[2] = 1.0f; g_uv_buffer_data[3] = 0.0f; 
	g_uv_buffer_data[4] = 1.0f; g_uv_buffer_data[5] = 1.0f; 
	//top left triangle
	g_uv_buffer_data[6] = 0.0f; g_uv_buffer_data[7] = 0.0f; 
	g_uv_buffer_data[8] = 1.0f; g_uv_buffer_data[9] = 1.0f; 
	g_uv_buffer_data[10] = 0.0f; g_uv_buffer_data[11] = 1.0f; 
	
	// Generate 1 vertex buffer, put the resulting identifier in vertexbuffer2DQuad
	glGenBuffers(1, &vertexbuffer2DQuad);
	// Make the buffer active
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer2DQuad);
	// Give our vertices to OpenGL.
	glBufferData(GL_ARRAY_BUFFER, 12*sizeof(GLfloat), g_vertex_buffer_data, GL_STATIC_DRAW);
	//same for UVs...
	glGenBuffers(1, &uvbuffer2DQuad);
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer2DQuad);
	glBufferData(GL_ARRAY_BUFFER, 12*sizeof(GLfloat), g_uv_buffer_data, GL_STATIC_DRAW);
	
	
	//clean up
	delete [] g_vertex_buffer_data;
	delete [] g_uv_buffer_data;
}


/* -------------------------------------------------------- */

void cleanUp()
{
	glDeleteBuffers(1, &vertexbuffer3DMesh);
	glDeleteBuffers(1, &normalbuffer3DMesh);
	glDeleteVertexArrays(1, &VertexArrayID);
	glDeleteFramebuffers(1, &framebufferID);
	glDeleteTextures(1, &renderedTexture);
	glDeleteRenderbuffers(1, &depthrenderbuffer);
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
	g_MainWindow=glutCreateWindow("TP5");
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
	
	//Create the framebuffer to render in texture
	createFrameBuffer();
	
	// Trackball init (controls the camera)
	trackballInit(g_W,g_H);
	trackballTranslation() = V3F(0.354f,-0.313f,-2.478f);
	trackballRotation()    = quatf(-0.0199f,-0.201f,0.679f,0.705f);

	// print a small documentation
	printf("[q]     - quit\n");

	// enter glut main loop - this *never* returns
	glutMainLoop();
	
	cleanUp();
}

/* -------------------------------------------------------- */

