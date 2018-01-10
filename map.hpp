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

struct building
{
  std::string id;
  std::string name;
  std::string addrNumber;
  std::string addrStreet;
  std::vector<std::vector<float> > coords;
};

struct rect
{
  float xmin;
  float xmax;
  float ymin;
  float ymax;
};


struct polygon
{
  int nvert;
  double *vertx;
  double *verty;
};

typedef std::map<std::string, building> cityMap; 

float xmin,xmax,ymin,ymax;

void* stdAlloc(void* userData, unsigned int size)
{
	int* allocated = (int*)userData;
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

void* poolAlloc(void* userData, unsigned int size)
{
	struct MemPool* pool = (struct MemPool*)userData;
	size = (size + 0x7) & ~0x7;
	if (pool->size + size < pool->cap)
	{
		unsigned char* ptr = pool->buf + pool->size;
		pool->size += size;
		return ptr;
	}
	printf("out of mem: %d < %d!\n", pool->size + size, pool->cap);
	return 0;
}

void poolFree(void* userData, void* ptr)
{
	// empty
	TESS_NOTUSED(userData);
	TESS_NOTUSED(ptr);
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

  auto f1 = jsonObj.find("features");


  std::map<std::string, building> m3;

  
  if (f1 != jsonObj.end()) {
    //m_force = m_forceLeft;
    //std::cout << "found features";
    //std::cout << ((*f1)[0]).dump(4);


	std::map<std::string, float**> m2;
	float** a2;
	float* a1[10];


	xmin = 30.2882633f;
	xmax = 30.2882633f;
	ymin = 59.9379525f;
	ymax = 59.9379525f;


	singlePolygon.nvert = 0;
	

    for (nlohmann::json::iterator it = (*f1).begin(); it != (*f1).end(); ++it) {
      if (((*it)["properties"]).find("building") != ((*it)["properties"]).end()) {
	//std::cout << "id:" << (*it)["properties"]["id"] << "\n";
		//std::cout <<  (*it)["geometry"]["type"];
		
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
		

		for (int j = 0; j<c1.size();j++){

		  a2[j] = new float[c1[j].size()*2];
		  m3[id].coords.push_back(std::vector<float>());
		  
		  //std::cout << "contour size:" << c1[j].size() << "\n";

		  // zero vert
		  singlePolygon.nvert++;
	  
		  for (int i = 0; i<c1[j].size();i++){
		    //a2[j][2*i]=c1[j][i][0];
		    //	a2[j][2*i+1]=c1[j][i][1];

			m3[id].coords[j].push_back(c1[j][i][0]);
			m3[id].coords[j].push_back(c1[j][i][1]);
			
			singlePolygon.nvert++;

			xmin=std::min(xmin, c1[j][i][0]);
			xmax=std::max(xmax, c1[j][i][0]);
	    
			ymin=std::min(ymin, c1[j][i][1]);
			ymax=std::max(ymax, c1[j][i][1]);
		  };
		  //std::cout << "adding contour" << "\n";
		  singlePolygon.nvert++;  // zero vert

		}

	
		};
      }
      /*
      if ((*it)["properties"]["type"]=="route"){
	std::cout << *it << '\n';
      }
      */
    }
  }



  float lowerx,lowery,upperx,uppery;

  lowery = 0.f;
  uppery = 1.f;

  lowerx = 0.f * (xmax-xmin)/(ymax-ymin); 
  upperx = 1.f * (xmax-xmin)/(ymax-ymin)* 0.5f;

  boundingBox.xmin = 0.f;
  boundingBox.xmax = 0.f;
  boundingBox.ymin = 0.f;
  boundingBox.ymax = 0.f;


  float aN = 30.f*glm::pi<float>()/180;
  
  glm::mat2 r1 (cos(aN), -sin(aN), sin(aN), cos(aN) );


  singlePolygon.vertx = new double[singlePolygon.nvert];
  singlePolygon.verty = new double[singlePolygon.nvert];
  
  int spCounter = 0;



  debug_log().AddLog("upperx: %f \n", upperx);

    for (auto &it : m3) {
	for (int j = 0; j < it.second.coords.size(); j++) {
	singlePolygon.vertx[spCounter] = 0.f;
	singlePolygon.vertx[spCounter] = 0.f;
	spCounter++;
	  for (int i = 0; i < it.second.coords[j].size();i=i+2){

	    float x = lowerx + (it.second.coords[j][i]-xmin)/(xmax-xmin)*(upperx-lowerx);
	    float y = lowery + (it.second.coords[j][i+1]-ymin)/(ymax-ymin)*(uppery-lowery);

	    glm::vec2 v1(x,y);
	    glm::vec2 v2 = r1 * v1;

	    singlePolygon.vertx[spCounter] = v2[0];
	    singlePolygon.verty[spCounter] = v2[1];
	    spCounter++;
	    
	    it.second.coords[j][i] = v2[0];
	    it.second.coords[j][i+1] = v2[1];

	    boundingBox.xmin = std::min(boundingBox.xmin,v2[0]);
	    boundingBox.xmax = std::max(boundingBox.xmax,v2[0]);

	    boundingBox.ymin = std::min(boundingBox.ymin,v2[1]);
	    boundingBox.ymax = std::max(boundingBox.ymax,v2[1]);
	    
	    
	  }		  
	  tessAddContour(tess, 2, it.second.coords[j].data(), sizeof(float) * 2, round(it.second.coords[j].size()/2));

	  singlePolygon.vertx[spCounter] = 0.f;
	  singlePolygon.vertx[spCounter] = 0.f;
	  spCounter++;

	  
	}

	
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
