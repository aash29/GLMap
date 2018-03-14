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

#include "tinydir.h"

#include <unordered_set>
//#include <gperftools/profiler.h>

#define IM_ARRAYSIZE(_ARR)  ((int)(sizeof(_ARR)/sizeof(*_ARR)))
#define strVec vector<string>

using namespace std;


//Camera g_camera;


string curAgent = "";


GLint uniTrans;

GLFWwindow *window = NULL;
bool rightMouseDown;
glm::vec2 lastp;
glm::vec2 selp;

std::map<std::string, building> city;
std::string selected;
shaderData lineSh;

map <string, strVec> objects;
map<string, agent> agents;


polygon singlePolygon;


//нумерация клеток

int xm = 0;
int xp = 160;
int ym = 0;
int yp = 80;

rect boundingBox = {xm, xp, ym, yp};



map_t* path_map;


bool drawGrid = true;
bool drawBlockedCells = true;
bool m_showOpenDialog = true;


char* text;
int stateSize;

//agent agent0;

string controlledAgent;

map_t pathfinding_map(xm,xp,ym,yp);

heatmap_t heatmap(xm,xp,ym,yp);

heatmap_t buildingTypes(xm,xp,ym,yp); //0 - ничего, 1 - жилье, 2 - магазин, 3 - контора

int width, height;

struct navigator {

    static float get_distance_estimate(location_t &pos, location_t &goal) {
        float d = distance2d_squared(pos.x, pos.y, goal.x, goal.y);
        return d;
    }

    static bool is_goal(location_t &pos, location_t &goal) {
        return pos == goal;
        //return (std::max(abs(pos.x-goal.x),abs(pos.y-goal.y))<=1.1f);
        //return ((abs(pos.x - goal.x)<=1)&&(abs(pos.y - goal.y)<=1));
    }

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

    static float get_cost(location_t &position, location_t &successor) {
        return 1.0f;
    }
    static bool is_same_state(location_t &lhs, location_t &rhs) {
        return lhs == rhs;
    }
};

void endTurn();
void planDay(agent &a0);


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
    //glm::mat4 rotN;
    //rotN = glm::rotate(rotN, glm::radians(-angleNorth), glm::vec3(0.0f, 0.0f, 1.0f));

    std::vector<float> unCol = std::vector<float>();
    std::vector<float> unDraw = std::vector<float>();

    unCol.push_back(0.f);
    unCol.push_back(0.f);

    for (auto b1: city) {
        std::string id1 = b1.first;
        /*
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
*/
        rect r1 = b1.second.bounds;
        if ((testx>r1.xmin) && (testx<r1.xmax) && (testy>r1.ymin) && (testy<r1.ymax))
        if (pnpoly(b1.second.numVert, b1.second.vertx, b1.second.verty, testx, testy)>0)
        {
            return id1;
        }


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
            debug_log().AddLog(city[id1].type);
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


                debug_log().AddLog("name: %s \n",city[id1].name.c_str());

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

    int dx, dy;

    if (!io.WantCaptureKeyboard) {
        TESS_NOTUSED(scancode);
        TESS_NOTUSED(mods);
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
            glfwSetWindowShouldClose(window, GL_TRUE);
        if (key == GLFW_KEY_SPACE && ((action == GLFW_PRESS)||(action == GLFW_REPEAT)) ) {
            endTurn();
        }
/*
        if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS)
        {
            dx = 1, dy = 0;

            agents["agent0"].planFunc.push_back(
                    [&, dx, dy]() {
                        if (path_map->walkable[path_map->at(agents["agent0"].x + dx,agents["agent0"].y + dy)])
                        {
                            agents["agent0"].x = agents["agent0"].x + dx;
                            agents["agent0"].y = agents["agent0"].y + dy;
                            return 0;
                        }
                        else {
                            return 1;
                        };


                    });

            endTurn();
        }
        if (key == GLFW_KEY_LEFT && action == GLFW_PRESS)
        {
            dx = -1, dy = 0;

            agents["agent0"].planFunc.push_back(
                    [&, dx, dy]() {
                        if (path_map->walkable[path_map->at(agents["agent0"].x + dx,agents["agent0"].y + dy)])
                        {
                            agents["agent0"].x = agents["agent0"].x + dx;
                            agents["agent0"].y = agents["agent0"].y + dy;
                            return 0;
                        }
                        else {
                            return 1;
                        };
                    }
            );

            endTurn();
        }
        if (key == GLFW_KEY_UP && action == GLFW_PRESS)
        {
            dx = 0, dy = 1;

            agents["agent0"].planFunc.push_back(
                    [&, dx, dy]() {
                        if (path_map->walkable[path_map->at(agents["agent0"].x + dx,agents["agent0"].y + dy)])
                        {
                            agents["agent0"].x = agents["agent0"].x + dx;
                            agents["agent0"].y = agents["agent0"].y + dy;
                            return 0;
                        }
                        else {
                            return 1;
                        };
                    }
            );
            endTurn();

        }
        if (key == GLFW_KEY_DOWN && action == GLFW_PRESS)
        {
            dx = 0, dy = -1;

            agents["agent0"].planFunc.push_back(
                    [&, dx, dy]() {
                        if (path_map->walkable[path_map->at(agents["agent0"].x + dx,agents["agent0"].y + dy)])
                        {
                            agents["agent0"].x = agents["agent0"].x + dx;
                            agents["agent0"].y = agents["agent0"].y + dy;
                            return 0;
                        }
                        else {
                            return 1;
                        };
                    }
            );
            endTurn();

        }
*/
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



void endTurn() {
    for (auto a1:agents) {
        string id = a1.first;

        agents[id].update();

        if (agents[id].planFunc.size() > 0) {
            agents[id].planFunc.front()();
            agents[id].planFunc.erase(agents[id].planFunc.begin());
        } else
        {
        }
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
		ImGui::SetNextWindowPos(ImVec2(0, 0));

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

        debug_log().Draw("Log",width,height);


        ImFontAtlas* atlas = ImGui::GetIO().Fonts;
        ImFont* font = atlas->Fonts[0];
        //font->Scale = 2.f;
    }


	ImGui::SetNextWindowPos(ImVec2(width-300, 0));

    ImGui::Begin("Path");


    static int beginPos[2] = {2,2};
    ImGui::InputInt2("start", beginPos);

    static int endPos[2];
    ImGui::InputInt2("end", endPos);


	
    if (ImGui::Button("Find")) {

        for (auto a1:agents) {

            string id = a1.first;

            std::shared_ptr<navigation_path<location_t>> path;

            location_t bpLoc(beginPos[0], beginPos[1]);

            location_t epLoc(endPos[0], endPos[1]);

            location_t apLoc(a1.second.x,a1.second.y);

            path = find_path<location_t, navigator>(bpLoc, epLoc);

            if (path->success) {
                debug_log().AddLog("path found \n");

                location_t curPos = apLoc;

                for (auto p1 = path->steps.begin(); p1 != path->steps.end(); p1++) {
                    if (!(curPos == *p1)) {
                        debug_log().AddLog("%d,%d \n", p1->x, p1->y);

                        int dx = p1->x;
                        int dy = p1->y;
                        agents[id].planFunc.push_back(
                                [&, dx, dy, id]() {
                                    if (path_map->walkable[path_map->at(dx, dy)]) {
                                        agents[id].x = dx;
                                        agents[id].y = dy;
                                        return 0;
                                    } else {
                                        return 1;
                                    };
                                }
                        );
                        curPos = *p1;
                    }
                }

            }
        }

    };



    ImGui::End();
	
	/*

    ImGui::Begin("State");


    //static char* text = &state[0];
    
    //ImGui::InputTextMultiline("##source", text, stateSize*2 , ImVec2(-1.0f, ImGui::GetTextLineHeight() * 16), ImGuiInputTextFlags_AllowTabInput);



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
    for (auto r1 : s_res){
        visitNodes(r1);
    }
    ImGui::End();
	*/	
	
    ImGui::Begin("Test actions");
	/*
    static char actionName[128] = "move";
    ImGui::InputText("action", actionName, IM_ARRAYSIZE(actionName));

    static char actionParameters[128] = "agent0 loc_1_1 loc_2_1";
    ImGui::InputText("parameters", actionParameters, IM_ARRAYSIZE(actionParameters));


    if (ImGui::Button("Move"))
    {
        doAction(actionName,actionParameters);
    }
	*/


	if (ImGui::Button("Plan Day"))
	{
        for (auto a0: agents)
		planDay(agents[a0.first]);
	}



    if (ImGui::Button("End Turn"))
    {
        endTurn();
    }


    ImGui::End();



    ImGui::Begin("Agent");
    {

        if (curAgent!="") {


            ImGui::LabelText( "id", "%s", agents[curAgent].id.c_str() );

            ImGui::InputInt("heat", &agents[curAgent].heat);

            ImGui::InputInt("energy", &agents[curAgent].energy);

            ImGui::InputInt("fed", &agents[curAgent].fed);

            const char *listbox_items[20];

            static int listbox_item_current = 1;

            int i = 0;
            for (string s1: agents[curAgent].inventory) {
                listbox_items[i] = s1.c_str();
                i++;
            }

            ImGui::ListBox("inventory", &listbox_item_current, listbox_items, i, 4);

        }

    }
    ImGui::End();

	

    if (m_showOpenDialog) {
        ImGui::SetNextWindowSize(ImVec2(500, 440), ImGuiSetCond_FirstUseEver);

        if (ImGui::Begin("Open level", &m_showOpenDialog)) {

            // left
            static char selected[100] = "";
            ImGui::BeginChild("left pane", ImVec2(150, 0), true);
            tinydir_dir dir;
            if (tinydir_open(&dir, "./maps/") != -1) {
                tinydir_file file;
                int i = 0;

                while (dir.has_next) {
                    if (tinydir_readfile(&dir, &file) != -1) {
                        //ImGui::TextWrapped(file.name);
                        //coinsLog.AddLog(file.extension, "\n");
                        if (!strcmp(file.extension, "txt")) {
                            if (ImGui::Selectable(file.name, !strcmp(selected, file.name)))
                                strcpy(selected, file.name);
                        }
                    }
                    tinydir_next(&dir);
                    i++;
                }
                tinydir_close(&dir);
            }

            ImGui::EndChild();
            ImGui::SameLine();

            // right
            ImGui::BeginGroup();
            ImGui::BeginChild("item view", ImVec2(0, -ImGui::GetItemsLineHeightWithSpacing())); // Leave room for 1 line below us
            ImGui::Text("Selected level: %s", selected);
            ImGui::Separator();

            char pathToFile[100] = "";
            strcat(pathToFile, "./maps/");
            strcat(pathToFile, selected);

            std::ifstream t(pathToFile);
            std::stringstream buffer;
            buffer << t.rdbuf();

            static char bstr[5000];



            strcpy(bstr,buffer.str().c_str());

            //coinsLog.AddLog(buffer.str().c_str());
            ImGui::TextWrapped(bstr);


            /*
              if (tinydir_open(&dir, "../") != -1) {
              tinydir_file file;
              while (dir.has_next)
              {
              if (tinydir_readfile(&dir, &file) != -1) {
              ImGui::TextWrapped(file.name);
              }
              tinydir_next(&dir);
              }
              }
            */


            ImGui::EndChild();
            ImGui::BeginChild("buttons");
            if (ImGui::Button("Load"))
            {
                //city = loadLevel(selected, tess, boundingBox, singlePolygon);
            };
            ImGui::SameLine();
            ImGui::EndChild();
            ImGui::EndGroup();
        }
        ImGui::End();
    }




};

void planDay(agent &a0){

	struct navShop : navigator
	{
		static bool is_goal(location_t &pos, location_t &goal) {
			// return (pos==goal);
			// std::string id1 = selectBuilding(pos.x, pos.y);
			return 	(buildingTypes.deltaHeat[buildingTypes.at(pos.x,pos.y)] == 2);
		}
		static float get_distance_estimate(location_t &pos, location_t &goal) {
			float d = 1.f;
			return d;
		}

	};

    a0.planFunc.reserve(300);

    std::function<int()> goToAnyShop = [&a0]() {
        auto shopPath = find_path<location_t, navShop>(location_t(a0.x, a0.y), location_t(0, 0));

        if (shopPath->success) {
            debug_log().AddLog("path found \n");

            location_t curPos = location_t(a0.x, a0.y);

            auto it = a0.planFunc.begin();

            for (auto p1 = shopPath->steps.begin(); p1 != shopPath->steps.end(); p1++) {
                if (!(curPos == *p1)) {
                    debug_log().AddLog("%d,%d \n", p1->x, p1->y);
                    int dx = p1->x;
                    int dy = p1->y;

                    it = a0.planFunc.insert(it+1,[dx, dy, &a0](){
                                           if (path_map->walkable[path_map->at(dx, dy)]) {
                                               a0.x = dx;
                                               a0.y = dy;
                                               a0.energy--;
                                               return 0;
                                           } else {
                                               return 1;
                                           };
                                       }
                    );
                    curPos = *p1;
                }
            }
        }
      return 1;
    };


    std::function<int()> eat = [&a0]() {

        auto it = a0.inventory.find("food");

        if (it!=a0.inventory.end()) {
            a0.fed = a0.fed + 20;
            a0.inventory.erase(it);
            return 0;
        } else {
            return 1;
        }


    };

    std::function<int()> goHome = [&a0]() {

        location_t target = location_t(static_cast<int>(city[a0.home].coords[0][0]), static_cast<int>(city[a0.home].coords[0][1]));

        auto homePath = find_path<location_t, navigator>(location_t(a0.x, a0.y), target);

        if (homePath->success) {
            debug_log().AddLog("path found \n");

            location_t curPos = location_t(a0.x, a0.y);

            auto it = a0.planFunc.begin();

            for (auto p1 = homePath ->steps.begin(); p1 != homePath ->steps.end(); p1++) {
                if (!(curPos == *p1)) {
                    debug_log().AddLog("%d,%d \n", p1->x, p1->y);
                    int dx = p1->x;
                    int dy = p1->y;

                    it = a0.planFunc.insert(it+1,
                                            [ dx, dy, &a0]() {
                                                if (path_map->walkable[path_map->at(dx, dy)]) {
                                                    a0.x = dx;
                                                    a0.y = dy;
                                                    a0.energy--;
                                                    return 0;
                                                } else {
                                                    return 1;
                                                };
                                            }
                    );
                    curPos = *p1;
                }
            }
        }
        return 1;
    };


    a0.planFunc.push_back(goToAnyShop);
    a0.planFunc.push_back(goHome);
    a0.planFunc.push_back(eat);

}



int main(int argc, char *argv[])
{
    //GLFWwindow* window;
    const GLFWvidmode* mode;

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


    glfwSetTime(0);



    if (argc == 1)
    {
        m_showOpenDialog = true;

        while (m_showOpenDialog)
        {

            glfwPollEvents();

            ImGui_ImplGlfwGL3_NewFrame();


            glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);


            ImGui::SetNextWindowSize(ImVec2(500, 440), ImGuiCond_Always);

            if (ImGui::Begin("Open level", &m_showOpenDialog)) {

                // left
                static char selected[100] = "";
                ImGui::BeginChild("left pane", ImVec2(150, 0), true);
                tinydir_dir dir;
                if (tinydir_open(&dir, "./maps/") != -1) {
                    tinydir_file file;
                    int i = 0;

                    while (dir.has_next) {
                        if (tinydir_readfile(&dir, &file) != -1) {
                            //ImGui::TextWrapped(file.name);
                            //coinsLog.AddLog(file.extension, "\n");
                            if (!strcmp(file.extension, "txt")) {
                                if (ImGui::Selectable(file.name, !strcmp(selected, file.name)))
                                    strcpy(selected, file.name);
                            }
                        }
                        tinydir_next(&dir);
                        i++;
                    }
                    tinydir_close(&dir);
                }

                ImGui::EndChild();
                ImGui::SameLine();

                // right
                ImGui::BeginGroup();
                ImGui::BeginChild("item view", ImVec2(0, -ImGui::GetItemsLineHeightWithSpacing())); // Leave room for 1 line below us
                ImGui::Text("Selected level: %s", selected);
                ImGui::Separator();

                char pathToFile[100] = "";
                strcat(pathToFile, "./maps/");
                strcat(pathToFile, selected);

                std::ifstream t(pathToFile);
                std::stringstream buffer;
                buffer << t.rdbuf();

                static char bstr[5000];



                strcpy(bstr,buffer.str().c_str());

                //coinsLog.AddLog(buffer.str().c_str());
                ImGui::TextWrapped(bstr);


                ImGui::EndChild();
                ImGui::BeginChild("buttons");
                if (ImGui::Button("Load"))
                {
                    printf(selected);
                    city = loadLevel(pathToFile, tess, boundingBox, singlePolygon);
                    m_showOpenDialog = false;

                };
                ImGui::SameLine();
                ImGui::EndChild();
                ImGui::EndGroup();
            }
            ImGui::End();

            ImGui::Render();

            glfwSwapBuffers(window);
            glfwPollEvents();
        }
    }

    else
        city = loadLevel(argv[1], tess, boundingBox, singlePolygon);

    printf("go...\n");

    if (!tessTesselate(tess, TESS_WINDING_POSITIVE, TESS_POLYGONS, nvp, 2, 0))
        return -1;
    printf("Memory used: %.1f kB\n", allocated/1024.0f);



    const float* verts = tessGetVertices(tess);
    const int* vinds = tessGetVertexIndices(tess);
    const int* elems = tessGetElements(tess);
    const int nverts = tessGetVertexCount(tess);
    const int nelems = tessGetElementCount(tess);




    g_camera.m_width = width;
    g_camera.m_height = height;

    g_camera.m_span = (boundingBox.xmax-boundingBox.xmin)/2;
    //g_camera.m_span = 0.5f;

    g_camera.m_center.x = (boundingBox.xmax + boundingBox.xmin) / 2;
    g_camera.m_center.y = (boundingBox.ymax + boundingBox.ymin) / 2;


    //GLuint shaderProgram =  initModernOpenGL( verts,  nverts, elems,  nelems );


    shaderData mapSh =  drawMapShaderInit(verts, nverts, elems, nelems);
    float stub[4] = {0.f,0.f,0.f,0.f};
    lineSh = drawLineShaderInit(stub, 2);

    std::vector<float> gridVec = std::vector<float>();


    debug_log().AddLog("bb: %g,%g,%g,%g \n",boundingBox.xmin, boundingBox.xmax, boundingBox.ymin ,boundingBox.ymax);
    /*
    xm = 0;
    xp = 0;
    ym = 0;
    yp = 0;
    */

    for (float x=0; x < boundingBox.xmax; x=x+g_camera.gridSize)
    {
        gridVec.push_back(x + g_camera.gridSize/2);
        gridVec.push_back(boundingBox.ymin);
        gridVec.push_back(x + g_camera.gridSize / 2);
        gridVec.push_back(boundingBox.ymax);
        //xp++;
    }

    for (float x=0; x > boundingBox.xmin; x=x-g_camera.gridSize)
    {
        gridVec.push_back(x - g_camera.gridSize / 2);
        gridVec.push_back(boundingBox.ymin);
        gridVec.push_back(x - g_camera.gridSize / 2);
        gridVec.push_back(boundingBox.ymax);
        //xm++;
    }

    for (float y=0; y < boundingBox.ymax; y=y+g_camera.gridSize)
    {
        gridVec.push_back(boundingBox.xmin);
        gridVec.push_back(y + g_camera.gridSize / 2);
        gridVec.push_back(boundingBox.xmax);
        gridVec.push_back(y + g_camera.gridSize / 2);
        //yp++;
    }


    for (float y=0; y > boundingBox.ymin; y=y-g_camera.gridSize)
    {
        gridVec.push_back(boundingBox.xmin);
        gridVec.push_back(y - g_camera.gridSize / 2);
        gridVec.push_back(boundingBox.xmax);
        gridVec.push_back(y - g_camera.gridSize / 2);
        //ym++;
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

    /*
    for (string o1: objects["agents"])
      {
    agents.insert(std::pair<string, agent>(o1,agent()));
        agents[o1].id = o1;
      };
    */



    agents.clear();
    for (auto b1 : city) {
        if (b1.second.type=="dwelling") {
            int x = b1.second.coords[0][0];
            int y = b1.second.coords[0][1];

            string id = "agent" + b1.second.id;

            debug_log().AddLog("\n");

            debug_log().AddLog("agent %s created \n", id.c_str());

            agents.insert(std::pair<string, agent>(id, agent()));
            agents[id].id = id;
            agents[id].x = x;
            agents[id].y = y;
            agents[id].home = b1.second.id;

        }
    }

    curAgent = agents.begin()->first;

    debug_log().AddLog("\n");
    debug_log().AddLog("agents count: %d", agents.size());
    debug_log().AddLog("\n");
    debug_log().AddLog(curAgent.c_str());
    debug_log().AddLog("\n");



/*
    agents.insert(std::pair<string, agent>("agent0", agent()));

    agents["agent0"].id = "agent0";
    agents["agent0"].x = 2;
    agents["agent0"].y = 2;
    agents["agent0"].home = "relation/3084726";
*/



    std::shared_ptr<navigation_path<location_t>> path;

    //pathfinding_map = map_t(-xm, xp, -ym, yp);

    path_map = &pathfinding_map;


    for (int i = -xm; i < xp; i++) {
        for (int j = -ym; j < yp; j++) {

            buildingTypes.deltaHeat[buildingTypes.at(i,j)] = 0;

            string id = selectBuilding((i+0.5f)*g_camera.gridSize, (j+0.5f)*g_camera.gridSize);

            if (id!="none"){
                path_map->walkable[path_map->at(i,j)] = false;
                heatmap.deltaHeat[heatmap.at(i,j)] = 1;
                if (city[id].type=="dwelling") {
                    buildingTypes.deltaHeat[buildingTypes.at(i, j)] = 1;
                    path_map->walkable[path_map->at(i,j)] = true;
                }
                if (city[id].type == "shop") {
                    buildingTypes.deltaHeat[buildingTypes.at(i, j)] = 2;
                    path_map->walkable[path_map->at(i,j)] = true;
                }
                if (city[id].type == "office") {
                    buildingTypes.deltaHeat[buildingTypes.at(i, j)] = 3;
                }
            }
        }
    }


    for (auto a0:agents){
        string id = a0.first;
        agents[id].effects.push_back([&heatmap,id]{
            agents[id].heat=agents[id].heat+heatmap.deltaHeat[heatmap.at(agents[id].x,agents[id].y)];
            agents[id].heat = std::max(0,agents[id].heat);
            agents[id].heat = std::min(100,agents[id].heat);

            agents[id].fed--;

            return 0;
        });
    }



    //agent0.getAgentPos(setState);

    /*
    agent0.planFunc.push_back(
                  [&](){
                agent0.x++;
                return 0;
                  });
                  */

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
                drawLine(gridSh,g_camera, 0.f, 0.f, 1.f);

            drawMap(mapSh, g_camera);
            drawBuildingOutlines( outlineSh, g_camera);
            
			if (selected!=std::string("none"))
                drawLine(lineSh, g_camera,0.f,0.f,1.f);

			
			for (auto b1 : city) {
				string id1 = b1.first;
				
				std::vector<float> unDraw = std::vector<float>();

				for (auto it : city[id1].coords)
				{
					unDraw.push_back(it[0]);
					unDraw.push_back(it[1]);
					for (int i = 2; i < it.size() - 1; i = i + 2)
					{
						unDraw.push_back(it[i]);
						unDraw.push_back(it[i + 1]);

						unDraw.push_back(it[i]);
						unDraw.push_back(it[i + 1]);
					}
					unDraw.push_back(it[0]);
					unDraw.push_back(it[1]);
				};

				lineSh.vertexCount = round(unDraw.size() / 2);
				lineSh.data = unDraw.data();
				glBindBuffer(GL_ARRAY_BUFFER, lineSh.vbo);
				glBufferData(GL_ARRAY_BUFFER, lineSh.vertexCount * 2 * sizeof(float), lineSh.data, GL_STATIC_DRAW);

				if (b1.second.type=="shop"){
					drawLine(lineSh, g_camera, 1.f, 0.f, 0.f);
				}

				if (b1.second.type == "dwelling") {
					drawLine(lineSh, g_camera, 0.f, 1.f, 0.f);
				}

			}


            for (auto a0: agents)
            {
                texQuadDraw(texSh,a0.second.x,a0.second.y);
            };
            /*
                  for (std::string s1 : agents){

                    getAgentPos(state, s1, x, y);


                    texQuadDraw(texSh,x,y);
                  }
                  */


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

