#define GLEW_STATIC
#include <GL/glew.h>


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <GLFW/glfw3.h>
#include "nanosvg.h"
#include "tesselator.h"

#include <thread>


void* stdAlloc(void* userData, unsigned int size)
{
	int* allocated = ( int*)userData;
	TESS_NOTUSED(userData);
	*allocated += (int)size;
	return malloc(size);
}

void stdFree(void* userData, void* ptr)
{
	TESS_NOTUSED(userData);
	free(ptr);
}

struct MemPool
{
	unsigned char* buf;
	unsigned int cap;
	unsigned int size;
};

void* poolAlloc( void* userData, unsigned int size )
{
	struct MemPool* pool = (struct MemPool*)userData;
	size = (size+0x7) & ~0x7;
	if (pool->size + size < pool->cap)
	{
		unsigned char* ptr = pool->buf + pool->size;
		pool->size += size;
		return ptr;
	}
	printf("out of mem: %d < %d!\n", pool->size + size, pool->cap);
	return 0;
}

void poolFree( void* userData, void* ptr )
{
	// empty
	TESS_NOTUSED(userData);
	TESS_NOTUSED(ptr);
}


void initModernOpenGL(float* verts, int nverts, int* elements, int nelements )
{
  GLuint vao;
  glGenVertexArrays(1, &vao);
  
  glBindVertexArray(vao);
  
  GLuint vbo;
  glGenBuffers(1, &vbo); // Generate 1 buffer
  
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
			
  glBufferData(GL_ARRAY_BUFFER, sizeof(float)*nverts, verts, GL_STATIC_DRAW);


  GLuint ebo;
  glGenBuffers(1, &ebo);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER,
	       sizeof(int)*nelements, elements, GL_STATIC_DRAW);


  
  const char* vertexSource = R"glsl(
	#version 150 core

	in vec2 position;

	void main()
	{
		gl_Position = vec4(position, 0.0, 1.0);
	}
	)glsl";
			
  GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertexShader, 1, &vertexSource, NULL);
			
  glCompileShader(vertexShader);
			
  GLint status;
  glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &status);
			
  char buffer[512];
  glGetShaderInfoLog(vertexShader, 512, NULL, buffer);
			
  if (status != GL_TRUE)
    {
      printf("%s\n", buffer);
    }
			
  const char* fragmentSource = R"glsl(
	#version 150 core

		out vec4 outColor;

	void main()
	{
		outColor = vec4(1.0, 1.0, 1.0, 1.0);
	}
	)glsl";
			
  GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
  glCompileShader(fragmentShader);
			
			
  glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &status);
			
  glGetShaderInfoLog(fragmentShader, 512, NULL, buffer);
			
  if (status != GL_TRUE)
    {
      printf("%s\n", buffer);
    }

  GLuint shaderProgram = glCreateProgram();
  glAttachShader(shaderProgram, vertexShader);
  glAttachShader(shaderProgram, fragmentShader);
			
  glBindFragDataLocation(shaderProgram, 0, "outColor");
			
  glLinkProgram(shaderProgram);

  glUseProgram(shaderProgram);
			
  GLint posAttrib = glGetAttribLocation(shaderProgram, "position");
			
  glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 0, 0);
			
  glEnableVertexAttribArray(posAttrib);

}


// Undefine this to see non-interactive heap allocator version.
//#define USE_POOL 1


int run = 1;

static void key(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	TESS_NOTUSED(scancode);
	TESS_NOTUSED(mods);
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
	if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
		run = !run;
}

int main(int argc, char *argv[])
{
	GLFWwindow* window;
	const GLFWvidmode* mode;
	int width,height,i,j;
	struct SVGPath* bg;
	//struct SVGPath* fg;
	struct SVGPath* it;
	float bounds[4],view[4],cx,cy,w,offx,offy;
	float t = 0.0f, pt = 0.0f;
	TESSalloc ma;
	TESStesselator* tess = 0;
	const int nvp = 3;
	unsigned char* vflags = 0;
	int nvflags = 0;
#ifdef USE_POOL
	struct MemPool pool;
	unsigned char mem[1024*1024];
#else
	int allocated = 0;
#endif
	TESS_NOTUSED(argc);
	TESS_NOTUSED(argv);

	if (!glfwInit()) {
		printf("Failed to init GLFW.");
		return -1;
	}

	printf("loading...\n");
	// Load assets
	bg = svgParseFromFile("./Bin/bg.svg");
	if (!bg) return -1;

	//	fg = svgParseFromFile("./Bin/fg.svg");
	//if (!fg) return -1;

	printf("go...\n");
	
	// Flip y
	for (it = bg; it != NULL; it = it->next)
		for (i = 0; i < it->npts; ++i)
			it->pts[i*2+1] = -it->pts[i*2+1];
	/*
	for (it = fg; it != NULL; it = it->next)
		for (i = 0; i < it->npts; ++i)
			it->pts[i*2+1] = -it->pts[i*2+1];
	*/

	// Find FG bounds and center.
	/*
	bounds[0] = bounds[2] = fg->pts[0];
	bounds[1] = bounds[3] = fg->pts[1];
	for (it = fg; it != NULL; it = it->next)
	{
		for (i = 0; i < it->npts; ++i)
		{
			const float x = it->pts[i*2];
			const float y = it->pts[i*2+1];
			if (x < bounds[0]) bounds[0] = x;
			if (y < bounds[1]) bounds[1] = y;
			if (x > bounds[2]) bounds[2] = x;
			if (y > bounds[3]) bounds[3] = y;
		}
	}
	cx = (bounds[0]+bounds[2])/2;
	cy = (bounds[1]+bounds[3])/2;
	for (it = fg; it != NULL; it = it->next)
	{
		for (i = 0; i < it->npts; ++i)
		{
			it->pts[i*2] -= cx;
			it->pts[i*2+1] -= cy;
		}
	}
	*/
			
	// Find BG bounds.
	/*
	bounds[0] = bounds[2] = bg->pts[0];
	bounds[1] = bounds[3] = bg->pts[1];
	for (it = bg; it != NULL; it = it->next)
	{
		for (i = 0; i < it->npts; ++i)
		{
			const float x = it->pts[i*2];
			const float y = it->pts[i*2+1];
			if (x < bounds[0]) bounds[0] = x;
			if (y < bounds[1]) bounds[1] = y;
			if (x > bounds[2]) bounds[2] = x;
			if (y > bounds[3]) bounds[3] = y;
		}
	}
	*/	

	memset(&ma, 0, sizeof(ma));
	ma.memalloc = stdAlloc;
	ma.memfree = stdFree;
	ma.userData = (void*)&allocated;
	ma.extraVertices = 256; // realloc not provided, allow 256 extra vertices.

	tess = tessNewTess(&ma);
	if (!tess)
		return -1;

	// Offset the foreground shape to center of the bg.
	offx = (bounds[2]+bounds[0])/2;
	offy = (bounds[3]+bounds[1])/2;

	/*
	for (it = fg; it != NULL; it = it->next)
	{
		for (i = 0; i < it->npts; ++i)
		{
			it->pts[i*2] += offx;
			it->pts[i*2+1] += offy;
		}
	}
	*/
	
	// Add contours.
	for (it = bg; it != NULL; it = it->next)
		tessAddContour(tess, 2, it->pts, sizeof(float)*2, it->npts);

	/*
	for (it = fg; it != NULL; it = it->next)
		tessAddContour(tess, 2, it->pts, sizeof(float)*2, it->npts);
	*/
	if (!tessTesselate(tess, TESS_WINDING_POSITIVE, TESS_POLYGONS, nvp, 2, 0))
		return -1;
	printf("Memory used: %.1f kB\n", allocated/1024.0f);
	
	
	//mode = glfwGetVideoMode(glfwGetPrimaryMonitor());

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	
	width = mode->width - 40;
	height = mode->height - 80;
	window = glfwCreateWindow(width, height, "Libtess2 Demo", NULL, NULL);
	if (!window) {
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);

	glewExperimental = GL_TRUE;
	glewInit();


	glfwSetKeyCallback(window, key);
	glfwMakeContextCurrent(window);

	// Adjust bounds so that we get nice view of the bg.
	/*
	cx = (bounds[0]+bounds[2])/2;
	cy = (bounds[3]+bounds[1])/2;
	w = (bounds[2]-bounds[0])/2;
	view[0] = cx - w*1.2f;
	view[2] = cx + w*1.2f;
	view[1] = cy - w*1.2f*(float)height/(float)width;
	view[3] = cy + w*1.2f*(float)height/(float)width;

	printf("v1:%f,v2:%f,v3:%f,v4:%f",view[0],view[1],view[2],view[3]);
	*/
	

	glfwSetTime(0);

         float* verts = tessGetVertices(tess);
	 int* vinds = tessGetVertexIndices(tess);
	 int* elems = tessGetElements(tess);
	 int nverts = tessGetVertexCount(tess);
	 int nelems = tessGetElementCount(tess);
				    
        initModernOpenGL( verts,  nverts, elems,  nelems );


	while (!glfwWindowShouldClose(window))
	{
		float ct = (float)glfwGetTime();
		if (run) t += ct - pt;
		pt = ct;
		
		// Update and render
		glViewport(0, 0, width, height);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glDisable(GL_TEXTURE_2D);
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(-1.f,1.f,-1.f,1.f,-1,1);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glDisable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);


		// Draw tesselated pieces.
		if (tess)
		{

			int polySize = 3;
			int vertexSize = 2;

			  
			// Draw polygons.
			glColor4ub(255,255,255,128);


			const int nelems2 = tessGetElementCount(tess);
			const TESSindex* elems2 = tessGetElements(tess);

			printf("num elems: %d \n",nelems2);
			for (int i = 0; i<nelems2; i++)
			  {
			    for (int j = 0; j < polySize; j++) {
			      printf("%d,",(&elems2[i * polySize])[j]);
			    }
			    printf("\n");
			  };
			printf("\n");			
			
			for (int i = 0; i < nelems2; i++) {
			  const TESSindex* poly = &elems2[i * polySize];
			  glBegin(GL_POLYGON);
			  for (int j = 0; j < polySize; j++) {
			    if (poly[j] == TESS_UNDEF) break;
			    glVertex2fv(&verts[poly[j]*vertexSize]);
			  }
			  glEnd();
			}

			/*
			for (i = 0; i < nelems; ++i)
			{
				const int* p = &elems[i*nvp];
				glBegin(GL_TRIANGLE_FAN);
				for (j = 0; j < nvp && p[j] != TESS_UNDEF; ++j)
					glVertex2f(verts[p[j]*2], verts[p[j]*2+1]);
				glEnd();
			}
			
			glColor4ub(0,0,0,16);
			for (i = 0; i < nelems; ++i)
			{
				const int* p = &elems[i*nvp];
				glBegin(GL_LINE_LOOP);
				for (j = 0; j < nvp && p[j] != TESS_UNDEF; ++j)
					glVertex2f(verts[p[j]*2], verts[p[j]*2+1]);
				glEnd();
			}
			
			glColor4ub(0,0,0,128);
			glPointSize(3.0f);
			glBegin(GL_POINTS);
			for (i = 0; i < nverts; ++i)
			{
				if (vflags && vflags[vinds[i]])
					glColor4ub(255,0,0,192);
				else
					glColor4ub(0,0,0,128);
				glVertex2f(verts[i*2], verts[i*2+1]);
			}
			glEnd();
			glPointSize(1.0f);

			*/
		}
		
		glEnable(GL_DEPTH_TEST);
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	
	if (tess) tessDeleteTess(tess);
	
	if (vflags)
		free(vflags);
	
	svgDelete(bg);	
	//svgDelete(fg);	

	glfwTerminate();
	return 0;
}
