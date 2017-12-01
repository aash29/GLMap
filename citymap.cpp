#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#define GLEW_STATIC
#include <GL/glew.h>


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <GLFW/glfw3.h>
//#include "nanosvg.h"


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

#include "tesselator.h"

#include "path_impl.hpp"

//#include "st_tree.h"

#include "pddltree.hpp"


#include "utils.hpp"

#include <stdlib.h>     /* atoi */

#define IM_ARRAYSIZE(_ARR)  ((int)(sizeof(_ARR)/sizeof(*_ARR)))

pddlTreeNode root("city");

//Camera g_camera;

GLint uniTrans;


GLFWwindow *window = NULL;
bool rightMouseDown;
glm::vec2 lastp;
glm::vec2 selp;

std::map<std::string, building> city;
std::string selected;
shaderData lineSh;

std::string state;

polygon singlePolygon;

int xm,xp,ym,yp;


map_t* path_map;


bool drawGrid = true;
bool drawBlockedCells = false;

char* text;
int stateSize;

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
  else
    {
        io.MouseWheel += (dy != 0.0f) ? dy > 0.0f ? 1 : - 1 : 0;
	//debug_log().AddLog("%f \n",io.MouseWheel);
	
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
	  /*
       debug_log().AddLog("vertx one building:");
      for (int i=0; i< numVert; i++){
	
	debug_log().AddLog("%g,",vertx[i] );
      };
      debug_log().AddLog("\n");
	  */
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
  
  if (!io.WantCaptureMouse) 
{

    // Use the mouse to move things around.
    if (button == GLFW_MOUSE_BUTTON_1) {
      std::string id1 = selectBuilding(selp.x,selp.y);  
      debug_log().AddLog(id1.c_str());
      debug_log().AddLog("\n");
	  /*
      debug_log().AddLog("vertx single polygon:");
      for (int i=0; i< singlePolygon.nvert; i++){
	
	debug_log().AddLog("%g,",singlePolygon.vertx[i] );
      };
      debug_log().AddLog("\n");
	  */
      if (pnpoly(singlePolygon.nvert, singlePolygon.vertx, singlePolygon.verty, selp.x, selp.y)>0)
	{	      
	  debug_log().AddLog("hit \n");
	}


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
  ImGuiIO &io = ImGui::GetIO();

  if (!io.WantCaptureKeyboard)
    {
      TESS_NOTUSED(scancode);
      TESS_NOTUSED(mods);
      if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
	glfwSetWindowShouldClose(window, GL_TRUE);
      if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
 	run = !run;
    }
  else
    {
      if (action == GLFW_PRESS)
        io.KeysDown[key] = true;
      if (action == GLFW_RELEASE)
        io.KeysDown[key] = false;
    }
};

void charCallback(GLFWwindow*, unsigned int c)
{
  ImGuiIO& io = ImGui::GetIO();
  if (c > 0 && c < 0x10000)
    io.AddInputCharacter((unsigned short)c);
};



/*
void loadJsonState(std::string name )
{
  nlohmann::json jsonObj;

  std::ifstream file;
  file.open(std::string(name), std::ios::in);
  if (file) {
    printf("file open \n");
    jsonObj << file;
    file.close();
  }

  nlohmann::json::iterator f1 = jsonObj.find("objects");

  debug_log().AddLog("********** \n"); 
  debug_log().AddLog(f1->dump().c_str());
  debug_log().AddLog("\n");
  debug_log().AddLog("********** \n");

  
  nlohmann::json::iterator f2 = f1->find("name");
  debug_log().AddLog(f2->dump().c_str());
  debug_log().AddLog("\n");
  
  //std::cout << f1;
  //auto f1 = jsonObj.find("objects");

  //g_camera.Add
  
}
*/

bool doAction(std::string name, std::string parameters)
{
  std::vector<std::string> parValues = tokenize(parameters, ' ');
  pddlTreeNode* r1 = root.search(":action",name+".*").front();
  pddlTreeNode* r2 = r1->search(":parameters", ".*").front();
  pddlTreeNode* init = root.search(":init", ".*").front();
  
  std::vector<std::string> parNames;
  for (auto n1: r2->children)
    {
      parNames.push_back(n1.data);
    };
  pddlTreeNode* preconditions = r1->search(":precondition",".*").front()->search("and",".*").front();

  
  
  for (auto n2: preconditions->children)
    {
      std::string s1 = n2.flattenChildren();
      
      for (int i=0; i<parNames.size();i++)
	{
	  replaceSubstrs(s1,parNames[i],parValues[i]);
	};

      debug_log().AddLog(s1.c_str());
      debug_log().AddLog("\n");
      
      debug_log().AddLog(n2.data.c_str());
      debug_log().AddLog("\n");

      s1 = s1 + ".*";
      
      if (init->search(n2.data,s1).size()==0)
	{
	   debug_log().AddLog("preconditions not satisfied");
	  return false;
	}
      
    };
  
  return true;
  
}


std::string loadState(std::string fileName )
{

  std::ifstream file;
  file.open(std::string(fileName), std::ios::in);

  std::string stateString((std::istreambuf_iterator<char>(file)),
			  std::istreambuf_iterator<char>());

  
  std::string stateOut(stateString);

  std::string curStr;
  std::vector<std::string> tokens = std::vector<std::string>();


  removeSubstrs(stateString,std::string("\t"));
  removeSubstrs(stateString,std::string("\n"));
  removeSubstrs(stateString,std::string("\r")); 
 

  for (int i = 0; i < stateString.length(); i++) {
    char c = stateString[i];
    /*
	if ((c == '(') or (c == ')'))
	{
		//tokens.push_back(std::string(1, c));
	};
    */

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
      }
  }


  debug_log().AddLog("\n ***** tokens: \n");
  for (auto s1 : tokens) {
    debug_log().AddLog(s1.c_str());
    debug_log().AddLog("\n");
  }
  file.close();
  /*
  int p1 = stateString.find("(:objects") + sizeof("(:objects");

  stateString.insert(p1," l-0-0 ");

  stateString.insert(p1," l-1-0 ");
  
  stateSize = stateString.length();
    
  text = new char[stateSize];
  
  memcpy(text, stateString.c_str(), stateSize);
  */
  
  pddlTreeNode* curNode = &root;
  std::vector<pddlTreeNode* > stack;
  
  for (auto t1 = tokens.begin(); t1 != tokens.end(); t1++)
    {

      if ((*t1 == "("))
	{
	  curNode->insert_back(pddlTreeNode(*(t1+1)));
	  t1++;
	  stack.push_back(curNode);
	  curNode = &(curNode->children.back());
	  continue;
	}
      else
	if (*t1 == ")")
	  {
	    curNode = stack.back();
	    stack.pop_back();
	    /*
	    debug_log().AddLog("\n  ");
	    
	    for (auto it1: curNode->children)
	      {
		debug_log().AddLog(it1.data.c_str());
	      }
	    debug_log().AddLog("\n  ");
	    */
	      
	  }
	else
	  {
	    curNode->insert_back(pddlTreeNode(*(t1)));
	  }
    }

	std::vector<pddlTreeNode*> r1 =  root.search(":objects",".*");


	for (int i=-xm;i<xp;i++)
		for (int j=-ym;j<yp;j++)
		{
			char ss1[50];
			sprintf ( ss1, "loc_%d_%d", i,j );
			r1.front()->insert_back(pddlTreeNode(ss1));
		}



	pddlTreeNode* cn = root.search(":init",".*").front();

	for (int i=-xm+1;i<xp-1;i++)
		for (int j=-ym+1;j<yp-1;j++)
		{
			if (path_map->walkable[path_map->at(i, j)])
			{
				if (path_map->walkable[path_map->at(i-1, j)])
				{
					pddlTreeNode tn = pddlTreeNode("con");

					char ss1[50];
					sprintf ( ss1, "loc_%d_%d", i-1,j );
					tn.insert_back(pddlTreeNode(ss1));

					char ss2[50];
					sprintf ( ss2, "loc_%d_%d", i,j );
					tn.insert_back(pddlTreeNode(ss2));

					cn->insert_back(tn);
				}

			}
		}


  //std::vector<pddlTreeNode*> r1 =  root.search("at","agent0");
  /*
  for (auto it1: r1)
    {
      debug_log().AddLog("\n");
      debug_log().AddLog(it1->data.c_str());
      debug_log().AddLog("\n");      
    }

  */
  return stateOut;  
};




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
    ImGui::Text("Current cell: (%f, %f)", floor(pw.x / g_camera.gridSize), floor(pw.y / g_camera.gridSize));
    
    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

    if (selected!=std::string("none"))
      {
        
	ImGui::Text((std::string("id:") + city[selected].id).c_str());
      }

    ImGui::SliderFloat("North dir", &angleNorth, -90.f, 90.f);

    ImGui::SliderFloat("aspect ratio", &geoRatio, 0.3f, 2.f);

    ImGui::Checkbox("Draw Blocked Cells", &drawBlockedCells);
    ImGui::Checkbox("Draw Grid", &drawGrid);
    
    
    ImGui::End();
    
    ImGui::PopStyleColor();


    ImGui::ShowTestWindow();

    debug_log().Draw("Log");

    
    ImFontAtlas* atlas = ImGui::GetIO().Fonts;
    ImFont* font = atlas->Fonts[0];
    //font->Scale = 2.f;
  }


  //visitNodes(&root);


	ImGui::Begin("State");


	//static char* text = &state[0];
	/*
    ImGui::InputTextMultiline("##source", text, stateSize*2 , ImVec2(-1.0f, ImGui::GetTextLineHeight() * 16), ImGuiInputTextFlags_AllowTabInput);


*/
	static char str0[128] = "city";
	ImGui::InputText("node", str0, IM_ARRAYSIZE(str0));

	static char str1[128] = ".*";
	ImGui::InputText("filter", str1, IM_ARRAYSIZE(str1));

	if (ImGui::IsItemHovered() || (ImGui::IsRootWindowOrAnyChildFocused() && !ImGui::IsAnyItemActive() && !ImGui::IsMouseClicked(0)))
	  ImGui::SetKeyboardFocusHere(-1); // Auto focus previous widget

	static std::vector<pddlTreeNode*> s_res;

	if (ImGui::Button("Load"))
	{       
	  s_res  = root.search(std::string(str0),std::string(str1));
	};
	for (auto r1:s_res){
	  visitNodes(r1);
	}
	ImGui::End();


	ImGui::Begin("Test actions");
	if (ImGui::Button("Move"))
	  {
	    doAction("move","agent0 loc_1_1 loc_2_1");
	  }
	ImGui::End();

	
};




void getAgentPos(std::string state, std::string agent, int &x, int &y)
{
	int p1 = state.find("(at "+ agent);
	int p2 = state.find(")", p1);
	std::string str2 = state.substr(p1 + 1, p2 - p1);

	std::stringstream ss(str2);

	std::string id;
	ss >> id;
	ss >> id;
	ss >> x;
	ss >> y;
}

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


//	debug_log().AddLog(root.children[0].data.c_str());

	//loadJsonState("city.json");
		
	
	rect boundingBox;


	city = loadLevel("little.geojson",tess, boundingBox, singlePolygon);

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
	glfwSetKeyCallback(window, key);
	glfwSetCharCallback(window, charCallback);
	  


	

	glewExperimental = GL_TRUE;
	glewInit();

	ImGui_ImplGlfwGL3_Init(window, false);

	
	//glfwSetWindowAspectRatio(window, width, height);
	
	//sInterfaceInit();
	
	//glEnable(GL_DEPTH_TEST);	
	

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

	std::vector<float> gridVec = std::vector<float>();


	debug_log().AddLog("bb: %g,%g,%g,%g \n",boundingBox.xmin, boundingBox.xmax, boundingBox.ymin ,boundingBox.ymax);



	xm = 0;
	xp = 0;
	ym = 0;
	yp = 0;

	
	for (float x=0; x < boundingBox.xmax; x=x+g_camera.gridSize)
	  {
	    gridVec.push_back(x + g_camera.gridSize/2);
	    gridVec.push_back(boundingBox.ymin);
	    gridVec.push_back(x + g_camera.gridSize / 2);
	    gridVec.push_back(boundingBox.ymax);
	    xp++;
	  }

	for (float x=0; x > boundingBox.xmin; x=x-g_camera.gridSize)
	  {
	    gridVec.push_back(x - g_camera.gridSize / 2);
	    gridVec.push_back(boundingBox.ymin);
	    gridVec.push_back(x - g_camera.gridSize / 2);
	    gridVec.push_back(boundingBox.ymax);
	    xm++;
	  }

    	for (float y=0; y < boundingBox.ymax; y=y+g_camera.gridSize)
	  {
	    gridVec.push_back(boundingBox.xmin);
	    gridVec.push_back(y + g_camera.gridSize / 2);
	    gridVec.push_back(boundingBox.xmax);
	    gridVec.push_back(y + g_camera.gridSize / 2);
	    yp++;
	  }


    	for (float y=0; y > boundingBox.ymin; y=y-g_camera.gridSize)
	  {
	    gridVec.push_back(boundingBox.xmin);
	    gridVec.push_back(y- g_camera.gridSize / 2);
	    gridVec.push_back(boundingBox.xmax);
	    gridVec.push_back(y - g_camera.gridSize / 2);
	    ym++;
	  }

	
	float* grid = gridVec.data();


	shaderData gridSh = drawLineShaderInit(grid, 2*(xm+xp+ym+yp));


	
	std::vector<float> outlines = getOutlines(city);
	debug_log().AddLog("%f,%f,%f,%f",outlines[0],outlines[1],outlines[2],outlines[3]);

	float* outlinesData = outlines.data();
	int outlineVerts = round(outlines.size()/2); 
	
	
	shaderData outlineSh = drawBuildingOutlinesInit(outlinesData,outlineVerts);


	shaderData texSh = texQuadInit();


	shaderData quadSh = drawQuadInit();
	

	int x, y;

	std::vector<std::string> agents;
/*
	int p1 = state.find("(:objects") + sizeof("(:objects");

	int p2 = state.find(")", p1) - 1;

	std::string str2 = state.substr(p1 + 1, p2 - p1);

	debug_log().AddLog(str2.c_str());
	debug_log().AddLog("\n");
	
	std::stringstream ss(str2);
	
	
	while (ss)
	  {
	    std::string id;
	    std::string type;
	    ss >> id;
	    agents.push_back(id);
	    debug_log().AddLog("agent id:");
	    debug_log().AddLog(id.c_str());
	    debug_log().AddLog("\n");
	    
	    ss>>type;
	    debug_log().AddLog("agent type:");
   	    debug_log().AddLog(type.c_str());
	    debug_log().AddLog("\n");

	    ss>>type;
	    
	}
*/
	
	getAgentPos(state, "agent0", x, y);
	debug_log().AddLog("agent0 pos: %d,%d", x, y);
	for (std::string s1 : agents){
	  debug_log().AddLog(s1.c_str());
	  debug_log().AddLog("\n");
	};
	

	std::shared_ptr<navigation_path<location_t>> path;


	static map_t pathfinding_map(-xm, xp, -ym, yp);

	path_map = &pathfinding_map;



	struct navigator {
		// This lets you define a distance heuristic. Manhattan distance works really well, but
		// for now we'll just use a simple euclidian distance squared.
		// The geometry system defines one for us.

		static float get_distance_estimate(location_t &pos, location_t &goal) {
			float d = distance2d_squared(pos.x, pos.y, goal.x, goal.y);
			return d;
		}

		// Heuristic to determine if we've reached our destination? In some cases, you'd not want
		// this to be a simple comparison with the goal - for example, if you just want to be
		// adjacent to (or even a preferred distance from) the goal. In this case,
		// we're trying to get to the goal rather than near it.
		static bool is_goal(location_t &pos, location_t &goal) {
			return pos == goal;
			//return (std::max(abs(pos.x-goal.x),abs(pos.y-goal.y))<=1.1f);
			//return ((abs(pos.x - goal.x)<=1)&&(abs(pos.y - goal.y)<=1));
		}

		// This is where we calculate where you can go from a given tile. In this case, we check
		// all 8 directions, and if the destination is walkable return it as an option.
		static bool get_successors(location_t pos, std::vector<location_t> &successors) {
			//std::cout << pos.x << "/" << pos.y << "\n";

			if (pathfinding_map.walkable[pathfinding_map.at(pos.x - 1, pos.y - 1)]) successors.push_back(location_t(pos.x - 1, pos.y - 1));
			if (pathfinding_map.walkable[pathfinding_map.at(pos.x, pos.y - 1)]) successors.push_back(location_t(pos.x, pos.y - 1));
			if (pathfinding_map.walkable[pathfinding_map.at(pos.x + 1, pos.y - 1)]) successors.push_back(location_t(pos.x + 1, pos.y - 1));

			if (pathfinding_map.walkable[pathfinding_map.at(pos.x - 1, pos.y)]) successors.push_back(location_t(pos.x - 1, pos.y));
			if (pathfinding_map.walkable[pathfinding_map.at(pos.x + 1, pos.y)]) successors.push_back(location_t(pos.x + 1, pos.y));

			if (pathfinding_map.walkable[pathfinding_map.at(pos.x - 1, pos.y + 1)]) successors.push_back(location_t(pos.x - 1, pos.y + 1));
			if (pathfinding_map.walkable[pathfinding_map.at(pos.x, pos.y + 1)]) successors.push_back(location_t(pos.x, pos.y + 1));
			if (pathfinding_map.walkable[pathfinding_map.at(pos.x + 1, pos.y + 1)]) successors.push_back(location_t(pos.x + 1, pos.y + 1));
			return true;
		}

		// This function lets you set a cost on a tile transition. For now, we'll always use a cost of 1.0.
		static float get_cost(location_t &position, location_t &successor) {
			return 1.0f;
		}

		// This is a simple comparison to determine if two locations are the same. It just passes
		// through to the location_t's equality operator in this instance (we didn't do that automatically)
		// because there are times you might want to behave differently.
		static bool is_same_state(location_t &lhs, location_t &rhs) {
			return lhs == rhs;
		}
	};





	state = loadState("city.problem");

	for (int i = -xm; i < xp; i++)
	{
		for (int j = -ym; j < yp; j++)
		{
			if (pnpoly(singlePolygon.nvert, singlePolygon.vertx, singlePolygon.verty, i*g_camera.gridSize, j*g_camera.gridSize)>0)
			{
				debug_log().AddLog("hit \n");
				path_map->walkable[path_map->at(i, j)] = false;

			}
		}
	}

	debug_log().AddLog("xm:%d,xp:%d,ym:%d,yp:%d \n", xm,xp,ym,yp);
	/*
	debug_log().AddLog("x=0,y=0, at(x,y)= %d \n", map.at(0,0));
	debug_log().AddLog("x=2,y=2, at(x,y)= %d \n", map.at(2,2));
	debug_log().AddLog("x=0,y=1, at(x,y)= %d \n", map.at(0,1));
	*/
	
	location_t dude_position {0,0};
	location_t destination {4,5};


	path = find_path<location_t, navigator>(dude_position, destination);
	if (path->success)
	  {
	    debug_log().AddLog("path found \n");
	    for (auto p1 = path->steps.begin(); p1 != path->steps.end(); p1++){
	      debug_log().AddLog("%d,%d \n", p1->x,p1->y);
	    }
	  }



	
	while (!glfwWindowShouldClose(window))
	{
		float ct = (float)glfwGetTime();
		if (run) t += ct - pt;
		pt = ct;

		glfwPollEvents();
		
		ImGui_ImplGlfwGL3_NewFrame();
		//glfwPollEvents();
		
		sInterface();
		
				
		// Draw tesselated pieces.
		if (tess)
		{

		  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		  glClear(GL_COLOR_BUFFER_BIT);

		  glEnable(GL_BLEND);
		  glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
		  glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);


		  if (drawGrid)
		    drawLine(gridSh,g_camera);

		  drawMap(mapSh, g_camera);
		  drawBuildingOutlines( outlineSh, g_camera);
		  if (selected!=std::string("none"))
		    drawLine(lineSh, g_camera);


		  
		  for (std::string s1 : agents){

		    getAgentPos(state, s1, x, y);
		  
		    texQuadDraw(texSh,x,y);
		  }
		  


		  if (drawBlockedCells)
		    {
		      for (int i = -xm; i < xp; i++)
			{
			  for (int j = -ym; j < yp; j++)
			    {
			      if (!path_map->walkable[path_map->at(i, j)]) {
				drawQuad(quadSh, i, j);
			      }
			    }
			};
		    };
		  
		  ImGui::Render();
		}
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	
	if (tess) tessDeleteTess(tess);
	
	if (vflags)
		free(vflags);
	
	glfwTerminate();
	return 0;
}
