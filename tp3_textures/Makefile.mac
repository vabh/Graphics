#Standard include and library path for GL/ and libGL, libGLU, libglut, update if needed 
INCLUDEDIR = -I/usr/include/ -I/System/Library/Frameworks/OpenGL.framework/Headers/ -I/System/Library/Frameworks/GLUT.framework/Headers/ -I/System/Library/Frameworks/GLEW.framework/Headers/ -Wno-deprecated-declarations
LIBDIR =  -framework OpenGL -framework GLUT 
LIBDIR =  -framework OpenGL -framework GLUT 

tp3: tp3.o glew.o glsl.o Vertex.o Trackball.o
	g++ $(LIBDIR) tp3.o glew.o glsl.o Vertex.o Trackball.o -o tp3

tp3.o: tp3.cpp
	g++ -c $(INCLUDEDIR) tp3.cpp

glew.o: glew/src/glew.c
	g++ -c $(INCLUDEDIR) -I glew/include/ glew/src/glew.c

glsl.o: glsl.cpp
	g++ -c $(INCLUDEDIR) -I glew/include/ glsl.cpp

Vertex.o: Vertex.cpp
	g++ -c $(INCLUDEDIR) Vertex.cpp

Trackball.o: Trackball.cpp
	g++ -c $(INCLUDEDIR) Trackball.cpp
