#Standard include and library path for GL/ and libGL, libGLU, libglut, update if needed 
INCLUDEDIR = -I/usr/include/ -I/System/Library/Frameworks/OpenGL.framework/Headers/ -I/System/Library/Frameworks/GLUT.framework/Headers/ -I/System/Library/Frameworks/GLEW.framework/Headers/ -Wno-deprecated-declarations
LIBDIR =  -framework OpenGL -framework GLUT 
LIBDIR =  -framework OpenGL -framework GLUT 

tp4: tp4.o glew.o glsl.o TextureLoader.o
	g++ $(LIBDIR) tp4.o glew.o glsl.o TextureLoader.o -o tp4 

tp4.o: tp4.cpp
	g++ -c $(INCLUDEDIR) tp4.cpp

glew.o: glew/src/glew.c
	g++ -c $(INCLUDEDIR) -I glew/include/ glew/src/glew.c

glsl.o: glsl.cpp
	g++ -c $(INCLUDEDIR) -I glew/include/ glsl.cpp

TextureLoader.o: TextureLoader.h TextureLoader.cpp
	g++ -c $(INCLUDEDIR) TextureLoader.cpp