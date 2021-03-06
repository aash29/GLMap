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

#include "soloud/soloud.h"
#include "soloud/soloud_wav.h"

#include "dialog.h"

#include "SOIL/SOIL.h"

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
bool drawGpuPath = true;
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

Car crow = Car(&world, 0.f, 4912.f, 3020.f);

sprite carSprite = {3.f,-1.5f,-3.f,1.5f, 
					0.f,0.02f,0.7f,0.98f};


sprite gpuSprite = { 0.5f,-0.5f,-0.5f,0.5f,
0.85f,0.33f,1.f,0.6f };


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


map <int, conversation> dialogs;

int run = 1;

int currentConversation = 0;
int currentSuspect = -1;

float stamina = 1;
float staminaRateIncStop = 0.2;
float staminaRateIncWalk = 0.1;
float staminaRateDec = 0.01;

char* levelPath = "./content/map3.osm";

int showInfo = 1;
int showCases = 1;
int showTimer = 1;

float infoDelay = 10.f;

bool cameraScroll = true;

b2Color groundColor = b2Color(0.f, 0.f, 0.f, 1.f);

b2Color buildingColor = b2Color(1.f, 0.f, 0.f, 1.f);


int mapWidth, mapHeight;

bool blockInput = false;

bool showMap = false;

map_record roads;

string getHouseInfo (int64_t id) { 

	return (roads.buildings[id].addrStreet+ "," + roads.buildings[id].addrNumber + roads.buildings[id].addrLetter);
};

void loadTexture() {


	glGenTextures(2, g_debugDraw.textures);

	//int width, height;
	unsigned char* image;

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, g_debugDraw.textures[0]);
	image = SOIL_load_image("./content/map.png", &mapWidth, &mapHeight, 0, SOIL_LOAD_RGBA);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, mapWidth, mapHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
	SOIL_free_image_data(image);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);


	
	//glGenTextures(1, textures);

	//int width, height;
	unsigned char* carSprite;

	int spriteWidth;
	int spriteHeight;

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, g_debugDraw.textures[1]);
	carSprite = SOIL_load_image("./content/m1.png", &spriteWidth, &spriteHeight, 0, SOIL_LOAD_RGBA);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, spriteWidth, spriteHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, carSprite);
	SOIL_free_image_data(carSprite);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	

}

void loadSettings(const char * name) {
	XMLDocument* doc = new XMLDocument();

	doc->LoadFile(name);

	XMLElement* n1 = doc->FirstChildElement("osm")->FirstChildElement("level");

	n1->Attribute("map", levelPath);

	n1 = doc->FirstChildElement("osm")->FirstChildElement("player");

	n1->QueryAttribute("velocity", &walkSpeed);
	n1->QueryAttribute("runVelocity", &runSpeed);
	n1->QueryAttribute("turnVelocity", &turnSpeed);
	n1->QueryAttribute("mass", &playerMass);
	n1->QueryAttribute("spawnPoint", &playerSpawnPoint);

	n1->QueryAttribute("staminaRateIncStop", &staminaRateIncStop);
	n1->QueryAttribute("staminaRateIncWalk", &staminaRateIncWalk);
	n1->QueryAttribute("staminaRateDec", &staminaRateDec);


	n1 = doc->FirstChildElement("osm")->FirstChildElement("gpu");

	n1->QueryAttribute("velocity", &gpuSpeed);
	n1->QueryAttribute("pause", &gpuPause);
	n1->QueryAttribute("spawnPoint", &gpuSpawnPoint);

	n1 = doc->FirstChildElement("osm")->FirstChildElement("windows");

	n1->QueryAttribute("info", &showInfo);
	n1->QueryAttribute("cases", &showCases);
	n1->QueryAttribute("timer", &showTimer);

	n1 = doc->FirstChildElement("osm")->FirstChildElement("camera");

	n1->QueryAttribute("follow", &cameraFollow);
	n1->QueryAttribute("scroll", &cameraScroll);
	n1->QueryAttribute("zoom", &g_camera.m_zoom);


	n1 = doc->FirstChildElement("osm")->FirstChildElement("appearance");

	const char * grColor = n1->Attribute("groundColor");
	sscanf(grColor, "%f,%f,%f,%f", &groundColor.r, &groundColor.g, &groundColor.b, &groundColor.a);


	const char * bColor = n1->Attribute("buildingColor");
	sscanf(bColor, "%f,%f,%f,%f", &buildingColor.r, &buildingColor.g, &buildingColor.b, &buildingColor.a);

	n1->QueryAttribute("drawPaths", &drawPaths);

	n1->QueryAttribute("drawGpuPath", &drawGpuPath);

};




struct Visitor {
	int count;
	int id;
	std::vector<int> visited = std::vector<int>();
	bool ContinueVisiting;

	Visitor() : count(0), ContinueVisiting(true) {};

	void operator()(const RTree::Leaf * const leaf)
	{
		std::cout << "#" << count << ": visited " << leaf->leaf << " with bound " << leaf->bound.ToString() << std::endl;
		count++;
		id = leaf->leaf;
		visited.push_back(id);
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
		if (cameraScroll) {
			if (dy > 0) {
				g_camera.m_zoom /= 1.1f;
			}
			else {
				g_camera.m_zoom *= 1.1f;
			}
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

			if (action == GLFW_PRESS) {
				BoundingBox bound;
				bound.edges[0].first = selp.x - 0.1f;
				bound.edges[0].second = selp.x + 0.1f;

				bound.edges[1].first = selp.y - 0.1f;
				bound.edges[1].second = selp.y + 0.1f;

				Visitor x = tree.Query(RTree::AcceptOverlapping(bound), Visitor());
				x.visited.erase(std::remove(x.visited.begin(), x.visited.end(), 1576083), x.visited.end());
				x.visited.erase(std::remove(x.visited.begin(), x.visited.end(), 1576085), x.visited.end());
				for (auto it = x.visited.begin(); it != x.visited.end(); it++) {
					
					double* xv;
					double* yv;
					xv = new double[roads.buildings[*it].renderCoords.size()*50];
					yv = new double[roads.buildings[*it].renderCoords.size()*50];
					int i = 0;
					for (auto с1 : roads.buildings[*it].renderCoords) {
						for (auto v1 : с1) {
							xv[i] = roads.nodes[v1].x;
							yv[i] = roads.nodes[v1].y;
							i++;
						}
					}
					
					if (pnpoly(i, xv, yv, selp.x, selp.y)>0)
					{
						debug_log().AddLog("%s, id: %d \n",getHouseInfo(*it).c_str(), *it);
					}

					delete xv;
					delete yv;


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

					b2Filter f1;
					f1.maskBits = 0x0001;
					agentBody->GetFixtureList()[0].SetFilterData(f1);

				}
				else {
					wj = (b2WeldJoint*)world.CreateJoint(&wjdef);
					b2MassData md;
					md.mass = 0.01f; 
					md.I = 0.01f;
					agentBody->SetMassData(&md);
					
					b2Filter f1;
					f1.maskBits = 0x0000;
					agentBody->GetFixtureList()[0].SetFilterData(f1);
					
				}
				mounted = !mounted;
			}
		};



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




void sInterface() {



	if (ImGui::IsAnyWindowHovered()) {
		ImGui::CaptureMouseFromApp(true);
		ImGui::CaptureKeyboardFromApp(true);
	}

	ImGuiIO &io = ImGui::GetIO();

	if (showMap) {
		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2((float)g_camera.m_width, (float)g_camera.m_height));
		ImGui::Begin("map");
		ImGui::Image((void*)(g_debugDraw.textures[0]), ImVec2(mapWidth, mapHeight));
		ImGui::End();
	}
	if (showTimer) {

		ImFontAtlas* atlas = ImGui::GetIO().Fonts;
		ImFont* font = atlas->Fonts[1];
		ImGui::PushFont(font);

		ImVec4 color = ImVec4(0.f, 0.f, 0.f, 0.0f);

		ImGuiStyle &style = ImGui::GetStyle();
		//style.Colors[ImGuiCol_WindowBg]=color;
		ImGui::PushStyleColor(ImGuiCol_WindowBg, color);
		//ImGui::PushStyleColor(ImGuiCol_Text, textColor);


		int minutes = currentTime / 60;
		g_debugDraw.DrawString(width / 2, 0, "%dm %2.1fs", minutes, currentTime - minutes * 60);

		ImGui::PopStyleColor();
		ImGui::PopFont();
	}

	int menuWidth = 600;
	{
		if (showInfo) {
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

			ImGui::ProgressBar(stamina, ImVec2(0.0f, 0.0f));


			ImGui::Text("stamina: %f", stamina);


			debug_log().Draw("log",ImGui::GetWindowWidth(), ImGui::GetWindowHeight());

			ImGui::End();


			ImGui::PopStyleColor();
		}
	}


	//ImGui::ShowTestWindow();
	static float infoTimer = 0.f;
	if (currentSuspect != -1)
	{
		if (infoTimer>infoDelay) { 
			currentSuspect = -1;
			infoTimer = 0.f;
		}
		else
		{
			infoTimer += 0.05f;
		}
		ImGui::SetNextWindowPos(ImVec2(500, 400));
		ImGui::SetNextWindowSize(ImVec2(700, -1));

		ImGuiWindowFlags window_flags = 0;
		window_flags |= ImGuiWindowFlags_NoTitleBar;
		
		window_flags |= ImGuiWindowFlags_NoResize;
		window_flags |= ImGuiWindowFlags_NoMove;
		window_flags |= ImGuiWindowFlags_NoScrollbar;
		window_flags |= ImGuiWindowFlags_NoCollapse;
		
	
		
		ImVec4 color = ImVec4(0.f, 0.f, 0.f, 0.0f);
		ImVec4 textColor = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
		ImGuiStyle &style = ImGui::GetStyle();
		//style.Colors[ImGuiCol_WindowBg]=color;
		ImGui::PushStyleColor(ImGuiCol_WindowBg, color);
		ImGui::PushStyleColor(ImGuiCol_Text, textColor);

		static bool b1 = true;

		ImGui::Begin("",&b1, window_flags);

		//static int currentReply = 1;

		ImGui::TextWrapped(things[currentSuspect].desc.c_str());

		/*
		//for (int i = 0; i < dialogs[currentConversation].replies[currentReply].answers.size(); i++) {
		for (auto a1 : dialogs[currentConversation].replies[currentReply].answers) {
			ImGui::PushID(a1.second.id);
			if (ImGui::Button(a1.second.text.c_str(), ImVec2(-1, 0))) {
				currentReply = a1.second.address;
			}
			ImGui::PopID();
		}


		if (currentReply == 0) {
			currentConversation = 0;
			currentReply = 1;
			run = 1;
			blockInput = false;
		};
		*/

		ImGui::End();

		ImGui::PopStyleColor();
		ImGui::PopStyleColor();
	}

	

	if (showCases) {
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
	}
	

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

	SoLoud::Soloud gSoloud; // SoLoud engine
	SoLoud::Wav gWave;      // One wave file

	gSoloud.init(); // Initialize SoLoud

	gWave.load("foot.wav"); // Load a wave

	gWave.setSingleInstance(true);
	gWave.setLooping(true);
	gWave.setInaudibleBehavior(true, true);


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

	loadTexture();

	xm = 0;
	ym = 0;
	xp = 7500;
	yp = 5300;


    if (argc == 1) {
        //m_showOpenDialog = true;

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


	roads = loadLevel(levelPath, tess, boundingBox, &world, computeBounds);

    loadSettings("./content/config.xml");



    tessSetOption(tess, TESS_CONSTRAINED_DELAUNAY_TRIANGULATION, 1);
    if (!tessTesselate(tess, TESS_WINDING_ODD, TESS_POLYGONS, nvp, 2, 0))
        return -1;
    printf("Memory used: %.1f kB\n", allocated/1024.0f);




    const float* verts = tessGetVertices(tess);
    const int* vinds = tessGetVertexIndices(tess);
    const int* elems = tessGetElements(tess);
    const int nverts = tessGetVertexCount(tess);
    const int nelems = tessGetElementCount(tess);


	debug_log().AddLog("%d,%d,%d", vinds[0], vinds[1], vinds[2]);

	//b2Color* buildingColors = new b2Color[]


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

				//tessAddContour(tess, 2, coords, sizeof(float)*2, 4);
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
            linesColorStore[vertexCount] = b2Color (0.f,0.f,0.f,1.f);
			vertexCount++;


			float x1 = roads.nodes[n1.second[neigh1]].x;
			float y1 = roads.nodes[n1.second[neigh1]].y;
            linesDataStore[vertexCount] = b2Vec2(x1,y1);
            linesColorStore[vertexCount] = b2Color (0.f,0.f,0.f,1.f);

			vertexCount++;
		}
	}

	//int i = 0;
	for (auto b1 : roads.buildings) {
		for (auto с1 : b1.second.renderCoords) {
			for (int i = 0; i <= с1.size() - 2; i++) {

				int64_t vb = с1[i];

				linesDataStore[vertexCount] = b2Vec2(roads.nodes[vb].x, roads.nodes[vb].y);
				linesColorStore[vertexCount] = b2Color(1.f, 0.f, 0.f, 1.f);
				vertexCount++;

				int64_t ve = с1[i + 1];

				linesDataStore[vertexCount] = b2Vec2(roads.nodes[ve].x, roads.nodes[ve].y);
				linesColorStore[vertexCount] = b2Color(1.f, 0.f, 0.f, 1.f);
				vertexCount++;
			}
			
			/*
			int64_t vb = с1.back();

			linesDataStore[vertexCount] = b2Vec2(roads.nodes[vb].x, roads.nodes[vb].y);
			linesColorStore[vertexCount] = b2Color(0.f, 1.f, 0.f, 1.f);
			vertexCount++;

			int64_t ve = с1[0];
			linesDataStore[vertexCount] = b2Vec2(roads.nodes[ve].x, roads.nodes[ve].y);
			linesColorStore[vertexCount] = b2Color(0.f, 1.f, 0.f, 1.f);
			vertexCount++;
			*/
		}
	}






	tess = tessNewTess(&ma);

	for (auto i1 : roads.background) {
		for (auto j1 = i1.second.begin(); j1 != i1.second.end(); j1++) {
			vector<float> coords = vector<float>();
			for (auto k1 = j1->begin(); k1 != j1->end(); k1++) {
				coords.push_back(roads.nodes[*k1].x);
				coords.push_back(roads.nodes[*k1].y);
			}
			tessAddContour(tess, 2, coords.data(), sizeof(float) * 2, round(coords.size() / 2));
		}
	}

	tessSetOption(tess, TESS_CONSTRAINED_DELAUNAY_TRIANGULATION, 1);
	if (!tessTesselate(tess, TESS_WINDING_ODD, TESS_POLYGONS, nvp, 2, 0))
		return -1;
	printf("Memory used: %.1f kB\n", allocated / 1024.0f);

	const float* vertsBack = tessGetVertices(tess);
	const int* vindsBack = tessGetVertexIndices(tess);
	const int* elemsBack = tessGetElements(tess);
	const int nvertsBack = tessGetVertexCount(tess);
	const int nelemsBack = tessGetElementCount(tess);


	g_debugDraw.Create();


	g_debugDraw.SetFlags(b2Draw::e_shapeBit);
	//g_debugDraw.SetFlags(b2Draw::e_centerOfMassBit);

    //world.SetDebugDraw(&g_debugDraw);


	// Define the dynamic body. We set its position and call the body factory.
	b2BodyDef bodyDef;
	bodyDef.type = b2_dynamicBody;
	bodyDef.position.Set(roads.nodes[playerSpawnPoint].x, roads.nodes[playerSpawnPoint].y);
	agentBody = world.CreateBody(&bodyDef);

	// Define another box shape for our dynamic body.
	b2PolygonShape agentShape;

    b2Vec2 verticesQuad[4];

	verticesQuad[0] = b2Vec2(-0.3f, 0.3f);
	verticesQuad[1] = b2Vec2(0.3f, 0.3f);
	verticesQuad[2] = b2Vec2(0.3f, -0.3f);
	verticesQuad[3] = b2Vec2(-0.3f, -0.3f);
    //verticesTri[2] =  b2Vec2(x1.y, -x1.x);
    //verticesTri[3] =  -b2Vec2(x1.y, -x1.x);

	

    agentShape.Set(verticesQuad,4);



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
	b2MassData massData = { playerMass, b2Vec2_zero, 1.f };

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


	loadThings("./content/checkpoints.xml", things);

	dialogs = loadDialog("./content/dialogues.xml");

	/*
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



	*/


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

	/*
	BoundingBox bound;
	bound.edges[0].first = 4064.f;
	bound.edges[0].second= 4140.f;

	bound.edges[1].first = 2280.f;
	bound.edges[1].second = 2353.f;

	Visitor x = tree.Query(RTree::AcceptOverlapping(bound), Visitor());


	*/



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
/*

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
	*/

	g_camera.m_center.x = agentBody->GetPosition().x;
	g_camera.m_center.y = agentBody->GetPosition().y;
	
	//checkpoints.push_back(b2Vec2(500.f, 575.f));
	//checkpoints.push_back(b2Vec2(1000.f, 390.f));

	float tzero = glfwGetTime();


	while (!glfwWindowShouldClose(window)) {
		float ct = (float)glfwGetTime();


		if (run) {
			t += ct - pt;
			currentTime += t;
		}





		ImGui_ImplGlfwGL3_NewFrame();


		sInterface();

		glClearColor(groundColor.r, groundColor.g, groundColor.b, groundColor.a);
		glClear(GL_COLOR_BUFFER_BIT);


		glEnable(GL_BLEND);
		//glEnable(GL_DEPTH_TEST);

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

		state = glfwGetKey(window, GLFW_KEY_TAB);
		if (state == GLFW_PRESS) {
			showMap = true;
		}
		else {
			showMap = false;
		}

		if (!blockInput) {
			if (!mounted) {

				static SoLoud::handle handle = 0;
				static bool walking = false;

				if (agentBody->GetLinearVelocity().Length() > 0.2f) {

					if (!walking) {
						handle = gSoloud.play(gWave, 1.f, 0.f, false); // Play the wave
						walking = true;
					}

				}
				else {
					walking = false;
					gSoloud.setPause(handle, true);
				}


				agentBody->SetAngularDamping(angularDamping);
				agentBody->SetLinearDamping(linearDamping);


				float refVel = walkSpeed;



				state = glfwGetKey(window, GLFW_KEY_LEFT_SHIFT);
				if (state == GLFW_PRESS) {

					if (stamina > 0) {
						refVel = runSpeed;
					}

					stamina = stamina - staminaRateDec*(ct - pt);

					stamina = max(0.f, stamina);
				}

				else {
					if (agentBody->GetLinearVelocity().Length() > 0.2f) {
						stamina = stamina + staminaRateIncWalk*(ct - pt);
					}
					else {
						stamina = stamina + staminaRateIncStop*(ct - pt);
					}
					stamina = min(1.f, stamina);
				}


				state = glfwGetKey(window, GLFW_KEY_D);
				if (state == GLFW_PRESS) {
					agentBody->SetAngularVelocity(-turnSpeed);
					//gSoloud.play(gWave);

				}

				state = glfwGetKey(window, GLFW_KEY_A);
				if (state == GLFW_PRESS) {
					agentBody->SetAngularVelocity(turnSpeed);
					//gSoloud.play(gWave);
				}


				state = glfwGetKey(window, GLFW_KEY_W);
				if (state == GLFW_PRESS) {
					b2Vec2 forward = agentBody->GetWorldVector(b2Vec2(1.f, 0.f));
					forward.Normalize();

					float K = 1.f * (refVel - b2Dot(agentBody->GetLinearVelocity(), forward));
					forward *= K;

					agentBody->ApplyForceToCenter(forward, true);
					//gSoloud.play(gWave);

					//agentBody->SetLinearVelocity(forward);
				}
				else
				{
					agentBody->SetLinearVelocity(b2Vec2(0.f, 0.f));
				}


				state = glfwGetKey(window, GLFW_KEY_S);
				if (state == GLFW_PRESS) {
					b2Vec2 forward = agentBody->GetWorldVector(b2Vec2(1.f, 0.f));
					forward.Normalize();

					float K = 1.f * (-refVel - b2Dot(agentBody->GetLinearVelocity(), forward));

					forward *= K;

					agentBody->ApplyForceToCenter(forward, true);
					//gSoloud.play(gWave);

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

		}

		if (t > timeStep) {
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
		/*
		static float tt1 = 0.f;
		//tt1 = followLine(officer, tt1);
		if (run) {
			if (planFunc.size() > 0) {
				tt1 = planFunc.front()(officer, tt1, ct - pt);
				if (tt1 < 0) {
					planFunc.erase(planFunc.begin());
					tt1 = 0;
				}
			}
		}
		*/
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
                    //g_debugDraw.DrawSolidCircle(origPos, 1.f, b2Vec2(0.f, 0.f), b2Color(1.f, 1.f, 1.f, 1.f));
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

		/*
		if (drawGpuPath) {
			if (res.size() > 0) {
				b2Vec2 p1(roads.nodes[res[0]].x, roads.nodes[res[0]].y);
				b2Vec2 p2(roads.nodes[res[1]].x, roads.nodes[res[1]].y);

				g_debugDraw.DrawSegment(p1, p2, b2Color(0.0f, 1.f, 0.0f, 1.0f));

				for (int i = 1; i < res.size() - 1; i++) {
					p1.Set(roads.nodes[res[i]].x, roads.nodes[res[i]].y);
					p2.Set(roads.nodes[res[i + 1]].x, roads.nodes[res[i + 1]].y);
					g_debugDraw.DrawSegment(p1, p2, b2Color(0.0f, 1.f, 0.0f, 1.0f));
				}
			}
		}
		*/

		
		g_debugDraw.DrawTexQuad(crow.hull->GetPosition(), b2Color(1.f, 1.f, 1.f, 1.f), carSprite, crow.hull->GetAngle());
		
		if (!mounted) {
			g_debugDraw.DrawTexQuad(agentBody->GetPosition(), b2Color(1.f, 1.f, 1.f, 1.f), gpuSprite, agentBody->GetAngle());
		}


		b2Vec2 vertices[3];
		for (int i = 0; i < nelemsBack; i++) {
			const TESSindex *poly = &elemsBack[i * 3];
			for (int j = 0; j < 3; j++) {
				if (poly[j] == TESS_UNDEF) break;
				vertices[j] = b2Vec2(vertsBack[poly[j] * 2], vertsBack[poly[j] * 2 + 1]);
			}
			g_debugDraw.DrawSolidPolygon(vertices, 3, b2Color(0.9f, 0.9f, 0.9f));
		}



		for (int i = 0; i < nelems; i++) {
			const TESSindex *poly = &elems[i * 3];
			for (int j = 0; j < 3; j++) {
				if (poly[j] == TESS_UNDEF) break;
				vertices[j] = b2Vec2(verts[poly[j] * 2], verts[poly[j] * 2 + 1]);
			}
			g_debugDraw.DrawSolidPolygon(vertices, 3, buildingColor);
		}





		for (auto & cp: things) {
			if (cp.second.active) {
				//for (int i = 0; i < checkpoints.size(); i++) {

				b2Vec2 p0 = b2Vec2(roads.nodes[things[cp.first].node].x, roads.nodes[things[cp.first].node].y);

				b2Vec2 vertices[3];


				vertices[0] = b2Vec2(cos(0.f), sin(0.f));
				vertices[1] = b2Vec2(cos(2*b2_pi/3.f), sin(2 * b2_pi / 3.f));
				vertices[2] = b2Vec2(cos(4 * b2_pi / 3.f), sin(4 * b2_pi / 3.f));

				float theta;
				theta = currentTime / 60 * (15 * b2_pi);
				b2Mat22 rot1 = b2Mat22(cos(theta), -sin(theta), sin(theta), cos(theta));

				for (int j = 0; j < 3; j++) {
					vertices[j] = p0 + b2Mul(rot1, 15.f * vertices[j]);
				}

				g_debugDraw.DrawSolidPolygon(vertices, 3, b2Color(0.f, 1.f, 0.f, 1.f));


				if (((p0 - agentBody->GetPosition()).Length() < 15.f) && cp.second.active) {
					currentSuspect = cp.first;
					/*
					if (dialogs[currentConversation].freeze) {
						run = 0;
					}
					else {
						blockInput = true;
					}
					*/
					cp.second.active = false;
				}


				if ((p0 - officer->GetPosition()).Length() < 15.f) {
					cp.second.active = false;
				}
			}

		}
		/*
		b2Vec2 officerPolygon[3];

		officerPolygon[0] = officer->GetPosition() + b2Vec2(0.f, -5.f);
		officerPolygon[1] = officer->GetPosition() + b2Vec2(5.f, 5.f);
		officerPolygon[2] = officer->GetPosition() + b2Vec2(-5.f, 5.f);

		g_debugDraw.DrawSolidPolygon(officerPolygon, 3, b2Color(0.f, 0.7f, 1.f, 1.f));
		*/



		/*
		checkpoints.erase(std::remove_if(checkpoints.begin(), checkpoints.end(),
			[&](b2Vec2 cp) { return (((cp - agentBody->GetPosition()).Length()<15.f) || ((cp - officer->GetPosition()).Length()<15.f) ); }), checkpoints.end());
			*/

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
	gSoloud.deinit(); // Clean up!
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

