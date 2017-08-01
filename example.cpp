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


// Undefine this to see non-interactive heap allocator version.
#define USE_POOL 1


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
	struct SVGPath* fg;
	struct SVGPath* it;
	float bounds[4],view[4],cx,cy,w,offx,offy;
	float t = 0.0f, pt = 0.0f;
	TESSalloc ma;
	TESStesselator* tess = 0;
	const int nvp = 6;
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
	
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	//glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
	

	printf("loading...\n");
	// Load assets
	bg = svgParseFromFile("./Bin/bg.svg");
	if (!bg) return -1;

	printf("go...\n");
	
			
	// Find BG bounds.
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
		

	pool.size = 0;
	pool.cap = sizeof(mem);
	pool.buf = mem;
	memset(&ma, 0, sizeof(ma));
	ma.memalloc = poolAlloc;
	ma.memfree = poolFree;
	ma.userData = (void*)&pool;
	ma.extraVertices = 256; // realloc not provided, allow 256 extra vertices.

	
	mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
	width = mode->width - 40;
	height = mode->height - 80;
	
	window = glfwCreateWindow(width, height, "Libtess2 Demo", NULL, NULL);
	if (!window) {
		glfwTerminate();
		return -1;
	}

	glfwSetKeyCallback(window, key);
	glfwMakeContextCurrent(window);

	// Adjust bounds so that we get nice view of the bg.
	cx = (bounds[0]+bounds[2])/2;
	cy = (bounds[3]+bounds[1])/2;
	w = (bounds[2]-bounds[0])/2;

	view[0] = cx - w*1.2f;
	view[2] = cx + w*1.2f;
	view[1] = cy - w*1.2f*(float)height/(float)width;
	view[3] = cy + w*1.2f*(float)height/(float)width;
		
	glfwSetTime(0);



	glewExperimental = GL_TRUE;
	glewInit();


	
	while (!glfwWindowShouldClose(window))
	{
		float ct = (float)glfwGetTime();
		if (run) t += ct - pt;
		pt = ct;
		
		glClearColor(0.3f, 0.3f, 0.32f, 1.0f);

#ifdef USE_POOL
		pool.size = 0; // reset pool
		tess = tessNewTess(&ma);
		if (tess)
		{
			offx = (view[2]+view[0])/2 + sinf(t) * (view[2]-view[0])/2;
			offy = (view[3]+view[1])/2 + cosf(t*3.13f) * (view[3]-view[1])/6;
			
			for (it = bg; it != NULL; it = it->next)
				tessAddContour(tess, 2, it->pts, sizeof(float)*2, it->npts);


			// First combine contours and then triangulate, this removes unnecessary inner vertices.
			if (tessTesselate(tess, TESS_WINDING_POSITIVE, TESS_BOUNDARY_CONTOURS, 0, 0, 0))
			{
				const float* verts = tessGetVertices(tess);
				const int* vinds = tessGetVertexIndices(tess);
				const int nverts = tessGetVertexCount(tess);
				const int* elems = tessGetElements(tess);
				const int nelems = tessGetElementCount(tess);

				if (nverts > nvflags)
				{
					if (vflags)
						free(vflags);
					nvflags = nverts;
					vflags = (unsigned char*)malloc(sizeof(unsigned char)*nvflags);
				}
				
				if (vflags)
				{
					// Vertex indices describe the order the indices were added and can be used
					// to map the tesselator output to input. Vertices marked as TESS_UNDEF
					// are the ones that were created at the inteesrsection of segments.
					// That is, if vflags is set it means that the vertex comes from intersegment.
					for (i = 0; i < nverts; ++i)
						vflags[i] = vinds[i] == TESS_UNDEF ? 1 : 0;
				}
				
				for (i = 0; i < nelems; ++i)
				{
					int b = elems[i*2];
					int n = elems[i*2+1];
					tessAddContour(tess, 2, &verts[b*2], sizeof(float)*2, n);
				}
				if (!tessTesselate(tess, TESS_WINDING_POSITIVE, TESS_POLYGONS, nvp, 2, 0))
					tess = 0;
			}
			else
				tess = 0;				
		}
#endif		

		// Draw tesselated pieces.
		if (tess)
		{
			const float* verts = tessGetVertices(tess);
			const int* vinds = tessGetVertexIndices(tess);
			const int* elems = tessGetElements(tess);
			const int nverts = tessGetVertexCount(tess);
			const int nelems = tessGetElementCount(tess);


			GLuint vertexBuffer;
			glGenBuffers(1, &vertexBuffer);

			for (int i = 0; i < nverts*2; ++i)
			  printf("%f, ", verts[i]);
			printf("\n");

			for (int i = 0; i < nelems; i++)
			  printf("%d, ", vinds[i]);
			printf("\n");



			printf("%u\n", vertexBuffer);
			
			float vertices[] = {
			  0.0f,  0.5f, // Vertex 1 (X, Y)
			  0.5f, -0.5f, // Vertex 2 (X, Y)
			  -0.5f, -0.5f,  // Vertex 3 (X, Y)
			};
			
			GLuint vao;
			glGenVertexArrays(1, &vao);

			glBindVertexArray(vao);
			
			GLuint vbo;
			glGenBuffers(1, &vbo); // Generate 1 buffer
			
			glBindBuffer(GL_ARRAY_BUFFER, vbo);
			
			glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
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


			glDrawElements(GL_TRIANGLES, 0, nverts, vinds);

			/*

			
			// Draw polygons.
			glColor4ub(255,255,255,128);
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
		}
		
		glEnable(GL_DEPTH_TEST);

			*/

		}
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	
	if (tess) tessDeleteTess(tess);
	
	if (vflags)
		free(vflags);
	
	svgDelete(bg);	
	svgDelete(fg);	

	glfwTerminate();
	return 0;
}
