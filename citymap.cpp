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
#include "graphics.hpp"
#include "imgui.h"
#include "imgui_impl_glfw_gl3.h"
#include "appLog.h"
#include "map.hpp"




//Camera g_camera;

GLint uniTrans;

GLFWwindow *window = NULL;
bool rightMouseDown;
glm::vec2 lastp;
glm::vec2 selp;

std::map<std::string, building> city;
std::string selected;
shaderData lineSh;

//UIState ui;

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

  ImGuiIO &io = ImGui::GetIO();

  //std::cout<<"pressed" << "\n";
  
  if (!io.WantCaptureMouse) {
        if (dy > 0) {
            g_camera.m_zoom /= 1.1f;
        }
        else {
            g_camera.m_zoom *= 1.1f;
        }
	//printf ("scroll");
    }
}


    static int pnpoly(int nvert, double *vertx, double *verty, double testx, double testy)
    {
        int i, j, c = 0;
        for (i = 0, j = nvert-1; i < nvert; j = i++) {
        if ( ((verty[i]>testy) != (verty[j]>testy)) && (testx < (vertx[j]-vertx[i]) * (testy-verty[i]) / (verty[j]-verty[i]) + vertx[i]) )
            c = !c;
        }
    return c;
    }


std::string selectBuilding(float testx, float testy)
{



  glm::mat4 rotN;
  rotN = glm::rotate(rotN, glm::radians(-angleNorth), glm::vec3(0.0f, 0.0f, 1.0f));

  


  std::vector<float> unCol = std::vector<float>();
  std::vector<float> unDraw = std::vector<float>();

  unCol.push_back(0.f);
  unCol.push_back(0.f);
	
  for (auto b1: city)
    {
      std::string id1 = b1.first;
      for (auto it: city[id1].coords)
	{
	  //numVert+=round(it.size()/2);
	  unCol.insert(unCol.end(),it.begin(),it.end());
	  unDraw.push_back(it[0]);
	  unDraw.push_back(it[1]);
	  for (int i=2;i<it.size();i=i+2)
	    {
	      unDraw.push_back(it[i]);
	      unDraw.push_back(it[i+1]);

	      unDraw.push_back(it[i]);
	      unDraw.push_back(it[i+1]);		
	    }
	  unDraw.push_back(it[0]);
	  unDraw.push_back(it[1]);
	  //unDraw.insert(unDraw.end(),it.begin(),it.end());
	  unCol.push_back(0.f);
	  unCol.push_back(0.f);
	  //numVert++;
	};


      float* cont1 = unCol.data();
      int numVert = round(unCol.size()/2);
	  
      double* vertx = new double[numVert];
      double* verty = new double[numVert];

      for (int i = 0; i<numVert; i++){
	vertx[i]=cont1[2*i];
	verty[i]=cont1[2*i+1];
      }

      if (pnpoly(numVert, vertx, verty, testx, testy)>0)
	{
	  return id1;
	}
      delete vertx;
      delete verty;
    }
  return std::string("none");

};


static void sMouseButton(GLFWwindow *, int button, int action, int mods) {
  double xd, yd;
  glfwGetCursorPos(window, &xd, &yd);
  glm::vec2 ps((float) xd, (float) yd);
  selp = g_camera.ConvertScreenToWorld(ps);
  
  ImGuiIO &io = ImGui::GetIO();

  //std::cout<<"pressed" << "\n";
  
  if (!io.WantCaptureMouse) {


    // Use the mouse to move things around.
    if (button == GLFW_MOUSE_BUTTON_1) {
      std::string id1 = selectBuilding(selp.x,selp.y);  
      debug_log().AddLog(id1.c_str());
      debug_log().AddLog("\n");

      std::vector<float> unDraw = std::vector<float>();

      if (id1!=std::string("none"))
	{
	  selected = id1;
	  std::vector<float> unDraw = std::vector<float>();

	  for (auto it: city[id1].coords)
	    {
	      unDraw.push_back(it[0]);
	      unDraw.push_back(it[1]);
	      for (int i=2;i<it.size()-1;i=i+2)
		{
		  unDraw.push_back(it[i]);
		  unDraw.push_back(it[i+1]);

		  unDraw.push_back(it[i]);
		  unDraw.push_back(it[i+1]);		
		}
	      unDraw.push_back(it[0]);
	      unDraw.push_back(it[1]);
	    };

	  lineSh.vertexCount = round(unDraw.size()/2);
	  lineSh.data = unDraw.data();
	  glBindBuffer(GL_ARRAY_BUFFER, lineSh.vbo);
	  glBufferData(GL_ARRAY_BUFFER, lineSh.vertexCount*2*sizeof(float), lineSh.data, GL_STATIC_DRAW);

	  //lineSh = drawLineShaderInit(unDraw.data(), round(unDraw.size()/2));
	}
      else {
	selected=std::string("none");
      };
      std::cout << id1 <<"\n";
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

/*
void sInterfaceInit() {


  ImGuiIO &io = ImGui::GetIO();

  //io.FontGlobalScale = 1.5f;


  //io.Fonts->AddFontFromFileTTF("DejaVuSans.ttf", 14);
  //io.Fonts->GetTexDataAsRGBA32();

  
}
*/

void sInterface() {

  if (ImGui::IsAnyWindowHovered()) {
    ImGui::CaptureMouseFromApp(true);
  }

  ImGuiIO &io = ImGui::GetIO();  
  
  {
    ImVec4 color = ImVec4(0.1f, 0.1f, 0.1f, 1.f);
    ImGuiStyle &style = ImGui::GetStyle();
    //style.Colors[ImGuiCol_WindowBg]=color;
    ImGui::PushStyleColor(ImGuiCol_WindowBg, color);    
    ImGui::Begin("Info");
    glm::vec2 ps = glm::vec2(io.MousePos.x, io.MousePos.y);  
    glm::vec2 pw = g_camera.ConvertScreenToWorld(ps);

    
    
    ImGui::Text("Mouse pos: (%f, %f)", pw.x, pw.y);    
    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

    if (selected!=std::string("none"))
      {
        
	ImGui::Text((std::string("id:") + city[selected].id).c_str());
      }



    ImGui::SliderFloat("North dir", &angleNorth, -90.f, 90.f);

    ImGui::SliderFloat("aspect ratio", &geoRatio, 0.3f, 2.f);
    
    ImGui::End();


    
    ImGui::PopStyleColor();

    ImGui::ShowTestWindow();

    debug_log().Draw("Log");
  }


}

void loadState(std::string fileName )
{

  //std::ifstream t("file.txt");

  std::ifstream file;
  file.open(std::string(fileName), std::ios::in);

  std::string stateString((std::istreambuf_iterator<char>(file)),
	  std::istreambuf_iterator<char>());


  stateString.erase(std::remove(stateString.begin(), stateString.end(), '\n'), stateString.end());

  //char c = file.get();
  std::string curStr;
  std::vector<std::string> tokens = std::vector<std::string>();
  for (int i = 0; i < stateString.length(); i++) {
	  char c = stateString[i];

		  if ((c != '(') and (c != ')') and (c != ' '))
		  {
			  curStr += c;
		  }
		  else
		  {
			  if (curStr!="")
				tokens.push_back(curStr);

			  curStr = "";
			  if (c != ' ') {
				  tokens.push_back(std::string(1, c));
				  curStr = "";
		  }

	//	  c = file.get();
	  }
  }

	for (auto s1 : tokens) {
		debug_log().AddLog(s1.c_str());
		debug_log().AddLog("\n");
	}
  file.close();


  std::vector<std::string> read_from_tokens(std::vector<std::string> tokens)
  if (tokens.size() == 0) {
	  debug_log().AddLog("unexpected EOF");
  };
  std::string token = tokens.back();
  tokens.pop_back();

  if ("(" == token ) {
	  std::vector<std::string> L = std::vector<std::string>();
	  while (tokens[0] != ")") {

	  }
				  L.append(read_from_tokens(tokens))
				  tokens.pop(0) # pop off ')'
				  return L
  }
				  elif ')' == token:
  raise SyntaxError('unexpected )')
		  else:
  return atom(token)

  
};


int main(int argc, char *argv[])
{
  //GLFWwindow* window;
	const GLFWvidmode* mode;
	int width,height,i,j;
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
	loadState("city.problem");
	
	city = loadLevel("test.geojson",tess);

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


	//width = 600;
	//height = 600;

	g_camera.m_width = width;
	g_camera.m_height = height;

	//g_camera.m_span = (xmax-xmin)/2;
	g_camera.m_span = 0.5f;

	g_camera.m_center.x = 0.1f;
	g_camera.m_center.y = 0.8f;
	
	
	window = glfwCreateWindow(width, height, "logistics", NULL, NULL);
	if (!window) {
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);
	//glfwSwapInterval(1); // Enable vsync

	
	glfwSetScrollCallback(window, sScrollCallback);
	glfwSetCursorPosCallback(window, sMouseMotion);
        glfwSetMouseButtonCallback(window, sMouseButton);


	glewExperimental = GL_TRUE;
	glewInit();

	ImGui_ImplGlfwGL3_Init(window, false);

	
	//glfwSetWindowAspectRatio(window, width, height);
	
	//sInterfaceInit();
	
	//glEnable(GL_DEPTH_TEST);	
	glfwSetKeyCallback(window, key);
	

	glfwSetTime(0);

	const float* verts = tessGetVertices(tess);
	const int* vinds = tessGetVertexIndices(tess);
	const int* elems = tessGetElements(tess);
	const int nverts = tessGetVertexCount(tess);
	const int nelems = tessGetElementCount(tess);
	
	//GLuint shaderProgram =  initModernOpenGL( verts,  nverts, elems,  nelems );

	
	shaderData mapSh =  drawMapShaderInit(verts, nverts, elems, nelems);
	float stub[4] = {0.f,0.f,0.f,0.f};
	lineSh = drawLineShaderInit(stub, 2);

	std::vector<float> outlines = getOutlines(city);
	debug_log().AddLog("%f,%f,%f,%f",outlines[0],outlines[1],outlines[2],outlines[3]);

	float* outlinesData = outlines.data();
	int outlineVerts = round(outlines.size()/2); 
	
	
	shaderData outlineSh = drawBuildingOutlinesInit(outlinesData,outlineVerts);
	
	while (!glfwWindowShouldClose(window))
	{
		float ct = (float)glfwGetTime();
		if (run) t += ct - pt;
		pt = ct;
		
		ImGui_ImplGlfwGL3_NewFrame();
		//glfwPollEvents();
		
		sInterface();
		
		
		//selected =  pnpoly(numVert, vertx, verty, selp.x, selp.y)>0;	
		
		// Draw tesselated pieces.
		if (tess)
		{

		  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		  glClear(GL_COLOR_BUFFER_BIT);
		  
		  
		  drawMap(mapSh, g_camera);
		  drawBuildingOutlines( outlineSh, g_camera);
		  if (selected!=std::string("none"))
		    drawLine(lineSh, g_camera);

		  

		  ImGui::Render();
		}
		
		
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
