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

#include <functional>
#include <queue>
#include <vector>


#include "RStarTree.h"
#include "car.h"


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
bool cameraFollow = false;
bool drawFOV = false;


bool computeBounds = false;


bool m_showOpenDialog = false;


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

b2Body* officer;

entity playerAgent;

b2WeldJoint* wj;

b2WeldJointDef wjdef;

Car crow = Car(&world, 0.f, 4606.f, 2636.f);

int agentCollisionMarker = 0;
int POImarker = 1;
float sight = 100.f;


//const int16	visibilityGroup = 1;
//const int16	buildingsGroup = 2;

const uint16 visibilityCategory = 0x0001;
const uint16 buildingsCategory = 0x0002;

float walkSpeed = 5.f;
float runSpeed = 40.f;
float turnSpeed = 5.f;
float playerMass = 1.f;
int64_t playerSpawnPoint = 306977;

float gpuSpeed = 5.f;
float gpuPause = 5.f;
int64_t gpuSpawnPoint = 4169083660;




float linearDamping = 5.f;
float angularDamping = 10.f;

float currentTime = 0; 


bool mounted = false;

std::vector<b2Vec2> checkpoints = std::vector<b2Vec2>();


typedef RStarTree<int, 2, 32, 64> 			RTree;
typedef RTree::BoundingBox			BoundingBox;
RTree tree;


struct Visitor {
	int count;
	int id;
	bool ContinueVisiting;

	Visitor() : count(0), ContinueVisiting(true) {};

	void operator()(const RTree::Leaf * const leaf)
	{
		std::cout << "#" << count << ": visited " << leaf->leaf << " with bound " << leaf->bound.ToString() << std::endl;
		count++;
		id = leaf->leaf;
	}
};


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



void loadSettings(const char * name) {
    XMLDocument* doc = new XMLDocument();

    doc->LoadFile(name);
    XMLElement* n1 = doc->FirstChildElement("osm")->FirstChildElement("player");

    n1->QueryAttribute("velocity", &walkSpeed);
    n1->QueryAttribute("turnVelocity", &turnSpeed);
    n1->QueryAttribute("mass", &playerMass);
    n1->QueryAttribute("spawnPoint", &playerSpawnPoint);

    n1 = doc->FirstChildElement("osm")->FirstChildElement("gpu");

    n1->QueryAttribute("velocity", &gpuSpeed);
    n1->QueryAttribute("pause", &gpuPause);
    n1->QueryAttribute("spawnPoint", &gpuSpawnPoint);

};



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
		/*
        rect r1 = b1.second.bounds;
        if ((testx>r1.xmin) && (testx<r1.xmax) && (testy>r1.ymin) && (testy<r1.ymax))
        if (pnpoly(b1.second.numVert, b1.second.vertx, b1.second.verty, testx, testy)>0)
        {
            return id1;
        }
		*/

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

		if (button == GLFW_MOUSE_BUTTON_1) {
			
			/*
			for (int i = 0; i < nElemsTriangSelect; i++) {
				if (pnpoly(3, &vertx[i * 3], &verty[i * 3], selp.x, selp.y) > 0)
				{

					debug_log().AddLog("hit %d \n", vertNumber[i]);
				}
			}
			*/
			BoundingBox bound;
			bound.edges[0].first = selp.x - 1.f;
			bound.edges[0].second = selp.x + 1.f;

			bound.edges[1].first = selp.y - 1.f;
			bound.edges[1].second = selp.y + 1.f;

			Visitor x = tree.Query(RTree::AcceptOverlapping(bound), Visitor());


			debug_log().AddLog("Selected: %d \n", x.id);

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
		/*
		if (key == GLFW_KEY_SPACE && ((action == GLFW_PRESS) || (action == GLFW_REPEAT))) {
			endTurn();
		}
		*/
		if (key == GLFW_KEY_ENTER && action == GLFW_PRESS) {

			if (b2Distance(agentBody->GetPosition(), crow.hull->GetPosition()) < 3.f) {


				if (mounted) {
					world.DestroyJoint(wj);
					agentBody->ResetMassData();
				}
				else {
					wj = (b2WeldJoint*)world.CreateJoint(&wjdef);
					b2MassData md;
					md.mass = 0.01f; 
					md.I = 0.01f;
					agentBody->SetMassData(&md);
					
				}
				mounted = !mounted;
			}
		};




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

		(void)mods; // Modifiers are not reliable across systems
		io.KeyCtrl = io.KeysDown[GLFW_KEY_LEFT_CONTROL] || io.KeysDown[GLFW_KEY_RIGHT_CONTROL];
		io.KeyShift = io.KeysDown[GLFW_KEY_LEFT_SHIFT] || io.KeysDown[GLFW_KEY_RIGHT_SHIFT];
		io.KeyAlt = io.KeysDown[GLFW_KEY_LEFT_ALT] || io.KeysDown[GLFW_KEY_RIGHT_ALT];
		io.KeySuper = io.KeysDown[GLFW_KEY_LEFT_SUPER] || io.KeysDown[GLFW_KEY_RIGHT_SUPER];

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
		ImGui::CaptureKeyboardFromApp(true);
	}

	ImGuiIO &io = ImGui::GetIO();

	ImFontAtlas* atlas = ImGui::GetIO().Fonts;
	ImFont* font = atlas->Fonts[1];
	ImGui::PushFont(font);

	ImVec4 color = ImVec4(0.f, 0.f, 0.f, 0.0f);
	ImGuiStyle &style = ImGui::GetStyle();
	//style.Colors[ImGuiCol_WindowBg]=color;
	ImGui::PushStyleColor(ImGuiCol_WindowBg, color);

	int minutes = currentTime / 60;
	g_debugDraw.DrawString(width / 2, 0, "%dm %2.1fs", minutes, currentTime - minutes * 60);

	ImGui::PopStyleColor();
	ImGui::PopFont();


	int menuWidth = 600;
	{
		ImVec4 color = ImVec4(0.f, 0.f, 0.f, 0.3f);
		ImGuiStyle &style = ImGui::GetStyle();
		//style.Colors[ImGuiCol_WindowBg]=color;
		ImGui::PushStyleColor(ImGuiCol_WindowBg, color);
		ImGui::SetNextWindowPos(ImVec2(10, 10));
		ImGui::SetNextWindowSize(ImVec2((float)menuWidth, (float)g_camera.m_height - 20));

		ImGui::Begin("Info");


		b2Vec2 ps = b2Vec2(io.MousePos.x, io.MousePos.y);
		b2Vec2 pw = g_camera.ConvertScreenToWorld(ps);

		ImGui::Text("Mouse pos: (%f, %f)", pw.x, pw.y);
		//ImGui::Text("Current cell: (%f, %f)", floor(pw.x / g_camera.gridSize), floor(pw.y / g_camera.gridSize));

		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

		if (selected != std::string("none"))
		{
			ImGui::Text((std::string("id:") + city[selected].id).c_str());
		}

		ImGui::Checkbox("Draw Paths", &drawPaths);
		ImGui::Checkbox("Camera Follow", &cameraFollow);
		ImGui::Checkbox("Draw FOV", &drawFOV);


		ImGui::Text("currentVelocity: %f", agentBody->GetLinearVelocity().Length());

		ImGui::End();

		ImGui::PopStyleColor();
	}


	ImGui::Begin("Дела");

	if (ImGui::TreeNode("Подозреваемые"))
	{
		ImGui::Unindent(ImGui::GetTreeNodeToLabelSpacing());

		static int selection_mask = (1 << 2); // Dumb representation of what may be user-side selection state. You may carry selection state inside or outside your objects in whatever format you see fit.
		int node_clicked = -1;                // Temporary storage of what node we have clicked to process selection at the end of the loop. May be a pointer to your own node type, etc.
		ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, ImGui::GetFontSize() * 3); // Increase spacing to differentiate leaves from expanded contents.

		auto t1 = things.begin();
		for (int i = 0; i < things.size(); i++) {

			ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;

			bool node_open = ImGui::TreeNodeEx((void*)(intptr_t)i, node_flags, t1->second.name.c_str());
			if (ImGui::IsItemClicked())
				node_clicked = i;
			if (node_open)
			{
				ImGui::TextWrapped(t1->second.desc.c_str());
				ImGui::TreePop();
			}

			t1++;
		}
		ImGui::PopStyleVar();
		ImGui::Indent(ImGui::GetTreeNodeToLabelSpacing());
		ImGui::TreePop();
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
	/*
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
	*/

    a0.planFunc.push_back(goToAnyShop);
    //a0.planFunc.push_back(goHome);
    a0.planFunc.push_back(eat);

};


vector<int64_t> findPath (map_record navGraph, int64_t start, int64_t goal) {

    class searchNode {
    public:
		int64_t id;
        float g;
        bool operator == (const searchNode& other)
        { return id == other.id; }
        bool operator != (const searchNode& other)
        { return id != other.id; }
    };


    vector<int64_t> result = vector<int64_t>();

    //vector<searchNode> open = vector<searchNode>();

    auto cmp = [&](searchNode left, searchNode right) { return (left.g + sqrt((navGraph.nodes[goal].x - navGraph.nodes[left.id].x)*(navGraph.nodes[goal].x - navGraph.nodes[left.id].x) +
                                                                                     (navGraph.nodes[goal].y - navGraph.nodes[left.id].y)*(navGraph.nodes[goal].y - navGraph.nodes[left.id].y)) >
                                                               right.g + sqrt((navGraph.nodes[goal].x - navGraph.nodes[right.id].x)*(navGraph.nodes[goal].x - navGraph.nodes[right.id].x) +
                                                                    (navGraph.nodes[goal].y - navGraph.nodes[right.id].y)*(navGraph.nodes[goal].y - navGraph.nodes[right.id].y))); };
    std::priority_queue<searchNode, std::vector<searchNode>, decltype(cmp)> open(cmp);

    std::set<int64_t> closed = std::set<int64_t>();
    std::map <int64_t, int64_t> cameFrom = std::map <int64_t, int64_t>();


    searchNode startNode = {start, 0.f};

    open.push(startNode);

    searchNode goalNode = {goal, 0.f};



    while (open.size()>0){
        searchNode n0 = open.top();
        open.pop();
        closed.insert(n0.id);

        if (n0 == goalNode) {
			int64_t nx = goal;
            while (nx!=start)
            {
                result.push_back(nx);
                nx = cameFrom[nx];
            }
			result.push_back(nx);
			std::reverse(result.begin(),result.end());
            return result;
        }

        for (int64_t n1: navGraph.pathGraph[n0.id]){
            float dist =  sqrt((navGraph.nodes[n1].x - navGraph.nodes[n0.id].x)*(navGraph.nodes[n1].x - navGraph.nodes[n0.id].x) +
                                     (navGraph.nodes[n1].y - navGraph.nodes[n0.id].y)*(navGraph.nodes[n1].y - navGraph.nodes[n0.id].y));
            if (closed.find(n1)==closed.end()) {
                searchNode s1 = {n1, n0.g + dist};
                open.push(s1);
                cameFrom[n1] = n0.id;
            }
        }
    }
    return result;
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


    char* levelPath = "./maps/map3.osm";


	xm = 0;
	ym = 0;
	xp = 7500;
	yp = 5300;


    if (argc == 1) {
        //m_showOpenDialog = true;

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
            //glfwPollEvents();
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


	map_record roads = loadLevel(levelPath, tess, boundingBox, &world, computeBounds);

    loadSettings("./maps/config.xml");



    tessSetOption(tess, TESS_CONSTRAINED_DELAUNAY_TRIANGULATION, 1);
    if (!tessTesselate(tess, TESS_WINDING_ODD, TESS_POLYGONS, nvp, 2, 0))
        return -1;
    printf("Memory used: %.1f kB\n", allocated/1024.0f);




    const float* verts = tessGetVertices(tess);
    const int* vinds = tessGetVertexIndices(tess);
    const int* elems = tessGetElements(tess);
    const int nverts = tessGetVertexCount(tess);
    const int nelems = tessGetElementCount(tess);

	tess = tessNewTess(&ma);
	roads = loadLevel(levelPath, tess, boundingBox, &world, computeBounds, true);

	for (auto const& n1 : roads.pathGraph)
	{
		for (int neigh1 = 0; neigh1 < n1.second.size(); neigh1++) {
			//float r =  3.f * static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
			float r = 0.f;
				b2Vec2 v1 = 1.05f * b2Vec2(roads.nodes[n1.second[neigh1]].x - roads.nodes[n1.first].x, roads.nodes[n1.second[neigh1]].y - roads.nodes[n1.first].y);

				b2Vec2 normal1 = v1.Skew();
				normal1.Normalize();
				normal1 = 1.f*normal1;

				

				float coords[10];
				b2Vec2 p0 = b2Vec2(roads.nodes[n1.first].x+r, roads.nodes[n1.first].y+r);
				b2Vec2 p1 = p0 - normal1;
				coords[0] = p1.x;
				coords[1] = p1.y;
				b2Vec2 p2 = p1 + v1;
				coords[2] = p2.x;
				coords[3] = p2.y;
				b2Vec2 p3 = p2 + 2 * normal1;
				coords[4] = p3.x;
				coords[5] = p3.y;
				b2Vec2 p4 = p3 - v1;
				coords[6] = p4.x;
				coords[7] = p4.y;

				//coords[8] = p1.x;
				//coords[9] = p1.y;

				tessAddContour(tess, 2, coords, sizeof(float)*2, 4);
			}
		
	};

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

			/*
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
			*/
			int actualCount = 0;

			vs[0].Set(vertsCont[(base ) * vertexSize], vertsCont[(base) * vertexSize + 1]);
			actualCount++;

			for (int j = 1; j < count; j++) {
					b2Vec2 v1 = b2Vec2(vertsCont[(base + j) * vertexSize], vertsCont[(base + j) * vertexSize + 1]);
					b2Vec2 v2 = b2Vec2(vertsCont[(base + j - 1) * vertexSize], vertsCont[(base + j - 1) * vertexSize + 1]);
				
					if (b2DistanceSquared(v1, v2) > b2_linearSlop * b2_linearSlop) {
						vs[actualCount].Set(vertsCont[(base + j) * vertexSize], vertsCont[(base + j) * vertexSize + 1]);
						actualCount++;
					}
			}



            if (actualCount >=3) {
                b2ChainShape shape;
                shape.CreateLoop(vs, actualCount);
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
	bodyDef.position.Set(roads.nodes[playerSpawnPoint].x, roads.nodes[playerSpawnPoint].y);
	agentBody = world.CreateBody(&bodyDef);

	// Define another box shape for our dynamic body.
	b2PolygonShape agentShape;

    b2Vec2 x1 = b2Vec2(1.f, 0.f);

	b2Vec2 x11 = b2Vec2(0.f, 0.6f);
	b2Vec2 x12 = b2Vec2(0.f, -0.6f);

    b2Vec2 verticesTri[3];

    verticesTri[0] = x1;
	verticesTri[1] = x11;
	verticesTri[2] = x12;
    //verticesTri[2] =  b2Vec2(x1.y, -x1.x);
    //verticesTri[3] =  -b2Vec2(x1.y, -x1.x);

    agentShape.Set(verticesTri,3);



    //agentShape.SetAsBox(1.0f, 1.0f);


	// Define the dynamic body fixture.
	b2FixtureDef fixtureDef;
	fixtureDef.shape = &agentShape;
	fixtureDef.density = 0.1f;

	// Set the box density to be non-zero, so it will be dynamic.
	//fixtureDef.density = 0.1f;

	// Override the default friction.
	fixtureDef.friction = 0.3f;

    //fixtureDef.filter.categoryBits = buildingsCategory;
    //fixtureDef.filter.maskBits = buildingsCategory;


	// Add the shape to the body.
    agentBody->CreateFixture(&fixtureDef);
	//b2MassData massData = { 1.f, b2Vec2_zero, 1.f };

	//agentBody->SetMassData(&massData);

    playerAgent.body = agentBody;
    playerAgent.type = player;
	
    agentBody->SetUserData(&playerAgent);

	wjdef.bodyA = agentBody;
	wjdef.bodyB = crow.hull;
	wjdef.localAnchorA = b2Vec2(0.f, 0.f);
	wjdef.localAnchorB = b2Vec2(0.f, 0.f);
	wjdef.referenceAngle = 0.f;
	if (mounted)
		wj = (b2WeldJoint*)world.CreateJoint(&wjdef);


	loadThings("./maps/enemy.xml", things);


	vector<int64_t> res = vector<int64_t>();
	std::vector<std::function<float(b2Body* b1, float t, float dt)> > planFunc;

	vector<int64_t> r1 = findPath(roads, gpuSpawnPoint, things[0].node);
	res.insert(res.end(), r1.begin(), r1.end());


	    for (int j = 0; j < r1.size() - 1; j++) {
            b2Vec2 vb = b2Vec2(roads.nodes[r1[j]].x, roads.nodes[r1[j]].y);
            b2Vec2 ve = b2Vec2(roads.nodes[r1[j + 1]].x, roads.nodes[r1[j + 1]].y);

            auto followLine =
                    [&, vb, ve](b2Body* b1, float t, float dt) {
                        b2Vec2 n1 = (ve - vb);
                        n1.Normalize();
                        float velocity = gpuSpeed;
                        b2Vec2 currentPos = vb + t*velocity*n1;
                        b1->SetTransform(currentPos, 0.f);

                        if (t*velocity > (ve - vb).Length()) {
                            return -1.f;
                        }
                        else {
                            return t + dt;
                        }
                    };
            planFunc.push_back(followLine);
        }
        auto gpuWait =
                [&](b2Body* b1, float t, float dt) {

                    if (t>gpuPause) {
                        return -1.f;
                    }
                    else {
                        return t + dt;
                    }
                };
        planFunc.push_back(gpuWait);


	for (int i = 0; i < things.size()-1; i++)
	{
		vector<int64_t> r1 = findPath(roads, things[i].node, things[i + 1].node);
		res.insert(res.end(), r1.begin(),r1.end());

        for (int j = 0; j < r1.size() - 1; j++) {
            b2Vec2 vb = b2Vec2(roads.nodes[r1[j]].x, roads.nodes[r1[j]].y);
            b2Vec2 ve = b2Vec2(roads.nodes[r1[j + 1]].x, roads.nodes[r1[j + 1]].y);

            auto followLine =
                    [&, vb, ve](b2Body* b1, float t, float dt) {
                        b2Vec2 n1 = (ve - vb);
                        n1.Normalize();
                        float velocity = gpuSpeed;
                        b2Vec2 currentPos = vb + t*velocity*n1;
                        b1->SetTransform(currentPos, 0.f);

                        if (t*velocity > (ve - vb).Length()) {
                            return -1.f;
                        }
                        else {
                            return t + dt;
                        }
                    };
            planFunc.push_back(followLine);
        }
        auto gpuWait =
                [&](b2Body* b1, float t, float dt) {

                    if (t>gpuPause) {
                        return -1.f;
                    }
                    else {
                        return t + dt;
                    }
                };
        planFunc.push_back(gpuWait);


    }



	


	bodyDef.position.Set(roads.nodes[gpuSpawnPoint].x, roads.nodes[gpuSpawnPoint].y);
	officer = world.CreateBody(&bodyDef);
	officer->CreateFixture(&fixtureDef);

	/*

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
	*/

	//world.SetContactListener(new sensorContactListener);


	// Prepare for simulation. Typically we use a time step of 1/60 of a
	// second (60Hz) and 10 iterations. This provides a high quality simulation
	// in most game scenarios.
	float32 timeStep = 1.0f / 60.0f;
	int32 velocityIterations = 6;
	int32 positionIterations = 2;

	static std::set < entity* > POIinFOV = std::set < entity* >();


	//for (int i = 0; i < roads.buildings.size(); i++) {
	for (auto b1: roads.buildings) {
		//auto b1 = roads.buildings[i];
		BoundingBox bb = BoundingBox::MaximumBounds();
		auto c1 = b1.second.renderCoords;
		for (auto contour1 : c1) {
			for (auto v1 : contour1) {
				if (roads.nodes[v1].x < bb.edges[0].first)
					bb.edges[0].first = roads.nodes[v1].x;

				if (roads.nodes[v1].x > bb.edges[0].second)
					bb.edges[0].second = roads.nodes[v1].x;

				if (roads.nodes[v1].y < bb.edges[1].first)
					bb.edges[1].first = roads.nodes[v1].y;

				if (roads.nodes[v1].y > bb.edges[1].second)
					bb.edges[1].second = roads.nodes[v1].y;
			}
		}
		tree.Insert(b1.first, bb);
	}

	BoundingBox bound;
	bound.edges[0].first = 4064.f;
	bound.edges[0].second= 4140.f;

	bound.edges[1].first = 2280.f;
	bound.edges[1].second = 2353.f;

	Visitor x = tree.Query(RTree::AcceptOverlapping(bound), Visitor());






	/*
    vector<b2Vec2> treePoints = vector<b2Vec2>();
    for (auto n1: roads.pathGraph){
        for (int i1: n1.second) {
            treePoints.push_back(b2Vec2(roads.nodes[i1].x, roads.nodes[i1].y));
        }
    }
	*/



	/*
	vector<b2Vec2> points = vector<b2Vec2>();
    points.push_back(b2Vec2(100.f,280.f));
    points.push_back(b2Vec2(500.f,575.f));
    points.push_back(b2Vec2(1000.f,390.f));

    kdNode* n1 = kdTree <b2Vec2> (treePoints, 0);
	*/


	int checkpointNum = things.size();
	for (int i = 0; i < checkpointNum; i++) {
		auto it = things.begin();
		//std::advance(it, rand() % things.size());
		std::advance(it, i);
		
		debug_log().AddLog("index %d \n", it->first);
		debug_log().AddLog("id: %d \n", things[it->first].id);
		//b2Vec2 t1 = b2Vec2(roads.nodes[things[it->first].nodeId].x, roads.nodes[things[it->first].nodeId].y);
		b2Vec2 t1 = b2Vec2(roads.nodes[things[it->first].node].x, roads.nodes[things[it->first].node].y);
		debug_log().AddLog("checkpoint %d: %g,%g \n", things[it->first].node, t1.x, t1.y);
		checkpoints.push_back(t1); 
	}
	

	g_camera.m_center.x = 4572;
	g_camera.m_center.y = 2047;
	
	//checkpoints.push_back(b2Vec2(500.f, 575.f));
	//checkpoints.push_back(b2Vec2(1000.f, 390.f));

	float tzero = glfwGetTime();

    while (!glfwWindowShouldClose(window)) {
        float ct = (float) glfwGetTime();

		currentTime = ct - tzero;
        if (run) t += ct - pt;





        ImGui_ImplGlfwGL3_NewFrame();


        sInterface();


        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);


        glEnable(GL_BLEND);
        glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
        glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);





		int state;


		glfwPollEvents();
		/*

		if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS) {
			if (b2Distance(agentBody->GetPosition(), crow.hull->GetPosition()) < 3.f) {
				if (mounted) {
					world.DestroyJoint(wj);
				}
				else {
					wj = (b2WeldJoint*)world.CreateJoint(&wjdef);
				}
				mounted = !mounted;
			}
		};

		*/


		if (!mounted) {

			agentBody->SetAngularDamping(angularDamping);
			agentBody->SetLinearDamping(linearDamping);


			float refVel = walkSpeed;

			state = glfwGetKey(window, GLFW_KEY_LEFT_SHIFT);
			if (state == GLFW_PRESS) {
				refVel = runSpeed;
			}


			state = glfwGetKey(window, GLFW_KEY_D);
			if (state == GLFW_PRESS) {
				agentBody->SetAngularVelocity(-turnSpeed);
			}

			state = glfwGetKey(window, GLFW_KEY_A);
			if (state == GLFW_PRESS) {
				agentBody->SetAngularVelocity(turnSpeed);
			}


			state = glfwGetKey(window, GLFW_KEY_W);
			if (state == GLFW_PRESS) {
				b2Vec2 forward = agentBody->GetWorldVector(b2Vec2(1.f, 0.f));
				forward.Normalize();

				float K = 1.f * (refVel - b2Dot(agentBody->GetLinearVelocity(), forward));
				forward *= K;

				agentBody->ApplyForceToCenter(forward, true);

				//agentBody->SetLinearVelocity(forward);
			}


			state = glfwGetKey(window, GLFW_KEY_S);
			if (state == GLFW_PRESS) {
				b2Vec2 forward = agentBody->GetWorldVector(b2Vec2(1.f, 0.f));
				forward.Normalize();

				float K = 1.f * (-refVel - b2Dot(agentBody->GetLinearVelocity(), forward));

				forward *= K;


				agentBody->ApplyForceToCenter(forward, true);

				//agentBody->SetLinearVelocity(forward);

			}
		}
		else {



			state = glfwGetKey(window, GLFW_KEY_W);
			if (state == GLFW_PRESS) {
				crow.accelerate(0.8f);
			}



			state = glfwGetKey(window, GLFW_KEY_S);
			if (state == GLFW_PRESS) {
				//crow.brake(0.8f);
				crow.accelerate(-0.8f);
			}

			if ((glfwGetKey(window, GLFW_KEY_W) == GLFW_RELEASE) && (glfwGetKey(window, GLFW_KEY_S) == GLFW_RELEASE)) {
				crow.accelerate(0.0f);
			}

			if (glfwGetKey(window, GLFW_KEY_SPACE)) {
				crow.brake(1.f);
			}
			else {
				crow.brake(0.f);
			}



			state = glfwGetKey(window, GLFW_KEY_D);
			if (state == GLFW_PRESS) {
				crow.steer(-1.f);
			}


			state = glfwGetKey(window, GLFW_KEY_A);
			if (state == GLFW_PRESS) {
				crow.steer(1.f);
			}

			if ((glfwGetKey(window, GLFW_KEY_D) == GLFW_RELEASE) && (glfwGetKey(window, GLFW_KEY_A) == GLFW_RELEASE)) {
				crow.steer(0.f);
			}

		}



		if (t>timeStep) {
			crow.step(timeStep);
			world.Step(timeStep, velocityIterations, positionIterations);
			if (cameraFollow) {
				g_camera.m_center = agentBody->GetPosition();
			}
			t = 0;
		}


        if (drawPaths) {
            g_debugDraw.DrawLines(linesDataStore, vertexCount, linesColorStore);
        }

		static float tt1 = 0.f;
		//tt1 = followLine(officer, tt1);

		if (planFunc.size() > 0) {
			tt1 = planFunc.front()(officer, tt1, ct-pt);
			if (tt1 < 0) {
				planFunc.erase(planFunc.begin());
				tt1 = 0;
			}
		}


        for (auto &t1: things){
            b2Vec2 agentPos = agentBody->GetPosition();
            b2Vec2 origPos = b2Vec2(roads.nodes[t1.second.node].x,roads.nodes[t1.second.node].y);
            b2Vec2 thingPos = agentPos + 0.97f*(b2Vec2(roads.nodes[t1.second.node].x,roads.nodes[t1.second.node].y) - agentPos);

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
						debug_log().AddLog(roads.nodes[t1.second.node].tags["name"]);
						debug_log().AddLog(t1.second.desc);
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



		if (res.size() > 0) {
			b2Vec2 p1(roads.nodes[res[0]].x, roads.nodes[res[0]].y);
			b2Vec2 p2(roads.nodes[res[1]].x, roads.nodes[res[1]].y);

			g_debugDraw.DrawSegment(p1, p2, b2Color(0.0f, 1.f, 0.0f, 1.0f));

			for (int i = 1; i < res.size()-1; i++) {
				p1.Set(roads.nodes[res[i]].x, roads.nodes[res[i]].y);
				p2.Set(roads.nodes[res[i+1]].x, roads.nodes[res[i+1]].y);
				g_debugDraw.DrawSegment(p1, p2, b2Color(0.0f, 1.f, 0.0f, 1.0f));
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



		for (int i = 0; i < checkpoints.size(); i++) {

			b2Vec2 vertices[3];
			b2Vec2 p0 = checkpoints[i];

			vertices[0] =  b2Vec2(0.f, 1.f);
			vertices[1] =  b2Vec2(1.f, -1.f);
			vertices[2] =  b2Vec2(-1.f, -1.f);

			float theta;
			theta = currentTime / 60 * (15 * b2_pi);
			b2Mat22 rot1 = b2Mat22(cos(theta), -sin(theta), sin(theta), cos(theta));

			for (int j = 0; j < 3; j++){
				vertices[j] = p0 + b2Mul(rot1, 15.f * vertices[j]);
			}

			g_debugDraw.DrawSolidPolygon(vertices, 3, b2Color(0.f, 1.f, 0.f, 1.f));
		}

		b2Vec2 officerPolygon[3];

		officerPolygon[0] = officer->GetPosition() + b2Vec2(0.f, -5.f);
		officerPolygon[1] = officer->GetPosition() + b2Vec2(5.f, 5.f);
		officerPolygon[2] = officer->GetPosition() + b2Vec2(-5.f, 5.f);

		g_debugDraw.DrawSolidPolygon(officerPolygon, 3, b2Color(0.f, 0.7f, 1.f, 1.f));





		checkpoints.erase(std::remove_if(checkpoints.begin(), checkpoints.end(),
			[&](b2Vec2 cp) { return (((cp - agentBody->GetPosition()).Length()<15.f) || ((cp - officer->GetPosition()).Length()<15.f) ); }), checkpoints.end());



        world.DrawDebugData();

	    g_debugDraw.Flush();
        ImGui::Render();

        glfwSwapBuffers(window);
        glfwPollEvents();

        pt = ct;
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

