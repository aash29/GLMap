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
//#include "camera.hpp"
//#include "graphics.hpp"
#include "imgui.h"
#include "imgui_impl_glfw_gl3.h"
#include "appLog.h"
#include "map.hpp"

#include "tesselator.h"
#include "path_impl.hpp"
#include "pathnode.hpp"


//#include "st_tree.h"

#include "pddltree.hpp"
#include "utils.hpp"
#include "agent.h"

#include "tinydir.h"

#include <unordered_set>
//#include <gklib_defs.h>
//#include <gperftools/profiler.h>

#include "Box2D/Box2D.h"
#include "DebugDraw.h"

#include "entity.h"


#define IM_ARRAYSIZE(_ARR)  ((int)(sizeof(_ARR)/sizeof(*_ARR)))
#define strVec vector<string>

using namespace std;


enum mode {AgentControl, TraingleInspect};


string curAgent = "";


GLint uniTrans;

GLFWwindow *window = NULL;
bool rightMouseDown;
b2Vec2 lastp;
b2Vec2 selp;

map<string, building> city;
string selected;
//shaderData lineSh;

map <string, strVec> objects;
map<string, agent> agents;

polygon singlePolygon;


//нумерация клеток

int xm;
int xp;
int ym;
int yp;

rect boundingBox;


map_t* path_map;


bool drawGrid = true;
bool drawBlockedCells = true;
bool drawPaths = true;
bool cameraFollow = true;
bool drawFOV = false;


bool computeBounds = false;


bool m_showOpenDialog = true;


char* text;
int stateSize;

//agent agent0;

string controlledAgent;

map_t* pathfinding_map;

heatmap_t* heatmap;

heatmap_t* buildingTypes; //0 - ничего, 1 - жилье, 2 - магазин, 3 - контора

int width, height;

int absoluteTurn = 0;

double* vertx;
double* verty;
int* vertNumber;
int nElemsTriangSelect;

b2World world(b2Vec2_zero);

b2Body* agentBody;
entity playerAgent;

int agentCollisionMarker = 0;
int POImarker = 1;
float sight = 10.f;


//const int16	visibilityGroup = 1;
//const int16	buildingsGroup = 2;

const uint16 visibilityCategory = 0x0001;
const uint16 buildingsCategory = 0x0002;


class RayCastClosestCallback : public b2RayCastCallback {
public:
    RayCastClosestCallback()
    {
        m_hit = false;
    }

    float32 ReportFixture(b2Fixture* fixture, const b2Vec2& point, const b2Vec2& normal, float32 fraction) override
    {
        b2Body* body = fixture->GetBody();
        void* userData = body->GetUserData();
        if (userData)
        {
            int32 index = *(int32*)userData;
            if ((index == 0) || (index == 1))
            {
                // By returning -1, we instruct the calling code to ignore this fixture and
                // continue the ray-cast to the next fixture.
                return -1.0f;
            }
        }

        m_hit = true;
        m_point = point;
        m_normal = normal;

        // By returning the current fraction, we instruct the calling code to clip the ray and
        // continue the ray-cast to the next fixture. WARNING: do not assume that fixtures
        // are reported in order. However, by clipping, we can always get the closest fixture.
        return fraction;
    }

    bool m_hit;
    b2Vec2 m_point;
    b2Vec2 m_normal;
};

/*
class sensorContactListener: public b2ContactListener {
	void BeginContact(b2Contact* contact) {
		b2Fixture* f1 = contact->GetFixtureA();

		void* userData = f1->GetUserData();
        entity* t1 = NULL;

		if (userData)
		{
			t1 = (entity*)userData;
			//debug_log().AddLog("collision with id: %d \n", index);
		}


		b2Fixture* f2 = contact->GetFixtureB();
		userData = f2->GetUserData();
        //int32 ud2 = -1;
        entity* t2 = NULL;
		if (userData)
		{
            t2 = (entity*)userData;
			//debug_log().AddLog("collision with id: %d \n", index);
		}

        if ((f1->GetFilterData().categoryBits==visibilityCategory) && (f2->GetFilterData().categoryBits==visibilityCategory)) {


            if ((t1->type == POI) && (t2->type == player)) {
                std::swap(t1, t2);
                std::swap(f1, f2);

            }

            if ((t1->type == player) && (t2->type == POI)) {

                t2->descriptionArmed = true;
            }
        }

	}


};

*/

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

        if (pathfinding_map->walkable[pathfinding_map->at(pos.x - 1, pos.y - 1)]) successors.push_back(location_t(pos.x - 1, pos.y - 1));
        if (pathfinding_map->walkable[pathfinding_map->at(pos.x, pos.y - 1)]) successors.push_back(location_t(pos.x, pos.y - 1));
        if (pathfinding_map->walkable[pathfinding_map->at(pos.x + 1, pos.y - 1)]) successors.push_back(location_t(pos.x + 1, pos.y - 1));

        if (pathfinding_map->walkable[pathfinding_map->at(pos.x - 1, pos.y)]) successors.push_back(location_t(pos.x - 1, pos.y));
        if (pathfinding_map->walkable[pathfinding_map->at(pos.x + 1, pos.y)]) successors.push_back(location_t(pos.x + 1, pos.y));

        if (pathfinding_map->walkable[pathfinding_map->at(pos.x - 1, pos.y + 1)]) successors.push_back(location_t(pos.x - 1, pos.y + 1));
        if (pathfinding_map->walkable[pathfinding_map->at(pos.x, pos.y + 1)]) successors.push_back(location_t(pos.x, pos.y + 1));
        if (pathfinding_map->walkable[pathfinding_map->at(pos.x + 1, pos.y + 1)]) successors.push_back(location_t(pos.x + 1, pos.y + 1));
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

int pairingNum(int a, int b) {
    if (a>b){
        int a1 = a;
        a = b;
        b = a1;
    }

    return (a+b)*(a+b+1)/2 + b;
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
    b2Vec2 ps((float) xd, (float) yd);
    selp = g_camera.ConvertScreenToWorld(ps);

    ImGuiIO &io = ImGui::GetIO();

    //std::cout<<"pressed" << "\n";


	struct navigator_graph {

		static float get_distance_estimate(pathNode &pos, pathNode &goal) {
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

			if (pathfinding_map->walkable[pathfinding_map->at(pos.x - 1, pos.y - 1)]) successors.push_back(location_t(pos.x - 1, pos.y - 1));

			return true;
		}

		static float get_cost(location_t &position, location_t &successor) {
			return 1.0f;
		}
		static bool is_same_state(location_t &lhs, location_t &rhs) {
			return lhs == rhs;
		}
	};



    if (!io.WantCaptureMouse)
    {

        // Use the mouse to move things around.
		if (button == GLFW_MOUSE_BUTTON_1) {

			for (int i = 0; i < nElemsTriangSelect; i++) {

				if (pnpoly(3, &vertx[i * 3], &verty[i * 3], selp.x, selp.y) > 0)
				{

					debug_log().AddLog("hit %d \n", vertNumber[i]);
				}


			}



			/*
			
			std::string id1 = selectBuilding(selp.x,selp.y);
            debug_log().AddLog(id1.c_str());
            debug_log().AddLog(city[id1].type);
            debug_log().AddLog("\n");


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


                debug_log().AddLog("name: %s \n",city[id1].name.c_str());

                lineSh.vertexCount = round(unDraw.size()/2);
                lineSh.data = unDraw.data();
                glBindBuffer(GL_ARRAY_BUFFER, lineSh.vbo);
                glBufferData(GL_ARRAY_BUFFER, lineSh.vertexCount*2*sizeof(float), lineSh.data, GL_STATIC_DRAW);

                //lineSh = drawLineShaderInit(unDraw.data(), round(unDraw.size()/2));
            }
            else {
                selected=string("none");
            };
<<<<<<< HEAD
            std::cout << id1 <<"\n";
			*/


           //cout << id1 <<"\n";

        }
        else if (button == GLFW_MOUSE_BUTTON_2) {
            if (action == GLFW_PRESS) {
                lastp = g_camera.ConvertScreenToWorld(ps);
                rightMouseDown = true;
                b2Vec2 pw = g_camera.ConvertScreenToWorld(ps);
                //test->RightMouseDown(pw);
            }

            if (action == GLFW_RELEASE) {
                rightMouseDown = false;
            }
        }
    }
}


static void sMouseMotion(GLFWwindow *, double xd, double yd) {
    b2Vec2 ps((float) xd, (float) yd);

    b2Vec2 pw = g_camera.ConvertScreenToWorld(ps);
    //test->MouseMove(pw);
    if (rightMouseDown) {
        b2Vec2 diff = pw - lastp;
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
		//TESS_NOTUSED(scancode);
		//TESS_NOTUSED(mods);
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
			glfwSetWindowShouldClose(window, GL_TRUE);
		if (key == GLFW_KEY_SPACE && ((action == GLFW_PRESS) || (action == GLFW_REPEAT))) {
			endTurn();
		}




/*
        agent* agent0 = &agents.begin()->second;


        if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS) {


            
            dx = 1, dy = 0;

            agent0->planFunc.push_back(
                    [&, dx, dy]() {
                        if (path_map->walkable[path_map->at(agent0->x + dx,agent0->y + dy)])
                        {
                            agent0->x = agent0->x + dx;
                            agent0->y = agent0->y + dy;
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
            agent0->planFunc.push_back(
                    [&, dx, dy]() {
                        if (path_map->walkable[path_map->at(agent0->x + dx,agent0->y + dy)])
                        {
                            agent0->x = agent0->x + dx;
                            agent0->y = agent0->y + dy;
                            return 0;
                        }
                        else {
                            return 1;
                        };


                    });

            endTurn();
             
        }
        if (key == GLFW_KEY_UP && action == GLFW_PRESS) {



            
            dx = 0, dy = 1;

            agent0->planFunc.push_back(
                    [&, dx, dy]() {
                        if (path_map->walkable[path_map->at(agent0->x + dx,agent0->y + dy)])
                        {
                            agent0->x = agent0->x + dx;
                            agent0->y = agent0->y + dy;
                            return 0;
                        }
                        else {
                            return 1;
                        };


                    });

            endTurn();
            

        }
        if (key == GLFW_KEY_DOWN && action == GLFW_PRESS) {
            
            dx = 0, dy = -1;

            agent0->planFunc.push_back(
                    [&, dx, dy]() {
                        if (path_map->walkable[path_map->at(agent0->x + dx,agent0->y + dy)])
                        {
                            agent0->x = agent0->x + dx;
                            agent0->y = agent0->y + dy;
                            return 0;
                        }
                        else {
                            return 1;
                        };


                    });
            endTurn();
            

        } */
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
        if (absoluteTurn%24 ==0){
            if (id != agents.begin()->first)
                planDay(agents[id]);
        }
    }
    absoluteTurn++;
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
        ImGui::SetNextWindowSize(ImVec2(0.f, 0.f));

        ImGui::Begin("Info");

        //static int absoluteTurn;
        int currentTurn = absoluteTurn % 24;

        ImGui::Text("Current turn: %d", currentTurn);
        ImGui::Text("Absolute turn: %d", absoluteTurn);

        b2Vec2 ps = b2Vec2(io.MousePos.x, io.MousePos.y);
        b2Vec2 pw = g_camera.ConvertScreenToWorld(ps);

        ImGui::Text("Mouse pos: (%f, %f)", pw.x, pw.y);
        //ImGui::Text("Current cell: (%f, %f)", floor(pw.x / g_camera.gridSize), floor(pw.y / g_camera.gridSize));

        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

        if (selected!=std::string("none"))
        {
            ImGui::Text((std::string("id:") + city[selected].id).c_str());
        }

        //ImGui::SliderFloat("North dir", &angleNorth, -90.f, 90.f);
        //ImGui::SliderFloat("aspect ratio", &geoRatio, 0.3f, 2.f);

        //ImGui::Checkbox("Draw Blocked Cells", &drawBlockedCells);
        //ImGui::Checkbox("Draw Grid", &drawGrid);
		ImGui::Checkbox("Draw Paths", &drawPaths);
        ImGui::Checkbox("Camera Follow", &cameraFollow);
        ImGui::Checkbox("Draw FOV", &drawFOV);

        ImGui::End();

        ImGui::PopStyleColor();


        ImGui::ShowTestWindow();

        debug_log().Draw("Log",width,height);


        ImFontAtlas* atlas = ImGui::GetIO().Fonts;
        ImFont* font = atlas->Fonts[0];
        //font->Scale = 2.f;





    }






    /*

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
	*/
	/*

    ImGui::Begin("State");


    //static char* text = &state[0];
    
    //ImGui::InputTextMultiline("##source", text, stateSize*2 , ImVec2(-1.0f, ImGui::GetTextLineHeight() * 16), ImGuiInputTextFlags_AllowTabInput);





	
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


    ImGui::Begin("Actions");

	if (ImGui::Button("Plan Day"))
	{
        for (map<string, agent>::iterator a0 = std::next(agents.begin()); a0!=agents.end(); a0++ )
        {        //for (auto a0: agents){
            planDay(agents[a0->first]);
        }
	}



    if (ImGui::Button("End Turn"))
    {
        endTurn();
    }


    ImGui::End();



    ImGui::Begin("Agent");
    {

        if (curAgent!="") {

			ImGui::InputInt("heat", &(agents[curAgent].heat));

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
			return 	(buildingTypes->deltaHeat[buildingTypes->at(pos.x,pos.y)] == 2);
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



int main(int argc, char *argv[])
{
    //GLFWwindow* window;
    const GLFWvidmode* mode;

    float t = 0.0f, pt = 0.0f;


    TESSalloc ma;
    TESStesselator* tess = 0;

    const int nvp = 3;
    //unsigned char* vflags = 0;
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

    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
	

    mode = glfwGetVideoMode(glfwGetPrimaryMonitor());

    width = mode->width;
    height = mode->height;


	g_camera.m_width = width;
	g_camera.m_height = height;




    window = glfwCreateWindow(width, height, "logistics", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    //glfwSwapInterval(1); // Enable vsync

	glfwSetInputMode(window, GLFW_STICKY_KEYS, 1);
    glfwSetScrollCallback(window, sScrollCallback);
    glfwSetCursorPosCallback(window, sMouseMotion);
    glfwSetMouseButtonCallback(window, sMouseButton);
    glfwSetKeyCallback(window, key);
    glfwSetCharCallback(window, charCallback);

    glewExperimental = GL_TRUE;
    //glewInit();


	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
		exit(EXIT_FAILURE);
	}

    ImGui_ImplGlfwGL3_Init(window, false);


    glfwSetTime(0);

    char* levelPath = "little.geojson";


	xm = 0;
	ym = 0;
	xp = 100;
	yp = 50;


    if (argc == 1) {
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
                            //if (!strcmp(file.extension, "txt")) {
                                if (ImGui::Selectable(file.name, !strcmp(selected, file.name)))
                                    strcpy(selected, file.name);
                            //}
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
                ImGui::Checkbox("Compute bounds", &computeBounds);
                ImGui::Text("Selected level: %s", selected);
                ImGui::Separator();

                char pathToFile[100] = "";
                strcat(pathToFile, "./maps/");
                strcat(pathToFile, selected);
                ImGui::EndChild();
                ImGui::BeginChild("buttons");
                if (ImGui::Button("Load"))
                {
                    printf(selected);
                    //city = loadLevel(pathToFile, tess, boundingBox, singlePolygon);
                    //city = loadLevel(pathToFile, tess2, boundingBox, singlePolygon);
                    levelPath = pathToFile;

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

    else {

        levelPath = argv[1];
        //city = loadLevel(argv[1], tess, boundingBox, singlePolygon);
        //city = loadLevel(argv[1], tess2, boundingBox, singlePolygon);


    }

    printf("go...\n");
	//debug_log().AddLog(u8"ляляля");

    //loadGrid(levelPath, xp,yp);

    boundingBox.xmin=xm;
    boundingBox.xmax=xp;
    boundingBox.ymin=ym;
    boundingBox.ymax=yp;


    pathways roads = loadLevel(levelPath, tess, boundingBox, &world, computeBounds);




    tessSetOption(tess, TESS_CONSTRAINED_DELAUNAY_TRIANGULATION, 1);
    if (!tessTesselate(tess, TESS_WINDING_POSITIVE, TESS_POLYGONS, nvp, 2, 0))
        return -1;
    printf("Memory used: %.1f kB\n", allocated/1024.0f);




    const float* verts = tessGetVertices(tess);
    const int* vinds = tessGetVertexIndices(tess);
    const int* elems = tessGetElements(tess);
    const int nverts = tessGetVertexCount(tess);
    const int nelems = tessGetElementCount(tess);

	tess = tessNewTess(&ma);
	roads = loadLevel(levelPath, tess, boundingBox, &world, computeBounds);

	if (!tessTesselate(tess, TESS_WINDING_POSITIVE, TESS_BOUNDARY_CONTOURS, nvp, 2, 0))
		return -1;


    b2BodyDef bd;
    b2Body* ground = world.CreateBody(&bd);

    ground->SetActive(true);

	int vertexSize = 2;
	const int nelemsCont = tessGetElementCount(tess);
	const TESSindex* elemsCont = tessGetElements(tess);
	const float* vertsCont = tessGetVertices(tess);
	for (int i = 0; i < nelemsCont; i++) {
		const TESSindex base = elemsCont[i * 2];
		const TESSindex count = elemsCont[i * 2 + 1];

		{
			b2Vec2* vs;
			vs = new b2Vec2[count];

            int c = 0;
			for (int j = 0; j < count; j++) {
				//&verts[(base + j) * vertexSize]
                if (j>0){
                    b2Vec2 v0 (vertsCont[(base + c) * vertexSize], vertsCont[(base + c) * vertexSize+1]);
                    if (b2DistanceSquared(v0, vs[c-1]) > b2_linearSlop * b2_linearSlop ){
                        vs[c].Set(vertsCont[(base + c) * vertexSize], vertsCont[(base + c) * vertexSize+1]);
                        c++;
                    }

                }

			}

            if (c>=3) {
                b2ChainShape shape;
                shape.CreateLoop(vs, c);
                b2FixtureDef fd;
                fd.shape = &shape;
                //fd.filter.categoryBits = buildingsCategory;
                //fd.filter.maskBits = buildingsCategory;

                ground->CreateFixture(&fd);
            }
		}
	}



    b2Vec2* linesDataStore = new b2Vec2[500000];
	b2Color* linesColorStore = new b2Color[500000];

	//std::fill(linesColorStore, linesColorStore + 4 * 500000, 1.f);

	int vertexCount = 0;
	for (auto const& n1 : roads.pathGraph)
	{
		for (int neigh1 = 0; neigh1 < n1.second.size(); neigh1++) {


			float x0 = roads.nodes[n1.first].x;
			float y0 = roads.nodes[n1.first].y;
            linesDataStore[vertexCount] = b2Vec2(x0,y0);
            linesColorStore[vertexCount] = b2Color (1.f,1.f,1.f,1.f);
			vertexCount++;


			float x1 = roads.nodes[n1.second[neigh1]].x;
			float y1 = roads.nodes[n1.second[neigh1]].y;
            linesDataStore[vertexCount] = b2Vec2(x1,y1);
            linesColorStore[vertexCount] = b2Color (1.f,1.f,1.f,1.f);

			vertexCount++;
		}
	}

	g_debugDraw.Create();

	g_debugDraw.SetFlags(b2Draw::e_shapeBit);

    world.SetDebugDraw(&g_debugDraw);


	// Define the dynamic body. We set its position and call the body factory.
	b2BodyDef bodyDef;
	bodyDef.type = b2_dynamicBody;
	bodyDef.position.Set(33.0f, 16.0f);
	agentBody = world.CreateBody(&bodyDef);

	// Define another box shape for our dynamic body.
	b2PolygonShape agentShape;

    b2Vec2 x1 = b2Vec2(0.3f, 0.f);

	b2Vec2 x11 = b2Vec2(0.2f, 0.1f);
	b2Vec2 x12 = b2Vec2(0.2f, -0.1f);

    b2Vec2 verticesTri[4];

    verticesTri[0] = x11;
	verticesTri[1] = x12;
    verticesTri[2] =  b2Vec2(x1.y, -x1.x);
    verticesTri[3] =  -b2Vec2(x1.y, -x1.x);

    agentShape.Set(verticesTri,4);



    //agentShape.SetAsBox(1.0f, 1.0f);


	// Define the dynamic body fixture.
	b2FixtureDef fixtureDef;
	fixtureDef.shape = &agentShape;

	// Set the box density to be non-zero, so it will be dynamic.
	fixtureDef.density = 1.0f;

	// Override the default friction.
	fixtureDef.friction = 0.3f;

    //fixtureDef.filter.categoryBits = buildingsCategory;
    //fixtureDef.filter.maskBits = buildingsCategory;


	// Add the shape to the body.
    agentBody->CreateFixture(&fixtureDef);


    playerAgent.body = agentBody;
    playerAgent.type = player;

/*


{
    b2CircleShape sensor;
    sensor.m_radius = sight;
    b2FixtureDef fd;
    fd.shape = &sensor;
    fd.isSensor = true;
	fd.userData = &playerAgent;
    fd.filter.categoryBits = visibilityCategory;
    fd.filter.maskBits = visibilityCategory;

    agentBody->CreateFixture(&fd);

}
*/

    agentBody->SetAngularDamping(10.f);
	agentBody->SetLinearDamping(10.f);

    agentBody->SetUserData(&playerAgent);


    for (auto &t1: things){
        b2BodyDef bd;
        bd.userData = &POImarker;
        bd.type = b2_staticBody;
        b2Vec2 thingPos = b2Vec2(roads.nodes[t1.second.nodeId].x,roads.nodes[t1.second.nodeId].y);
        bd.position = thingPos;

        t1.second.body = world.CreateBody(&bd);

        b2CircleShape sensor;
        sensor.m_radius = 0.5f;
        b2FixtureDef fd;
        fd.shape = &sensor;
        fd.isSensor = true;
        fd.userData = &things[t1.second.id];
        fd.filter.categoryBits = visibilityCategory;
        fd.filter.maskBits = visibilityCategory;
        t1.second.body->CreateFixture(&fd);


    }

	//world.SetContactListener(new sensorContactListener);


	// Prepare for simulation. Typically we use a time step of 1/60 of a
	// second (60Hz) and 10 iterations. This provides a high quality simulation
	// in most game scenarios.
	float32 timeStep = 1.0f / 60.0f;
	int32 velocityIterations = 6;
	int32 positionIterations = 2;

	static std::set < entity* > POIinFOV = std::set < entity* >();

    while (!glfwWindowShouldClose(window)) {
        float ct = (float) glfwGetTime();
        if (run) t += ct - pt;

        pt = ct;

        glfwPollEvents();

        ImGui_ImplGlfwGL3_NewFrame();


        sInterface();


        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT);


        glEnable(GL_BLEND);
        glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
        glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);


        if (t>timeStep) {
            world.Step(timeStep, velocityIterations, positionIterations);
            if (cameraFollow) {
                g_camera.m_center = agentBody->GetPosition();
            }
			t = 0;
        }



		int state;

		float refVel = 5.f;

		state = glfwGetKey(window, GLFW_KEY_LEFT_SHIFT);
		if (state == GLFW_PRESS) {
			refVel = 10.f;
		}


		state = glfwGetKey(window, GLFW_KEY_RIGHT);
		if (state == GLFW_PRESS){
			agentBody->SetAngularVelocity(-4.f);
		}

		state = glfwGetKey(window, GLFW_KEY_LEFT);
		if (state == GLFW_PRESS) {
			agentBody->SetAngularVelocity(4.f);
		}


		state = glfwGetKey(window, GLFW_KEY_UP);
		if (state == GLFW_PRESS) {
			b2Vec2 forward = agentBody->GetWorldVector(b2Vec2(1.f, 0.f));
			forward.Normalize();

			float K = 2.f * (refVel - b2Dot(agentBody->GetLinearVelocity(), forward));
			forward*=K;

			agentBody->ApplyForceToCenter(forward,true);

			//agentBody->SetLinearVelocity(forward);
		}


        state = glfwGetKey(window, GLFW_KEY_DOWN);
        if (state == GLFW_PRESS) {
            b2Vec2 forward = agentBody->GetWorldVector(b2Vec2(1.f, 0.f));
            forward.Normalize();

			float K = 2.f * (-refVel - b2Dot(agentBody->GetLinearVelocity(),forward));

			forward *= K;


			agentBody->ApplyForceToCenter(forward,true);

            //agentBody->SetLinearVelocity(forward);
			
        }



        if (drawPaths) {
            g_debugDraw.DrawLines(linesDataStore, vertexCount, linesColorStore);
        }




        for (auto &t1: things){
            b2Vec2 agentPos = agentBody->GetPosition();
            b2Vec2 origPos = b2Vec2(roads.nodes[t1.second.nodeId].x,roads.nodes[t1.second.nodeId].y);
            b2Vec2 thingPos = agentPos + 0.97f*(b2Vec2(roads.nodes[t1.second.nodeId].x,roads.nodes[t1.second.nodeId].y) - agentPos);

            if ((agentPos - thingPos).Length() < sight) {
                RayCastClosestCallback cb;

                world.RayCast(&cb, agentPos, thingPos);

                /*
                if (cb.m_hit)
                {
                    g_debugDraw.DrawPoint(cb.m_point, 5.0f, b2Color(0.4f, 0.9f, 0.4f));
                    g_debugDraw.DrawSegment(agentPos, cb.m_point, b2Color(0.8f, 0.8f, 0.8f));
                    b2Vec2 head = cb.m_point + 0.5f * cb.m_normal;
                    g_debugDraw.DrawSegment(cb.m_point, head, b2Color(0.9f, 0.9f, 0.4f));
                }
                else
                {
                    g_debugDraw.DrawSegment(agentPos, thingPos, b2Color(0.8f, 0.8f, 0.8f));
                }
                 */


                if (!cb.m_hit) {

                    /*
					if (t1.second.descriptionArmed){
                        debug_log().AddLog("collision with POI \n");
                        t1.second.descriptionArmed = false;
                    }
					*/
					if (POIinFOV.find(&t1.second) == POIinFOV.end()) {
						debug_log().AddLog(roads.nodes[t1.second.nodeId].tags["name"]);
						debug_log().AddLog("\n");
						POIinFOV.insert(&t1.second);
					}
                    g_debugDraw.DrawSolidCircle(origPos, 1.f, b2Vec2(0.f, 0.f), b2Color(1.f, 1.f, 1.f, 1.f));
				}
				else {
					POIinFOV.erase(&t1.second);
					//if (POIinFOV.find(&t1.second) == POIinFOV.end()) {
				}
            }
			else {
				POIinFOV.erase(&t1.second);
			}



        }

        if (drawFOV) {

            for (float phi = 0.f; phi < 2 * b2_pi; phi = +phi + 0.01f) {
                b2Vec2 p1 = agentBody->GetPosition();
                b2Vec2 p2;
                p2.x = p1.x + sight * cos(phi);
                p2.y = p1.y + sight * sin(phi);



                //b2Vec2 p2 = agentBody->GetPosition() + 10.f * agentBody->GetWorldVector(b2Vec2(1.f, 0.f));

                RayCastClosestCallback callback;
                world.RayCast(&callback, p1, p2);

                if (callback.m_hit) {
                    //g_debugDraw.DrawPoint(callback.m_point, 5.0f, b2Color(0.4f, 0.9f, 0.4f));
                    g_debugDraw.DrawSegment(p1, callback.m_point, b2Color(0.8f, 0.8f, 0.8f, 0.4f));
                    //b2Vec2 head = callback.m_point + 0.5f * callback.m_normal;
                    //g_debugDraw.DrawSegment(callback.m_point, head, b2Color(0.9f, 0.9f, 0.4f));
                } else {
                    g_debugDraw.DrawSegment(p1, p2, b2Color(0.8f, 0.8f, 0.8f, 0.4f));
                }
            }
        }


		b2Vec2 vertices[3];
		for (int i = 0; i < nelems; i++) {
			const TESSindex *poly = &elems[i * 3];
			for (int j = 0; j < 3; j++) {
				if (poly[j] == TESS_UNDEF) break;
				vertices[j] = b2Vec2(verts[poly[j] * 2], verts[poly[j] * 2 + 1]);
			}
			g_debugDraw.DrawSolidPolygon(vertices, 3, b2Color(1.f, 0.f, 0.f, 1.f));
		}


        world.DrawDebugData();

	    g_debugDraw.Flush();
        ImGui::Render();

        glfwSwapBuffers(window);
        glfwPollEvents();
}

//if (tess) tessDeleteTess(tess);


//    if (vflags)
//        free(vflags);

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

