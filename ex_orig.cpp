#define GLEW_STATIC
#include <GL/glew.h>


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <GLFW/glfw3.h>
#include "nanosvg.h"
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

float xmin,xmax,ymin,ymax;
Camera g_camera;

GLint uniTrans;

GLFWwindow *window = NULL;
bool rightMouseDown;
glm::vec2 lastp;

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



void loadLevel(const char *name,TESStesselator* tess)
{
  
  
  std::string m_currentLevel = std::string(name);
  

  nlohmann::json jsonObj;

  std::ifstream file;
  file.open(std::string(name), std::ios::in);
  if (file) {
    printf("file open \n");
    jsonObj << file;
    file.close();
  }

  auto f1 = jsonObj.find("features");


  std::map<std::string, building> m3;

  
  if (f1 != jsonObj.end()) {
    //m_force = m_forceLeft;
    std::cout << "found features";
    //std::cout << ((*f1)[0]).dump(4);


	std::map<std::string, float**> m2;
	float** a2;
	float* a1[10];


	xmin = 30.2882633f;
	xmax = 30.2882633f;
	ymin = 59.9379525f;
	ymax = 59.9379525f;

    for (nlohmann::json::iterator it = (*f1).begin(); it != (*f1).end(); ++it) {
      if (((*it)["properties"]).find("building") != ((*it)["properties"]).end()) {
		std::cout << "id:" << (*it)["properties"]["id"] << "\n";
		//std::cout <<  (*it)["geometry"]["type"];
		
		if ((*it)["geometry"]["type"]=="Polygon"){

		std::vector< std::vector<std::vector<float> > > c1 =  (*it)["geometry"]["coordinates"];


		a2 = new float*[c1.size()];
		std::string id = (*it)["properties"]["id"];
		m3[id].coords=std::vector <std::vector <float> >();
		m3[id].id=id;

		for (int j = 0; j<c1.size();j++){

		  a2[j] = new float[c1[j].size()*2];
		  m3[id].coords.push_back(std::vector<float>());
		  
		  std::cout << "contour size:" << c1[j].size() << "\n";
	  
		  for (int i = 0; i<c1[j].size();i++){
			a2[j][2*i]=c1[j][i][0];
			a2[j][2*i+1]=c1[j][i][1];

			m3[id].coords[j].push_back(c1[j][i][0]);
			m3[id].coords[j].push_back(c1[j][i][1]);
			
	    
			xmin=std::min(xmin,a2[j][2*i]);
			xmax=std::max(xmax,a2[j][2*i]);
	    
			ymin=std::min(ymin,a2[j][2*i+1]);
			ymax=std::max(ymax,a2[j][2*i+1]);
		  };
		  std::cout << "adding contour" << "\n";
		}
		
		m2[(*it)["properties"]["id"]] = a2;
		/*
		for (int j = 0; j<c1.size();j++){

		  tessAddContour(tess, 2, m3[id].coords[j].data(), sizeof(float) * 2, c1[j].size());
		  };*/
		
		  //free(a1);
		};

	
	//std::cout << c1 << '\n';
      }
      /*
      if ((*it)["properties"]["type"]=="route"){
	std::cout << *it << '\n';
      }
      */
    }
  }

      for (auto &it : m3) {
	for (int j = 0; j < it.second.coords.size(); j++) {
	  for (int i = 0; i < it.second.coords[j].size();i=i+2){
	    it.second.coords[j][i] = (it.second.coords[j][i]-xmin)/(xmax-xmin);
	    it.second.coords[j][i+1] = (it.second.coords[j][i+1]-ymin)/(ymax-ymin);
	  }		  
	  tessAddContour(tess, 2, it.second.coords[j].data(), sizeof(float) * 2, round(it.second.coords[j].size()/2));
	}
      }

}


void initModernOpenGL(const float* verts, const int nverts, const TESSindex* elements, const  int nelements )
{
  


  GLuint vertexBuffer;
  glGenBuffers(1, &vertexBuffer);

  //printf("%u\n", vertexBuffer);

  GLuint vao;
  glGenVertexArrays(1, &vao);
  
  glBindVertexArray(vao);
  
  GLuint vbo;
  glGenBuffers(1, &vbo); // Generate 1 buffer
  
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
			
  glBufferData(GL_ARRAY_BUFFER, sizeof(float)*nverts*2, verts, GL_STATIC_DRAW);

  
  GLuint ebo;
  glGenBuffers(1, &ebo);

  printf("nelements:%d \n", nelements);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER,
	       sizeof(int)*nelements*3, elements, GL_STATIC_DRAW);
  

  
  const char* vertexSource = R"glsl(
	in vec2 position;
in vec3 color;
in vec2 texcoord;


uniform mat4 Model;

void main()
{
    gl_Position = Model * vec4(position, 0.0, 1.0);
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


  //glm::mat4 Model = glm::scale(glm::mat4(1.0f), glm::vec3(0.001f));

  g_camera.m_center = glm::vec2(0.5f,0.5f);

  //  float proj[16] = { 0.0f };
  glm::mat4 Model = g_camera.BuildProjectionMatrix();

  

  //glm::mat4 Model = glm::ortho(xmin,xmax,ymin,ymax,-1.f,1.f);
  
  uniTrans = glGetUniformLocation(shaderProgram, "Model");
  glUniformMatrix4fv(uniTrans, 1, GL_FALSE, glm::value_ptr(Model));
  //glUniformMatrix4fv(uniTrans, 1, GL_FALSE, proj);

			
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

	loadLevel("little.geojson",tess);

	printf("go...\n");
	

	
	// Offset the foreground shape to center of the bg.
	offx = (bounds[2]+bounds[0])/2;
	offy = (bounds[3]+bounds[1])/2;
	
	// Add contours.
	//for (it = bg; it != NULL; it = it->next)
	//	tessAddContour(tess, 2, it->pts, sizeof(float)*2, it->npts);

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
	const TESSindex* elems = tessGetElements(tess);
	const int nverts = tessGetVertexCount(tess);
	const int nelems = tessGetElementCount(tess);

	float vertices[] = {
		0.0f,  0.5f, // Vertex 1 (X, Y)
		0.5f, -0.5f, // Vertex 2 (X, Y)
		-0.5f, -0.5f,  // Vertex 3 (X, Y)
	};

	
	
	initModernOpenGL( verts,  nverts, elems,  nelems );
	//initModernOpenGL( vertices,  3, elems,  nelems );

	/*		
	printf("num elems: %d \n",nelems);
	for (int i = 0; i<nelems; i++)
	  {
	    for (int j = 0; j < 3; j++) {
	      printf("%d,",(&elems[i * 3])[j]);
	    }
	    printf("\n");
	  };
	printf("\n");			
	*/

	
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


			int polySize = 3;
			int vertexSize = 2;

			
			
			// Draw polygons.
			//glColor4ub(255,255,255,128);

			//initModernOpenGL( verts,  nverts, elems,  nelems );
			
			//glDrawArrays(GL_TRIANGLES, 0, 3);


			  glm::mat4 Model = g_camera.BuildProjectionMatrix();
 
			  glUniformMatrix4fv(uniTrans, 1, GL_FALSE, glm::value_ptr(Model));

			  glDrawElements(GL_TRIANGLES, nelems*3, GL_UNSIGNED_INT, 0);
			/*
			for (int i = 0; i < nelems2; i++) {
			  const TESSindex* poly = &elems2[i * polySize];
			  glBegin(GL_POLYGON);
			  for (int j = 0; j < polySize; j++) {
			    if (poly[j] == TESS_UNDEF) break;
			    glVertex2fv(&verts[poly[j]*vertexSize]);
			  }
			  glEnd();
			}
			*/
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
