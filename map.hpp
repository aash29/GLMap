#ifndef MAP_HPP
#define MAP_HPP

#include "tesselator.h"
#include <map>
#include "json.hpp"
#include <fstream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "camera.hpp"
using namespace std;

struct rect
{
    float xmin,xmax,ymin,ymax;
};

struct building
{
  std::string id;
  std::string name;
  std::string addrNumber;
  std::string addrStreet;
  std::string type;
  
  std::vector<std::vector<float> > coords;

  double *vertx;
	double *verty;
	int numVert;

  rect bounds;

};


struct polygon
{
  int nvert;
  double *vertx;
  double *verty;
};

typedef std::map<std::string, building> cityMap; 

float xmin,xmax,ymin,ymax;


void loadGrid(const char *name, int& xgrid, int& ygrid)
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



    auto fg = jsonObj.find("grid");
    std::vector<int> c1 =  (*fg);

    xgrid = c1[0];
    ygrid = c1[1];


}

cityMap loadLevel(const char *name, TESStesselator* tess, rect &boundingBox, polygon &singlePolygon)
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



	float aN = 0.f;

	auto f0 = jsonObj.find("mapBounds");
    std::vector<float> c1 =  (*f0);

    xmin = c1[0];
    xmax = c1[1];
    ymin = c1[2];
    ymax = c1[3];



	if (jsonObj.find("northAngle") != jsonObj.end()) {
		auto northAngle = jsonObj.find("northAngle");
		aN = *northAngle;
		aN =aN * glm::pi<float>()/180;;
	};



  auto f1 = jsonObj.find("features");


  std::map<std::string, building> m3;

  
  if (f1 != jsonObj.end()) {
    //m_force = m_forceLeft;
    //std::cout << "found features";
    //std::cout << ((*f1)[0]).dump(4);


	std::map<std::string, float**> m2;
	float** a2;
	float* a1[10];

	/*
	xmin = 30.2882633f;
	xmax = 30.2882633f;
	ymin = 59.9379525f;
	ymax = 59.9379525f;
	*/


	singlePolygon.nvert = 0;
	

    for (nlohmann::json::iterator it = (*f1).begin(); it != (*f1).end(); ++it) {
      if (((*it)["properties"]).find("building") != ((*it)["properties"]).end() )  {
	std::cout << "id:" << (*it)["properties"]["id"] << "\n";
		std::cout <<  (*it)["geometry"]["type"];
		
		if ((*it)["geometry"]["type"]=="Polygon"){

		std::vector< std::vector<std::vector<float> > > c1 =  (*it)["geometry"]["coordinates"];


		a2 = new float*[c1.size()];
		std::string id = (*it)["properties"]["id"];
		
		m3[id].coords=std::vector <std::vector <float> >();
		m3[id].id = id;

		if (((*it)["properties"]).find("name") != ((*it)["properties"]).end())
		  m3[id].name = (*it)["properties"]["name"];
		if (((*it)["properties"]).find("addr:street") != ((*it)["properties"]).end())
		  m3[id].addrStreet = (*it)["properties"]["addr:street"];
		if (((*it)["properties"]).find("addr:housenumber") != ((*it)["properties"]).end())
		  m3[id].addrNumber = (*it)["properties"]["addr:housenumber"];
		if (((*it)["properties"]).find("type") != ((*it)["properties"]).end())
		  m3[id].type = (*it)["properties"]["type"];

		m3[id].bounds = {std::numeric_limits<float>::lowest(),std::numeric_limits<float>::max(),
							   std::numeric_limits<float>::lowest(),std::numeric_limits<float>::max()};


		for (int j = 0; j<c1.size();j++){

		  m3[id].coords.push_back(std::vector<float>());
		  
		  std::cout << "contour size:" << c1[j].size() << "\n";

		  // zero vert
		  singlePolygon.nvert++;


		  for (int i = 0; i<c1[j].size();i++){

			  //unCol.insert(unCol.end(),c1[j].begin(),c1[j].end());


			  m3[id].coords[j].push_back(c1[j][i][0]);
			m3[id].coords[j].push_back(c1[j][i][1]);
			
			singlePolygon.nvert++;

			  m3[id].bounds.xmin=std::min(m3[id].bounds.xmin, c1[j][i][0]);
			  m3[id].bounds.xmax=std::max(m3[id].bounds.xmax, c1[j][i][0]);

			  m3[id].bounds.ymin=std::min(m3[id].bounds.ymin, c1[j][i][1]);
			  m3[id].bounds.ymax=std::max(m3[id].bounds.ymax, c1[j][i][1]);
		  };



			singlePolygon.nvert++;  // zero vert

		}


	
		};
      }
    }
  }



  float lowerx,lowery,upperx,uppery;


  lowery = boundingBox.ymin;
  lowerx = boundingBox.xmin;

  upperx = boundingBox.xmax;
  uppery = boundingBox.ymax;




  float frame[8] = { lowerx,lowery,lowerx,uppery, upperx, uppery, upperx, lowerx };


  tessAddContour(tess, 2, frame, sizeof(float) * 2, 4);



  glm::mat2 r1 (cos(aN), -sin(aN), sin(aN), cos(aN) );


  singlePolygon.vertx = new double[singlePolygon.nvert];
  singlePolygon.verty = new double[singlePolygon.nvert];
  
  int spCounter = 0;



  debug_log().AddLog("upperx: %f \n", upperx);

    for (auto &it : m3) {

	std::vector<float> unCol = std::vector<float>();

	for (int j = 0; j < it.second.coords.size(); j++) {


	singlePolygon.vertx[spCounter] = 0.f;
	singlePolygon.verty[spCounter] = 0.f;
	spCounter++;
	  for (int i = 0; i < it.second.coords[j].size();i=i+2){

	    float x = lowerx + (it.second.coords[j][i]-xmin)/(xmax-xmin)*(upperx-lowerx);
	    float y = lowery + (it.second.coords[j][i+1]-ymin)/(ymax-ymin)*(uppery-lowery);

	    glm::vec2 v1(x,y);
	    glm::vec2 v2 = r1 * v1;


		  unCol.push_back(v2[0]);
		  unCol.push_back(v2[1]);


	    singlePolygon.vertx[spCounter] = v2[0];
	    singlePolygon.verty[spCounter] = v2[1];
	    spCounter++;
	    
	    it.second.coords[j][i] = v2[0];
	    it.second.coords[j][i+1] = v2[1];
	  }

	  tessAddContour(tess, 2, it.second.coords[j].data(), sizeof(float) * 2, round(it.second.coords[j].size()/2));

		unCol.push_back(0.f);
		unCol.push_back(0.f);

	  singlePolygon.vertx[spCounter] = 0.f;
	  singlePolygon.verty[spCounter] = 0.f;
	  spCounter++;
	}

		float* cont1 = unCol.data();
		int numVert = round(unCol.size()/2);

		it.second.vertx = new double[numVert];
		it.second.verty = new double[numVert];

		for (int i = 0; i<numVert; i++){
			it.second.vertx[i]=cont1[2*i];
			it.second.verty[i]=cont1[2*i+1];
		}

		it.second.numVert = numVert;

	
	}


    return m3;
};

std::vector<float> getOutlines(cityMap city1)
{
  std::vector<float> unDraw = std::vector<float>();
  
  for (auto b1: city1)
    {
      std::string id1 = b1.first;
      for (auto it: city1[id1].coords)
	{
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
	};
    };
  return unDraw;  

};

static int pnpoly(int nvert, double *vertx, double *verty, double testx, double testy)
{
	int i, j, c = 0;
	for (i = 0, j = nvert - 1; i < nvert; j = i++) {
		if (((verty[i]>testy) != (verty[j]>testy)) && (testx < (vertx[j] - vertx[i]) * (testy - verty[i]) / (verty[j] - verty[i]) + vertx[i]))
			c = !c;
	}
	return c;
};

std::string selectBuilding( map<string, building> city, float testx, float testy)
{
	glm::mat4 rotN;
	rotN = glm::rotate(rotN, glm::radians(-g_camera.angleNorth), glm::vec3(0.0f, 0.0f, 1.0f));

	std::vector<float> unCol = std::vector<float>();
	std::vector<float> unDraw = std::vector<float>();

	unCol.push_back(0.f);
	unCol.push_back(0.f);

	for (auto b1 : city)
	{
		std::string id1 = b1.first;
		for (auto it : city[id1].coords)
		{
			//numVert+=round(it.size()/2);
			unCol.insert(unCol.end(), it.begin(), it.end());
			unDraw.push_back(it[0]);
			unDraw.push_back(it[1]);
			for (int i = 2;i<it.size();i = i + 2)
			{
				unDraw.push_back(it[i]);
				unDraw.push_back(it[i + 1]);
				unDraw.push_back(it[i]);
				unDraw.push_back(it[i + 1]);
			}
			unDraw.push_back(it[0]);
			unDraw.push_back(it[1]);
			//unDraw.insert(unDraw.end(),it.begin(),it.end());
			unCol.push_back(0.f);
			unCol.push_back(0.f);
			//numVert++;
		};


		float* cont1 = unCol.data();
		int numVert = round(unCol.size() / 2);

		double* vertx = new double[numVert];
		double* verty = new double[numVert];

		for (int i = 0; i<numVert; i++) {
			vertx[i] = cont1[2 * i];
			verty[i] = cont1[2 * i + 1];
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


#endif
