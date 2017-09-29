#ifndef MAP_HPP
#define MAP_HPP

#include "tesselator.h"


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


typedef std::map<std::string, building> cityMap; 

float xmin,xmax,ymin,ymax;

cityMap loadLevel(const char *name,TESStesselator* tess, rect &boundingBox)
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
		  
		  std::cout << "contour size:" << c1[j].size() << "\n";
	  
		  for (int i = 0; i<c1[j].size();i++){
		    //a2[j][2*i]=c1[j][i][0];
		    //	a2[j][2*i+1]=c1[j][i][1];

			m3[id].coords[j].push_back(c1[j][i][0]);
			m3[id].coords[j].push_back(c1[j][i][1]);
			
	    
			xmin=std::min(xmin, c1[j][i][0]);
			xmax=std::max(xmax, c1[j][i][0]);
	    
			ymin=std::min(ymin, c1[j][i][1]);
			ymax=std::max(ymax, c1[j][i][1]);
		  };
		  std::cout << "adding contour" << "\n";
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

  boundingBox.xmin = lowerx;
  boundingBox.xmax = upperx;
  boundingBox.ymin = lowery;
  boundingBox.ymax = uppery;


  float aN = 30.f*glm::pi<float>()/180;
  
  glm::mat2 r1 (cos(aN), -sin(aN), sin(aN), cos(aN) );


  debug_log().AddLog("upperx: %f \n", upperx);

      for (auto &it : m3) {
	for (int j = 0; j < it.second.coords.size(); j++) {
	  for (int i = 0; i < it.second.coords[j].size();i=i+2){
	    //it.second.coords[j][i] = -0.5f + (it.second.coords[j][i]-xmin)/(xmax-xmin);
	    //it.second.coords[j][i+1] = (-0.5f + (it.second.coords[j][i+1]-ymin)/(ymax-ymin));

	    float x = lowerx + (it.second.coords[j][i]-xmin)/(xmax-xmin)*(upperx-lowerx);
	    float y = lowery + (it.second.coords[j][i+1]-ymin)/(ymax-ymin)*(uppery-lowery);

	    glm::vec2 v1(x,y);
	    glm::vec2 v2 = r1 * v1;

	    it.second.coords[j][i] = v2[0];
	    it.second.coords[j][i+1] = v2[1];
	    
	    
	  }		  
	  tessAddContour(tess, 2, it.second.coords[j].data(), sizeof(float) * 2, round(it.second.coords[j].size()/2));
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


#endif
