#ifndef MAP_HPP
#define MAP_HPP

#include "tesselator.h"
//#include "../json.hpp"
#include <fstream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
//#include "../camera.hpp"
using namespace std;

#include "tinyxml2.h"
using namespace tinyxml2;

#include "pathnode.hpp"
#include <Box2D/Box2D.h>

#include "entity.h"

#include <algorithm>

struct rect
{
    double xmin =+INFINITY,xmax=-INFINITY,ymin=+INFINITY,ymax=-INFINITY;
};

struct building
{
    std::string id;
    std::string name;
    std::string addrNumber;
    std::string addrStreet;
    std::string type;

    std::vector<std::vector<unsigned int> > renderCoords;
    std::vector<std::vector<unsigned int> > collisionCoords;

    rect bounds;

};


struct polygon
{
    int nvert;
    double *vertx;
    double *verty;
};


float xmin,xmax,ymin,ymax;

struct node {
    unsigned int id;
    double lat;
    double lon;
    float x;
    float y;
	map <string, string> tags;
};

struct map_record {
    map<unsigned int, node> nodes;
    map<unsigned int, vector<unsigned int> > pathGraph;
    map<unsigned int, building> buildings;
};



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

map_record loadLevel(const char *name, TESStesselator* tess, rect &gameCoords, b2World* world, bool computeBounds)  {
    std::string m_currentLevel = std::string(name);

    int di = m_currentLevel.find_last_of('.');

    std::string ext = m_currentLevel.substr(di+1, m_currentLevel.size());

    double lowery = gameCoords.ymin;
    double lowerx = gameCoords.xmin;

    double upperx = gameCoords.xmax;
    double uppery = gameCoords.ymax;


    double xmin = +INFINITY;
    double xmax = -INFINITY;
    double ymin = +INFINITY;
    double ymax = -INFINITY;
    if (ext=="osm"){

        map<unsigned int, vector<unsigned int> > pathGraph;
        map<unsigned int, building> buildings;

        map<unsigned int, node> nodes;
		map<unsigned int, vector<unsigned int> > ways;

        XMLDocument* doc = new XMLDocument();

        doc->LoadFile(name);

        XMLElement* n1 = doc->FirstChildElement("osm")->FirstChildElement("node");
        while (n1) {

            unsigned int id;
            double lat;
            double lon;
            n1->QueryAttribute("id", &id);
            n1->QueryAttribute("lat", &lat);
            n1->QueryAttribute("lon", &lon);


            xmin = std::min(xmin, lon);
            xmax = std::max(xmax, lon);

            ymin = std::min(ymin, lat);
            ymax = std::max(ymax, lat);


			map<string, string> tags;
			XMLElement* tag = n1->FirstChildElement("tag");
			while (tag) {
				const char *key = tag->Attribute("k");
				const char *value = tag->Attribute("v");
				tags[key] = value;
				tag = tag->NextSiblingElement("tag");
			}
			

            node node1 = {id, lat, lon, lat, lon, map<string, string>(tags)};
			
            nodes[id] = node1;
            pathGraph[id] = vector<unsigned int>();




            if ((tags.count("historic")>0) || (tags.count("memorial")>0)) {
                things.insert(std::pair<int, entity>(id,entity()));
                things[id].id = id;
                things[id].nodeId = id;
                things[id].type = POI;
                //things[id].x = &nodes[id].x;
                //things[id].y = &nodes[id].y;
            }

            n1 = n1->NextSiblingElement("node");

        }


        double width = xmax - xmin;
        double height = ymax - ymin;

        double xcp = (xmax + xmin)/2;
        double ycp = (ymax + ymin)/2;

        xmax = xcp + width/2*1.2f;
        xmin = xcp - width/2*1.2f;

        ymax = ycp + height/2*1.2f;
        ymin = ycp - height/2*1.2f;

        if (!computeBounds) {
            XMLElement* bounds = doc->FirstChildElement("osm")->FirstChildElement("bounds");
            bounds->QueryAttribute("minlon",&xmin);
            bounds->QueryAttribute("maxlon",&xmax);
            bounds->QueryAttribute("minlat",&ymin);
            bounds->QueryAttribute("maxlat",&ymax);
        }

        //for (map<unsigned int, node>::iterator n1 = nodes.begin(); n1 != nodes) {
        for (auto& n1 : nodes){
            n1.second.x = lowerx + (n1.second.lon - xmin) / (xmax - xmin) * (upperx - lowerx);
            n1.second.y = lowery + (n1.second.lat - ymin) / (ymax - ymin) * (uppery - lowery);
        }



        XMLElement* w1 = doc->FirstChildElement("osm")->FirstChildElement("way");
        while (w1) {
            const char* id = w1->Attribute("id");


            ways[stoi(id)] = vector<unsigned int>();

            XMLElement* nd1 = w1->FirstChildElement("nd");

            unsigned int ref;

            while (nd1) {
                nd1->QueryAttribute("ref", &ref);
                ways[stoi(id)].push_back(ref);
                nd1 = nd1->NextSiblingElement("nd");
            }


            map<string, string> tags;
            XMLElement* tag = w1->FirstChildElement("tag");
            while (tag) {
                const char *key = tag->Attribute("k");
                const char *value = tag->Attribute("v");
                tags[key] = value;
                tag = tag->NextSiblingElement("tag");
            }
            if ((tags.count("highway")>0) || (tags.count("area:highway"))){
                XMLElement* nd1 = w1->FirstChildElement("nd");

                unsigned int ref;
                nd1->QueryAttribute("ref",&ref);

                //pathGraph[ref] = vector<unsigned int>();
                nd1->NextSiblingElement("nd");
                unsigned int previd = ref;

                while(nd1) {
                    //unsigned int id;
                    nd1->QueryAttribute("ref",&ref);
                    //pathGraph[ref] = vector<unsigned int>();
                    pathGraph[ref].push_back(previd);
                    pathGraph[previd].push_back(ref);
                    previd = ref;

                    nd1 = nd1->NextSiblingElement("nd");
                }
            }

            if (tags.count("building")>0) {

                building b1;

                b1.renderCoords.push_back(vector<unsigned int>());
                XMLElement* nd1 = w1->FirstChildElement("nd");

                vector<float> coordsx = vector<float>();
                vector<float> coordsy = vector<float>();

                float wind = 0.f; //determine winding
                while(nd1) {
                    unsigned int ref;
                    nd1->QueryAttribute("ref",&ref);

                    b1.renderCoords.back().push_back(ref);
                    coordsx.push_back(nodes[ref].x);
                    coordsy.push_back(nodes[ref].y);
                    if (coordsx.size()>1) {
                        wind += (coordsx.end()[-1] - coordsx.end()[-2]) * (coordsy.end()[-1] + coordsy.end()[-2]);
                    }

                    nd1 = nd1->NextSiblingElement("nd");
                }
                if (wind < 0) {
                    std::reverse(coordsx.begin(), coordsx.end());
                    std::reverse(coordsy.begin(), coordsy.end());
                }
                vector<float> coords = vector<float>();
                for (int i = 0; i < coordsx.size(); i++) {
                    coords.push_back(coordsx[i]);
                    coords.push_back(coordsy[i]);
                }



                b1.bounds.xmin = *(std::min_element(coordsx.begin(),coordsx.end()));
                b1.bounds.xmax = *(std::max_element(coordsx.begin(),coordsx.end()));

                b1.bounds.ymin = *(std::min_element(coordsy.begin(),coordsy.end()));
                b1.bounds.ymax = *(std::max_element(coordsy.begin(),coordsy.end()));


                tessAddContour(tess, 2,coords.data(), sizeof(float) * 2, round(coords.size() / 2));
                buildings.insert(pair<int, building>(id,b1));

            }



            w1 = w1->NextSiblingElement("way");
        }


        XMLElement* r1 = doc->FirstChildElement("osm")->FirstChildElement("relation");
        while (r1) {
			const char* relId = r1->Attribute("id");
            map<string, string> tags;
            XMLElement *tag = r1->FirstChildElement("tag");
            while (tag) {
                const char *key = tag->Attribute("k");
                const char *value = tag->Attribute("v");
                tags[key] = value;
                tag = tag->NextSiblingElement("tag");
            }

			if (tags["place"]=="island") {



                building b1;

				XMLElement* mem1 = r1->FirstChildElement("member");
				while (mem1) {
					//if (mem1->Attribute("role","outer")) {
					int ref;
					mem1->QueryAttribute("ref", &ref);

					XMLElement* w1 = doc->FirstChildElement("osm")->FirstChildElement("way");
					while (w1) {

						int id;
						w1->QueryAttribute("id", &id);
						if (id == ref) break;
						w1 = w1->NextSiblingElement("way");
					}

					if (not w1) {
						mem1 = mem1->NextSiblingElement("member");
						continue;
					}


					XMLElement* nd1 = w1->FirstChildElement("nd");
					vector<float> coordsx = vector<float>();
					vector<float> coordsy = vector<float>();



                    b1.renderCoords.push_back(vector<unsigned int>());

					float wind = 0.f; //determine winding
					while (nd1) {
						unsigned int ref;
						nd1->QueryAttribute("ref", &ref);

						b1.renderCoords.back().push_back(ref);
						coordsx.push_back(nodes[ref].x);
						coordsy.push_back(nodes[ref].y);
						if (coordsx.size() > 1) {
							wind += (coordsx.end()[-1] - coordsx.end()[-2]) * (coordsy.end()[-1] + coordsy.end()[-2]);
						}

						nd1 = nd1->NextSiblingElement("nd");
					}
					/*
					if (mem1->Attribute("role", "outer")) {
						if (wind < 0) {
							std::reverse(coordsx.begin(), coordsx.end());
							std::reverse(coordsy.begin(), coordsy.end());
						}
					}
					else {
						if (wind > 0) {
							std::reverse(coordsx.begin(), coordsx.end());
							std::reverse(coordsy.begin(), coordsy.end());
						}
					}
					*/
					{
						b2BodyDef bd;
						b2Body* ground = world->CreateBody(&bd);

						b2Vec2* vs;
						vs = new b2Vec2[coordsx.size()];

						for (int i = 0; i < coordsx.size(); i++) {
							vs[i].Set(coordsx[i], coordsy[i]);
						}
						b2ChainShape shape;
						shape.CreateChain(vs, coordsx.size());
						ground->CreateFixture(&shape, 0.0f);
					}


                    b1.bounds.xmin = *(std::min_element(coordsx.begin(),coordsx.end()));
                    b1.bounds.xmax = *(std::max_element(coordsx.begin(),coordsx.end()));

                    b1.bounds.ymin = *(std::min_element(coordsy.begin(),coordsy.end()));
                    b1.bounds.ymax = *(std::max_element(coordsy.begin(),coordsy.end()));

					mem1 = mem1->NextSiblingElement("member");
				}



                buildings.insert(pair<int, building>(relId,b1));
			}




            if (tags.count("building")>0) {

                building b1;

                b1.renderCoords= vector< vector<unsigned int> >();

                XMLElement* mem1 = r1->FirstChildElement("member");
                while (mem1){
                //if (mem1->Attribute("role","outer")) {
                    int ref;
                    mem1->QueryAttribute("ref", &ref);

                    /*
                    XMLElement* w1 = doc->FirstChildElement("osm")->FirstChildElement("way");
                    while (w1){

                        int id;
                        w1->QueryAttribute("id", &id);
                        if (id==ref) break;
                        w1 = w1->NextSiblingElement("way");
                    }

                    if (not w1) break;
                    */

                    //XMLElement* nd1 = w1->FirstChildElement("nd");
                    vector<float> coordsx = vector<float>();
                    vector<float> coordsy = vector<float>();

                    b1.renderCoords.push_back(vector<unsigned int>());

                    float wind = 0.f; //determine winding
                    for (int ni: ways[ref]) {

                        unsigned int ref;
                        b1.renderCoords.back().push_back(ref);
                        coordsx.push_back(nodes[ni].x);
                        coordsy.push_back(nodes[ni].y);
                        if (coordsx.size()>1){
                            wind += (coordsx.end()[-1] - coordsx.end()[-2]) * (coordsy.end()[-1] + coordsy.end()[-2]);
                        }
                    }
					if (mem1->Attribute("role", "outer")) {
						if (wind < 0) {
							std::reverse(coordsx.begin(), coordsx.end());
							std::reverse(coordsy.begin(), coordsy.end());
						}
					} else {
						if (wind > 0) {
							std::reverse(coordsx.begin(), coordsx.end());
							std::reverse(coordsy.begin(), coordsy.end());
						}
					}

                    vector<float> coords = vector<float>();
                    for (int i= 0; i < coordsx.size(); i++) {
                        coords.push_back(coordsx[i]);
                        coords.push_back(coordsy[i]);
                    }

                    tessAddContour(tess, 2, coords.data(), sizeof(float) * 2, round(coords.size() / 2));
                //}
					/*
					{
						b2BodyDef bd;
						b2Body* ground = world->CreateBody(&bd);

						b2Vec2* vs;
						vs = new b2Vec2[coordsx.size()];

						for (int i = 0; i < coordsx.size() - 1; i++) {
							vs[i].Set(coordsx[i], coordsy[i]);
						}
						b2ChainShape shape;
						shape.CreateLoop(vs, coordsx.size() - 1);
						ground->CreateFixture(&shape, 0.0f);
					}
					*/

                    mem1 = mem1->NextSiblingElement("member");
                }
            }
            r1 = r1->NextSiblingElement("relation");
        }

        map_record mapRecord = {nodes, pathGraph, buildings};
        return mapRecord;
    }
    /*
    else {

          nlohmann::json jsonObj;

          std::ifstream file;
          file.open(std::string(name), std::ios::in);
          if (file) {
              printf("file open \n");
              jsonObj << file;
              file.close();
          }


          float aN = 0.f;


          if (jsonObj.find("northAngle") != jsonObj.end()) {
              auto northAngle = jsonObj.find("northAngle");
              aN = *northAngle;
              aN = aN * glm::pi<float>() / 180;;
          };


          auto f1 = jsonObj.find("features");


          std::map<std::string, building> m3;


          if (f1 != jsonObj.end()) {
              //m_force = m_forceLeft;
              //std::cout << "found features";
              //std::cout << ((*f1)[0]).dump(4);


              std::map<std::string, float **> m2;
              float **a2;
              float *a1[10];





              singlePolygon.nvert = 0;


              for (nlohmann::json::iterator it = (*f1).begin(); it != (*f1).end(); ++it) {
                  if (((*it)["properties"]).find("building") != ((*it)["properties"]).end()) {
                      std::cout << "id:" << (*it)["properties"]["id"] << "\n";
                      std::cout << (*it)["geometry"]["type"];

                      if ((*it)["geometry"]["type"] == "Polygon") {

                          std::vector<std::vector<std::vector<float> > > c1 = (*it)["geometry"]["coordinates"];


                          a2 = new float *[c1.size()];
                          std::string id = (*it)["properties"]["id"];

                          m3[id].coords = std::vector<std::vector<float> >();
                          m3[id].id = id;

                          if (((*it)["properties"]).find("name") != ((*it)["properties"]).end())
                              m3[id].name = (*it)["properties"]["name"];
                          if (((*it)["properties"]).find("addr:street") != ((*it)["properties"]).end())
                              m3[id].addrStreet = (*it)["properties"]["addr:street"];
                          if (((*it)["properties"]).find("addr:housenumber") != ((*it)["properties"]).end())
                              m3[id].addrNumber = (*it)["properties"]["addr:housenumber"];
                          if (((*it)["properties"]).find("type") != ((*it)["properties"]).end())
                              m3[id].type = (*it)["properties"]["type"];

                          m3[id].bounds = {std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max(),
                                           std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max()};


                          for (int j = 0; j < c1.size(); j++) {

                              m3[id].coords.push_back(std::vector<float>());

                              std::cout << "contour size:" << c1[j].size() << "\n";

                              // zero vert
                              singlePolygon.nvert++;


                              for (int i = 0; i < c1[j].size(); i++) {

                                  m3[id].coords[j].push_back(c1[j][i][0]);
                                  m3[id].coords[j].push_back(c1[j][i][1]);

                                  singlePolygon.nvert++;

                                  m3[id].bounds.xmin = std::min(m3[id].bounds.xmin, c1[j][i][0]);
                                  m3[id].bounds.xmax = std::max(m3[id].bounds.xmax, c1[j][i][0]);

                                  xmin = std::min(xmin, c1[j][i][0]);
                                  xmax = std::max(xmax, c1[j][i][0]);

                                  m3[id].bounds.ymin = std::min(m3[id].bounds.ymin, c1[j][i][1]);
                                  m3[id].bounds.ymax = std::max(m3[id].bounds.ymax, c1[j][i][1]);

                                  ymin = std::min(ymin, c1[j][i][1]);
                                  ymax = std::max(ymax, c1[j][i][1]);
                              };


                              singlePolygon.nvert++;  // zero vert

                          }


                      };
                  }
              }

          }

          if (!computeBounds) {
              auto f0 = jsonObj.find("mapBounds");
              std::vector<float> c1 = (*f0);

              xmin = c1[0];
              xmax = c1[1];
              ymin = c1[2];
              ymax = c1[3];
          }







          float frame[8] = {lowerx, lowery, lowerx, uppery, upperx, uppery, upperx, lowerx};


          tessAddContour(tess, 2, frame, sizeof(float) * 2, 4);


          glm::mat2 r1(cos(aN), -sin(aN), sin(aN), cos(aN));


          singlePolygon.vertx = new double[singlePolygon.nvert];
          singlePolygon.verty = new double[singlePolygon.nvert];

          int spCounter = 0;
          int buildingCounter = 7;


          debug_log().AddLog("upperx: %f \n", upperx);


          for (auto &it : m3) {
              std::vector<float> unCol = std::vector<float>();
              it.second.firstVertexIndex = buildingCounter;

              for (int j = 0; j < it.second.coords.size(); j++) {

                  singlePolygon.vertx[spCounter] = 0.f;
                  singlePolygon.verty[spCounter] = 0.f;
                  spCounter++;
                  buildingCounter++;

                  for (int i = 0; i < it.second.coords[j].size(); i = i + 2) {

                      float x = lowerx + (it.second.coords[j][i] - xmin) / (xmax - xmin) * (upperx - lowerx);
                      float y = lowery + (it.second.coords[j][i + 1] - ymin) / (ymax - ymin) * (uppery - lowery);

                      glm::vec2 v1(x, y);
                      glm::vec2 v2 = r1 * v1;


                      unCol.push_back(v2[0]);
                      unCol.push_back(v2[1]);


                      singlePolygon.vertx[spCounter] = v2[0];
                      singlePolygon.verty[spCounter] = v2[1];
                      spCounter++;
                      buildingCounter++;

                      it.second.coords[j][i] = v2[0];
                      it.second.coords[j][i + 1] = v2[1];

                      if (j == 0) {
                          it.second.anchorx += v2[0];
                          it.second.anchory += v2[1];
                      }
                  }
                  if (j == 0) {
                      it.second.anchorx /= (it.second.coords[j].size() / 2);
                      it.second.anchory /= (it.second.coords[j].size() / 2);
                  }


                  tessAddContour(tess, 2, it.second.coords[j].data(), sizeof(float) * 2,
                                 round(it.second.coords[j].size() / 2));


                  unCol.push_back(0.f);
                  unCol.push_back(0.f);

                  singlePolygon.vertx[spCounter] = 0.f;
                  singlePolygon.verty[spCounter] = 0.f;
                  spCounter++;
              }

              float *cont1 = unCol.data();
              int numVert = round(unCol.size() / 2);

              it.second.vertx = new double[numVert];
              it.second.verty = new double[numVert];

              for (int i = 0; i < numVert; i++) {
                  it.second.vertx[i] = cont1[2 * i];
                  it.second.verty[i] = cont1[2 * i + 1];
              }

              it.second.numVert = numVert;

              it.second.lastVertexIndex = buildingCounter;


          }


          return m3;
      }
     */
};

int buildingIndex(cityMap city, int index, std::string & id)
{
    int b1 = 0;
    for (auto &it : city) {
        if ((index >= it.second.firstVertexIndex) && (index <= it.second.lastVertexIndex)) {
            id = it.second.id;
            return b1;
        }
        b1++;
    }

    return -1;


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
    //rotN = glm::rotate(rotN, glm::radians(-g_camera.angleNorth), glm::vec3(0.0f, 0.0f, 1.0f));

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
