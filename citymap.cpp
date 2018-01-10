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
#include "agent.h"

#include <unordered_set>
//#include <gperftools/profiler.h>

#include "camera.hpp"

#define IM_ARRAYSIZE(_ARR)  ((int)(sizeof(_ARR)/sizeof(*_ARR)))
#define strVec vector<string> 

using namespace std;


pddlTreeNode root("city");
pddlTreeNode* init;

//Camera g_camera;

GLint uniTrans;


GLFWwindow *window = NULL;
bool rightMouseDown;
glm::vec2 lastp;
glm::vec2 selp;

map<string, building> city;
string selected;
shaderData lineSh;


string state;
unordered_set<string> setState;
unordered_set<string> constants;

map<string, actionPrefab> actionPrefabs;
map <string, strVec> objects;
map<string, agent> agents;


polygon singlePolygon;

int xm,xp,ym,yp;


map_t* path_map;


bool drawGrid = true;
bool drawBlockedCells = true;

char* text;
int stateSize;

agent agent0;





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
            string id1 = selectBuilding(city,selp.x,selp.y);
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


            vector<float> unDraw = vector<float>();

            if (id1!=string("none"))
            {
                selected = id1;
                vector<float> unDraw = vector<float>();

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
                selected=string("none");
            };
            cout << id1 <<"\n";
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


void moveAgent(string id, int dx, int dy);
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
        if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS)
        {
	  moveAgent("agent0",1,0);
        }
        if (key == GLFW_KEY_LEFT && action == GLFW_PRESS)
        {
            moveAgent("agent0",-1,0);
        }
        if (key == GLFW_KEY_UP && action == GLFW_PRESS)
        {
            moveAgent("agent0",0,1);
        }
        if (key == GLFW_KEY_DOWN && action == GLFW_PRESS)
        {
            moveAgent("agent0",0,-1);
        }


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

unordered_set<string> hashState(string nodeName )
{

  unordered_set<string> result;
  
  pddlTreeNode* init = root.findFirstName(nodeName);
  for (pddlTreeNode p1: init->children)
    {
      result.insert(p1.data +" "+ p1.flattenChildren());
    }
  return result;
  
}


bool doConcreteAction (vector<string> precond, vector<string> peff, vector<string> neff, vector<string> qeff)
{

  for (string p1 : precond)
    {
      auto n3 = setState.find(p1);
  
      if (n3==setState.end())
	  {
	    debug_log().AddLog("preconditions not satisfied");
            return false;
	  }
    }

  for (auto e1 : peff)
    {

      setState.insert(e1);
      vector<string> s1 = utils::tokenize(e1,' ');
      init->insert_back(pddlTreeNode(s1[0]));
      

      
      for (int i = 1; i<s1.size();i++) {
	init->children.back().insert_back(pddlTreeNode(s1[i]));
      }
      
    }
  
  
  
}

bool doAction(string name, string parameters)
{
  
  using micro = std::chrono::microseconds;
  auto start = std::chrono::high_resolution_clock::now();
  
  //std::vector<std::string> parValues = utils::tokenize(parameters, ' ');
  //pddlTreeNode* action = root.findFirst(":action",name+".*");
  //pddlTreeNode* action = root.findFirstName(":action");
  //pddlTreeNode* r2 = action->findFirstName(":parameters");

  actionPrefab a1 = actionPrefabs[name];

  vector<string> groundedPreconditions = a1.getPreconditions(parameters);
	
	
  for (string p1: groundedPreconditions)
  {
	auto n3 = setState.find(p1);
	auto n4 = constants.find(p1);

	if ((n3==setState.end()) && (n4 == constants.end()))
	  {
	    debug_log().AddLog("preconditions not satisfied");
            return false;
	  }
	
    }
    //all preconditions met

  vector<string> grndPosEffect = a1.getPosEffects(parameters);
  vector<string> grndNegEffect = a1.getNegEffects(parameters);

	for (string e1: grndNegEffect)
	{

	    auto n3 = setState.find(e1);

			if (n3!=setState.end())
			{
	    		setState.erase(e1);

				/*
				for (auto it = init->children.begin(); it != init->children.end(); it++)
				{
					std::string s1 = it->flattenChildren();
					if ((it->data == effectName) && (s1 == effectParameters))
					{
						init->children.erase(it);
						break;
					}
					//pddlTreeNode* n3 = it->findFirstExact(effectName, effectParameters);
				}
				*/
			}
	}

	for (string e1 : grndPosEffect)
	{
		/*
	           init->insert_back(pddlTreeNode(n1.data));
	    
			string s2;
            for (pddlTreeNode n2 : n1.children) {
                string s1 = n2.data;
                for (int i = 0; i < parNames.size(); i++) {
                    utils::replaceSubstrs(s1, parNames[i], parValues[i]);
                };
                init->children.back().insert_back(pddlTreeNode(s1));
				s2.append(" ");
				s2.append(s1);
                debug_log().AddLog(s1);
            }
			*/

			setState.insert(e1);
     }

    auto finish = std::chrono::high_resolution_clock::now();

    debug_log().AddLog("time taken:%d",std::chrono::duration_cast<micro>(finish - start).count());

    return true;

}


string loadState(string fileName )
{

    ifstream file;
    file.open(string(fileName), ios::in);

    string stateString((istreambuf_iterator<char>(file)),
                            istreambuf_iterator<char>());


    string stateOut(stateString);

    string curStr;
    vector<string> tokens = vector<string>();

    utils::removeSubstrs(stateString,string("\t"));
    utils::removeSubstrs(stateString,string("\n"));
    utils::removeSubstrs(stateString,string("\r"));


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
                tokens.push_back(string(1, c));
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


    pddlTreeNode* curNode = &root;
    vector<pddlTreeNode* > stack;

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
        }
        else
        {
            curNode->insert_back(pddlTreeNode(*(t1)));
        }
    }


    pddlTreeNode* obj = root.findFirstName(":objects");


    strVec currentType;
    for (int i = 0; i< obj->children.size(); i++)
      {
	
	if (obj->children[i].data=="-")
	  {
	    objects[obj->children[i+1].data].insert(objects[obj->children[i+1].data].begin(),currentType.begin(),currentType.end());
	    currentType.clear();
	    i++;
	  }
	else
	  {
	    currentType.push_back(obj->children[i].data);
	  }
      }

    debug_log().AddLog("agents: \n");
		       
    for (string o1: objects["agent"])
      {
	debug_log().AddLog(o1);
      }
    
    vector<pddlTreeNode*> r1 =  root.search(":objects",".*");

    for (int i=-xm;i<xp;i++)
        for (int j=-ym;j<yp;j++)
        {
            char ss1[50];
            sprintf ( ss1, "loc_%d_%d", i,j );
            r1.front()->insert_back(pddlTreeNode(ss1));
        }

    init = root.findFirstName(":init");

	pddlTreeNode* constNode = root.insert_back(pddlTreeNode(":constants"));



    pddlTreeNode* cn = constNode;

    for (int i=-xm+1;i<xp-1;i++)
        for (int j=-ym+1;j<yp-1;j++)
        {
            if (path_map->walkable[path_map->at(i, j)])
            {
                for (int k=-1; k<2; k++)
                    for (int l=-1;l<2;l++)
                        if (not ((k==0)&&(l==0)))
                            if (path_map->walkable[path_map->at(i+k, j+l)])
                            {
                                pddlTreeNode tn = pddlTreeNode("con");

                                char ss1[50];
                                sprintf ( ss1, "loc_%d_%d", i,j );
                                tn.insert_back(pddlTreeNode(ss1));

                                sprintf ( ss1, "loc_%d_%d", i+k,j+l );
                                tn.insert_back(pddlTreeNode(ss1));

                                cn->insert_back(tn);
                            }

            }

        }
    return stateOut;
};


void endTurn() {


	for (auto a0 = agents.begin(); a0 != agents.end();a0++)
	{

		if (a0->second.plan.size() > 0)
		{
			action a1 = a0->second.plan.front();
			doAction(a1.name, a1.params);

			a0->second.plan.erase(a0->second.plan.begin());

			a0->second.getAgentPos(setState);

		}
		a0->second.getAgentPos(setState);
	}
}



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

        if (selected!=string("none"))
        {

            ImGui::Text((string("id:") + city[selected].id).c_str());
        }

        ImGui::SliderFloat("North dir", &(g_camera.angleNorth), -90.f, 90.f);

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


    static vector<pddlTreeNode*> s_res;

    if (ImGui::Button("Load"))
    {
        s_res  = root.search(string(str0),string(str1));
    };
    for (auto r1 : s_res){
        visitNodes(r1);
    }
    ImGui::End();


    ImGui::Begin("Test actions");

    static char actionName[128] = "move";
    ImGui::InputText("action", actionName, IM_ARRAYSIZE(actionName));

    static char actionParameters[128] = "agent0 loc_1_1 loc_2_1";
    ImGui::InputText("parameters", actionParameters, IM_ARRAYSIZE(actionParameters));


    if (ImGui::Button("Move"))
    {
        doAction(actionName,actionParameters);
    }

    if (ImGui::Button("End Turn"))
    {
      endTurn();
    }


    ImGui::End();


};


void moveAgent(string id, int dx, int dy)
{
  //ProfilerStart("nameOfProfile.log");
  
  string s1 = id+" ";
  agent a0 = agents[id];
  s1+= "loc_" + to_string(a0.x) +"_"+to_string(a0.y)+ " ";
  s1+= "loc_" + to_string(a0.x+dx) +"_"+to_string(a0.y+dy);
  doAction("move", s1);

  agents[id].getAgentPos(setState);

  //ProfilerStop();
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

    width = mode->width;
    height = mode->height;


    //width = 600;
    //height = 600;

    g_camera.m_width = width;
    g_camera.m_height = height;

    //g_camera.m_span = (xmax-xmin)/2;
    g_camera.m_span = 0.5f;

    g_camera.m_center.x = 0.8f;
    g_camera.m_center.y = 0.0f;
	//GLFWwindow* window = glfwCreateWindow(640, 480, "My Title", glfwGetPrimaryMonitor(), NULL);

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

    vector<float> gridVec = vector<float>();


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



    vector<float> outlines = getOutlines(city);
    debug_log().AddLog("%f,%f,%f,%f",outlines[0],outlines[1],outlines[2],outlines[3]);

    float* outlinesData = outlines.data();
    int outlineVerts = round(outlines.size()/2);


    shaderData outlineSh = drawBuildingOutlinesInit(outlinesData,outlineVerts);


    shaderData texSh = texQuadInit();


    shaderData quadSh = drawQuadInit();


    int x, y;


    shared_ptr<navigation_path<location_t>> path;


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
        static bool get_successors(location_t pos, vector<location_t> &successors) {
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






    for (int i = -xm; i < xp; i++)
    {
        for (int j = -ym; j < yp; j++)
        {
	  if (pnpoly(singlePolygon.nvert, singlePolygon.vertx, singlePolygon.verty, (i+0.5f)*g_camera.gridSize, (j+0.5f)*g_camera.gridSize)>0)
            {
                debug_log().AddLog("hit \n");
                path_map->walkable[path_map->at(i, j)] = false;

            }
        }
    }

    state = loadState("city.problem");
    setState = hashState(":init");
	constants = hashState(":constants");


	for (string o1 : objects["agent"])
	{
		agents.insert(pair<string, agent>(o1, agent()));
		agents[o1].id = o1;
	};


	
	static actionPrefab a1;
	a1.init(root.findFirst(":action", "move.*"));
	actionPrefabs.insert(pair<string, actionPrefab>("move", a1));
	/*
	vector<string> s1 =a1.getPreconditions("agent0 loc-1-1 loc-1-2");

	debug_log().AddLog(a1.getPreconditions("agent0 loc-1-1 loc-1-2")[0]);
	*/
    debug_log().AddLog("xm:%d,xp:%d,ym:%d,yp:%d \n", xm,xp,ym,yp);
  

    location_t dude_position {1,1};
    location_t destination {4,5};


    path = find_path<location_t, navigator>(dude_position, destination);
    if (path->success)
    {
        debug_log().AddLog("path found \n");

	location_t curPos = dude_position;
	
        for (auto p1 = path->steps.begin(); p1 != path->steps.end(); p1++){
	  if (!(curPos==*p1))
	    {
	      debug_log().AddLog("%d,%d \n", p1->x,p1->y);
	      string s1 = "agent0 ";
	      s1+= "loc_" + to_string(curPos.x) +"_"+to_string(curPos.y)+ " ";
	      s1+= "loc_" + to_string(p1->x) +"_"+to_string(p1->y);

	      curPos = *p1;
	      agents["agent0"].plan.push_back({"move", s1});
	    }
        }
    }
    
    for (auto a0 = agents.begin(); a0!=agents.end();a0++)
      {
		a0->second.getAgentPos(setState);
      };
    //agent0.getAgentPos(setState);


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
            if (selected!=string("none"))
                drawLine(lineSh, g_camera);



	    for (auto a0: agents)
	      {
		texQuadDraw(texSh,a0.second.x,a0.second.y);
	      };

            if (drawBlockedCells)
            {
                for (int i = -xm; i < xp; i++)
                {
                    for (int j = -ym; j < yp; j++)
                    {
                        if (!path_map->walkable[path_map->at(i, j)]) {
                            drawQuad(quadSh, i+0.5f, j+0.5f);
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


/*
(forall (?x)
(when (and (has ?agent ?x))
(and (not (at ?x ?from))
(at ?x to)
)
)
)
*/

