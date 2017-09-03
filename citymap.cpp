#define GLEW_STATIC
#include <GL/glew.h>


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <GLFW/glfw3.h>
//#include "nanosvg.h"
#include "tesselator.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <thread>

#include "json.hpp"
#include <fstream>
#include <map>
#include "camera.hpp"
#include "map.hpp"
#include "graphics.hpp"

Camera g_camera;

GLint uniTrans;

GLFWwindow *window = NULL;
bool rightMouseDown;
glm::vec2 lastp;

GLuint circleProgram;

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




static void sScrollCallback(GLFWwindow *, double, double dy) {
  //    if (ui.mouseOverMenu) {
  //      ui.scroll = -int(dy);
  //  }
  //  else {
        if (dy > 0) {
            g_camera.m_zoom /= 1.1f;
        }
        else {
            g_camera.m_zoom *= 1.1f;
        }
	printf ("scroll");
	//   }
}


static void sMouseButton(GLFWwindow *, int button, int action, int mods) {
  double xd, yd;
  glfwGetCursorPos(window, &xd, &yd);
  glm::vec2 ps((float) xd, (float) yd);
  
  // Use the mouse to move things around.
  if (button == GLFW_MOUSE_BUTTON_1) {
  }
  else if (button == GLFW_MOUSE_BUTTON_2) {
    if (action == GLFW_PRESS) {
      lastp = g_camera.ConvertScreenToWorld(ps);
      rightMouseDown = true;
      glm::vec2 pw = g_camera.ConvertScreenToWorld(ps);
      //test->RightMouseDown(pw);
    }
    
    if (action == GLFW_RELEASE) {
      rightMouseDown = false;
    }
  }
}


static void sMouseMotion(GLFWwindow *, double xd, double yd) {
  glm::vec2 ps((float) xd, (float) yd);
  
  glm::vec2 pw = g_camera.ConvertScreenToWorld(ps);
  //test->MouseMove(pw);
  if (rightMouseDown) {    
    glm::vec2 diff = pw - lastp;
    g_camera.m_center.x -= diff.x;
    g_camera.m_center.y -= diff.y;
    lastp = g_camera.ConvertScreenToWorld(ps);
  }
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
  //GLFWwindow* window;
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

	printf("loading...\n");
	// Load assets
	//bg = svgParseFromFile("F:\\cpp\\GLMap\\Bin\\bg2.svg");
	//if (!bg) return -1;

	
	memset(&ma, 0, sizeof(ma));
	ma.memalloc = stdAlloc;
	ma.memfree = stdFree;
	ma.userData = (void*)&allocated;
	ma.extraVertices = 256; // realloc not provided, allow 256 extra vertices.
 

	tess = tessNewTess(&ma);

	if (!tess)
		return -1;

	std::map<std::string, building> city = loadLevel("test.geojson",tess);

	printf("go...\n");

	if (!tessTesselate(tess, TESS_WINDING_POSITIVE, TESS_POLYGONS, nvp, 2, 0))
		return -1;
	printf("Memory used: %.1f kB\n", allocated/1024.0f);
	

	if (!glfwInit()) {
		printf("Failed to init GLFW.");
		return -1;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);


	mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
	
	width = mode->width - 40;
	height = mode->height - 80;

	g_camera.m_width = width;
	g_camera.m_height = height;

	//g_camera.m_span = (xmax-xmin)/2;
	g_camera.m_span = 0.5f;

	//width=800;
	//height = 600;
	
	window = glfwCreateWindow(width, height, "logistics", NULL, NULL);
	if (!window) {
		glfwTerminate();
		return -1;
	}


	glfwSetScrollCallback(window, sScrollCallback);
	glfwSetCursorPosCallback(window, sMouseMotion);
        glfwSetMouseButtonCallback(window, sMouseButton);

	glfwMakeContextCurrent(window);

	glewExperimental = GL_TRUE;
	glewInit();


	glfwSetKeyCallback(window, key);
	glfwMakeContextCurrent(window);
	

	glfwSetTime(0);

	const float* verts = tessGetVertices(tess);
	const int* vinds = tessGetVertexIndices(tess);
	const int* elems = tessGetElements(tess);
	const int nverts = tessGetVertexCount(tess);
	const int nelems = tessGetElementCount(tess);
	
	//GLuint shaderProgram =  initModernOpenGL( verts,  nverts, elems,  nelems );

	
	shaderData mapSh =  drawMapShaderInit(verts, nverts, elems, nelems);

	float points[] = {
	  0.f,  0.45f,
	  0.45f,  0.45f,
	  0.45f,  0.45f,
	  0.45f, -0.f,
	  0.45f, -0.f,
	  -0.f, -0.f,
	};


	float* cont1 = city.begin()->second.coords[0].data();
	int contSize = city.begin()->second.coords[0].size();
	
	shaderData lineSh = drawLineShaderInit(cont1, contSize);
	
	while (!glfwWindowShouldClose(window))
	{
		float ct = (float)glfwGetTime();
		if (run) t += ct - pt;
		pt = ct;
		
		
		
		// Draw tesselated pieces.
		if (tess)
		{

		  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		  glClear(GL_COLOR_BUFFER_BIT);

		  
		  
		  drawMap(mapSh, g_camera);
		  drawLine(lineSh, g_camera);
		}
		
		//glEnable(GL_DEPTH_TEST);
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	
	if (tess) tessDeleteTess(tess);
	
	if (vflags)
		free(vflags);
	
	//svgDelete(bg);	
	//svgDelete(fg);	

	glfwTerminate();
	return 0;
}
