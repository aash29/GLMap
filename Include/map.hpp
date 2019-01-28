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
	std::string addrLetter;
    std::string type;

    std::vector<std::vector<int64_t> > renderCoords;
    std::vector<std::vector<int64_t> > collisionCoords;

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
	int64_t id;
    double lat;
    double lon;
    float x;
    float y;
	map <string, string> tags;
};

struct map_record {
    map< int64_t, node> nodes;
    map< int64_t, vector<int64_t> > pathGraph;
    map< int64_t, building> buildings;
	map< int64_t, std::vector<std::vector<int64_t> > > background;
};





void loadThings(const char *name, std::map <int, entity> &things) {
	XMLDocument* doc = new XMLDocument();

	doc->LoadFile(name);
	XMLElement* n1 = doc->FirstChildElement("osm")->FirstChildElement("enemy");
	int i = 0;
	while (n1) {

		int64_t id;
		n1->QueryAttribute("id", &id);

		int conversationId;
		n1->QueryAttribute("dialogue", &conversationId);

		const char *desc = n1->Attribute("desc");
		const char *name = n1->Attribute("name");


		things.insert(std::pair<int, entity>(i, entity()));
		things[i].id = i;
		things[i].desc = desc;
		things[i].name = name;
		things[i].node = id;
		things[i].conversationId = conversationId;
		i++;

		n1 = n1->NextSiblingElement("enemy");
	}



}

map_record loadLevel(const char *name, TESStesselator* tess, rect &gameCoords, b2World* world, bool computeBounds, bool contours = false)  {
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

        map<int64_t, vector<int64_t> > pathGraph;
        map<int64_t, building> buildings; 
		map< int64_t, std::vector<std::vector<int64_t> > > background;

        map<int64_t, node> nodes;
		map<int64_t, vector<int64_t> > ways;

        XMLDocument* doc = new XMLDocument();

        doc->LoadFile(name);

        XMLElement* n1 = doc->FirstChildElement("osm")->FirstChildElement("node");
        while (n1) {

			int64_t id;
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
            pathGraph[id] = vector<int64_t>();



			/*
            if ((tags.count("historic")>0) || (tags.count("memorial")>0)) {
                things.insert(std::pair<int, entity>(id,entity()));
                things[id].id = id;
                things[id].nodeId = id;
                things[id].type = POI;
                //things[id].x = nodes[id].x;
                //things[id].y = nodes[id].y;
            }
			*/
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


            ways[stoi(id)] = vector<int64_t>();

            XMLElement* nd1 = w1->FirstChildElement("nd");

			int64_t ref;

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

				int64_t ref;
                nd1->QueryAttribute("ref",&ref);

                //pathGraph[ref] = vector<unsigned int>();
                nd1->NextSiblingElement("nd");
				int64_t previd = ref;

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

				b1.addrStreet = tags["addr:street"];
				b1.addrNumber = tags["addr:housenumber"];
				b1.addrLetter = tags["addr:letter"];

                b1.renderCoords.push_back(vector<int64_t>());
                XMLElement* nd1 = w1->FirstChildElement("nd");

                vector<float> coordsx = vector<float>();
                vector<float> coordsy = vector<float>();

                float wind = 0.f; //determine winding
                while(nd1) {
					int64_t ref;
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
                buildings.insert(pair<int64_t, building>(atoi(id),b1));

            }



            w1 = w1->NextSiblingElement("way");
        }


        XMLElement* r1 = doc->FirstChildElement("osm")->FirstChildElement("relation");
        while (r1) {
			int64_t relId;
			r1->QueryAttribute("id", &relId);
            map<string, string> tags;
            XMLElement *tag = r1->FirstChildElement("tag");
            while (tag) {
                const char *key = tag->Attribute("k");
                const char *value = tag->Attribute("v");
                tags[key] = value;
                tag = tag->NextSiblingElement("tag");
            }

			if (tags["place"] == "background") {


				vector< vector<int64_t> > patchCoords = vector< vector<int64_t> >();

				vector<float> coordsx = vector<float>();
				vector<float> coordsy = vector<float>();

				patchCoords.push_back(vector<int64_t>());

				XMLElement* mem1 = r1->FirstChildElement("member");

				int64_t wayRef;
				mem1->QueryAttribute("ref", &wayRef);
				//первый отрезок задает направление обхода
				int64_t contourStart = ways[wayRef][0];
				int64_t prevWayEnd = -1;
				int64_t wayStart = ways[wayRef][0];
				int64_t wayEnd = ways[wayRef].back();

				while (mem1) {
					//if (mem1->Attribute("role","outer")) {
					int64_t wayRef;
					mem1->QueryAttribute("ref", &wayRef);

					const char *role = mem1->Attribute("role");

					vector<float> coordsxWay = vector<float>();
					vector<float> coordsyWay = vector<float>();
					vector<int64_t> renderCoordsWay = vector<int64_t>();

					wayStart = ways[wayRef][0];
					wayEnd = ways[wayRef].back();



					for (int64_t ni : ways[wayRef]) {
						renderCoordsWay.push_back(ni);
						coordsxWay.push_back(nodes[ni].x);
						coordsyWay.push_back(nodes[ni].y);
					}


					if (prevWayEnd == wayEnd) {
						std::reverse(renderCoordsWay.begin(), renderCoordsWay.end());
						std::reverse(coordsxWay.begin(), coordsxWay.end());
						std::reverse(coordsyWay.begin(), coordsyWay.end());
						std::swap(wayStart, wayEnd);
					}

					patchCoords.back().insert(patchCoords.back().end(), renderCoordsWay.begin(), renderCoordsWay.end());
					coordsx.insert(coordsx.end(), coordsxWay.begin(), coordsxWay.end());
					coordsy.insert(coordsy.end(), coordsyWay.begin(), coordsyWay.end());

					mem1 = mem1->NextSiblingElement("member");

					if (wayEnd == contourStart) {

						float wind = 0.f; //determine winding https://en.wikipedia.org/wiki/Shoelace_formula
						for (int i = 1; i < coordsx.size(); i++) {
							wind += (coordsx[i] - coordsx[i - 1]) * (coordsy[i] + coordsy[i - 1]);
						}
						wind += (coordsx[0] - coordsx.back()) * (coordsy[0] + coordsy.back());

						if (strcmp(role, "outer") == 0) {
							if (wind < 0) {
								std::reverse(coordsx.begin(), coordsx.end());
								std::reverse(coordsy.begin(), coordsy.end());
							}
						}
						if (strcmp(role, "inner") == 0) {
							if (wind > 0) {
								std::reverse(coordsx.begin(), coordsx.end());
								std::reverse(coordsy.begin(), coordsy.end());
							}
						}

						vector<float> coords = vector<float>();
						for (int i = 0; i < coordsx.size(); i++) {
							coords.push_back(coordsx[i]);
							coords.push_back(coordsy[i]);
						}

						//tessAddContour(tess, 2, coords.data(), sizeof(float) * 2, round(coords.size() / 2));

						coordsx.clear();
						coordsy.clear();

						if (mem1) {
							int64_t wayRef;
							mem1->QueryAttribute("ref", &wayRef);

							contourStart = ways[wayRef][0];


							patchCoords.push_back(vector<int64_t>());
						}

					}

					prevWayEnd = wayEnd;


				}
	
				background.insert(pair<int64_t, std::vector<std::vector<int64_t> > >(relId, patchCoords));


			}





			if (tags["place"]=="island") {



                building b1;

				XMLElement* mem1 = r1->FirstChildElement("member");
				while (mem1) {
					//if (mem1->Attribute("role","outer")) {
					int64_t ref;
					mem1->QueryAttribute("ref", &ref);

					XMLElement* w1 = doc->FirstChildElement("osm")->FirstChildElement("way");
					while (w1) {

						int64_t id;
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



                    b1.renderCoords.push_back(vector<int64_t>());

					float wind = 0.f; //determine winding
					while (nd1) {
						int64_t ref;
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
					
					/*

					vector<float> coordsxOuter = vector<float>();
					vector<float> coordsyOuter = vector<float>();
					vector<float> coordsOuter = vector<float>();


					vector<float> coordsxInner = vector<float>();
					vector<float> coordsyInner = vector<float>();
					vector<float> coordsInner = vector<float>();

					for (int i = 0; i < coordsx.size() - 1; i++) {

						float v1x = coordsx[i + 1] - coordsx[i];
						float v1y = coordsy[i + 1] - coordsy[i];

						float n1x = -v1y;
						float n1y = v1x;
						float nmag = sqrt(n1x*n1x + n1y*n1y);

						float nx = n1x / nmag;
						float ny = n1y / nmag;

						coordsxOuter.push_back(coordsx[i] + nx);
						coordsyOuter.push_back(coordsy[i] + ny);

						coordsxInner.push_back(coordsx[i] - nx);
						coordsyInner.push_back(coordsy[i] - ny);
					}

					std::reverse(coordsxInner.begin(), coordsxInner.end());
					std::reverse(coordsyInner.begin(), coordsyInner.end());

					//vector<float> coords = vector<float>();
					for (int i = 0; i < coordsx.size()-1; i++) {
						coordsInner.push_back(coordsxInner[i]);
						coordsInner.push_back(coordsyInner[i]);

						coordsOuter.push_back(coordsxOuter[i]);
						coordsOuter.push_back(coordsyOuter[i]);
					}



					if (contours) {
						//tessAddContour(tess, 2, coordsInner.data(), sizeof(float) * 2, round(coordsInner.size() / 2));
						//tessAddContour(tess, 2, coordsOuter.data(), sizeof(float) * 2, round(coordsOuter.size() / 2));
					}
					*/

                    b1.bounds.xmin = *(std::min_element(coordsx.begin(),coordsx.end()));
                    b1.bounds.xmax = *(std::max_element(coordsx.begin(),coordsx.end()));

                    b1.bounds.ymin = *(std::min_element(coordsy.begin(),coordsy.end()));
                    b1.bounds.ymax = *(std::max_element(coordsy.begin(),coordsy.end()));

					mem1 = mem1->NextSiblingElement("member");
				}



                buildings.insert(pair<int64_t, building>(relId,b1));
			}




            if (tags.count("building")>0) {

                building b1;

				b1.addrStreet = tags["addr:street"];
				b1.addrNumber = tags["addr:housenumber"];
				b1.addrLetter = tags["addr:letter"];

                b1.renderCoords = vector< vector<int64_t> >();

				vector<float> coordsx = vector<float>();
				vector<float> coordsy = vector<float>();

				b1.renderCoords.push_back(vector<int64_t>());
				
				XMLElement* mem1 = r1->FirstChildElement("member");

				int64_t wayRef;
				mem1->QueryAttribute("ref", &wayRef);
				//первый отрезок задает направление обхода
				int64_t contourStart = ways[wayRef][0];
				int64_t prevWayEnd = -1;
				int64_t wayStart = ways[wayRef][0];
				int64_t wayEnd = ways[wayRef].back();

                while (mem1){
                //if (mem1->Attribute("role","outer")) {
					int64_t wayRef;
                    mem1->QueryAttribute("ref", &wayRef);

					const char *role = mem1->Attribute("role");

					vector<float> coordsxWay = vector<float>();
					vector<float> coordsyWay = vector<float>();
					vector<int64_t> renderCoordsWay = vector<int64_t>();

					wayStart = ways[wayRef][0];
					wayEnd = ways[wayRef].back();

                    

                    for (int64_t ni: ways[wayRef]) {
						renderCoordsWay.push_back(ni);
						coordsxWay.push_back(nodes[ni].x);
						coordsyWay.push_back(nodes[ni].y);
                    }
					
					
					if (prevWayEnd == wayEnd) {
						std::reverse(renderCoordsWay.begin(), renderCoordsWay.end());
						std::reverse(coordsxWay.begin(), coordsxWay.end());
						std::reverse(coordsyWay.begin(), coordsyWay.end());
						std::swap(wayStart, wayEnd);
					}
					
					b1.renderCoords.back().insert(b1.renderCoords.back().end(), renderCoordsWay.begin(), renderCoordsWay.end());
					coordsx.insert(coordsx.end(), coordsxWay.begin(), coordsxWay.end());
					coordsy.insert(coordsy.end(), coordsyWay.begin(), coordsyWay.end());

					mem1 = mem1->NextSiblingElement("member");

					if (wayEnd == contourStart) {

						float wind = 0.f; //determine winding https://en.wikipedia.org/wiki/Shoelace_formula
						for (int i = 1; i < coordsx.size(); i++) {
							wind += (coordsx[i] - coordsx[i-1]) * (coordsy[i] + coordsy[i-1]);
						}
						wind += (coordsx[0] - coordsx.back()) * (coordsy[0] + coordsy.back());

						if (strcmp(role, "outer")==0) {
							if (wind < 0) {
								std::reverse(coordsx.begin(), coordsx.end());
								std::reverse(coordsy.begin(), coordsy.end());
							}
						}
						if (strcmp(role, "inner") == 0) {
							if (wind > 0) {
								std::reverse(coordsx.begin(), coordsx.end());
								std::reverse(coordsy.begin(), coordsy.end());
							}
						}
						
						vector<float> coords = vector<float>();
						for (int i = 0; i < coordsx.size(); i++) {
							coords.push_back(coordsx[i]);
							coords.push_back(coordsy[i]);
						}

						tessAddContour(tess, 2, coords.data(), sizeof(float) * 2, round(coords.size() / 2));

						coordsx.clear();
						coordsy.clear();

						if (mem1) {
							int64_t wayRef;
							mem1->QueryAttribute("ref", &wayRef);

							contourStart = ways[wayRef][0];
			

							b1.renderCoords.push_back(vector<int64_t>());
						}

					}

					prevWayEnd = wayEnd;


                }

				buildings.insert(pair<int64_t, building>(relId, b1));


            }




            r1 = r1->NextSiblingElement("relation");

        }

        map_record mapRecord = {nodes, pathGraph, buildings,background};
        return mapRecord;
    }
};
/*
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
*/
/*
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
*/

static int pnpoly(int nvert, double *vertx, double *verty, double testx, double testy)
{
    int i, j, c = 0;
    for (i = 0, j = nvert - 1; i < nvert; j = i++) {
        if (((verty[i]>testy) != (verty[j]>testy)) && (testx < (vertx[j] - vertx[i]) * (testy - verty[i]) / (verty[j] - verty[i]) + vertx[i]))
            c = !c;
    }
    return c;
};

/*
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
        if (pnpoly(numVert, vertx, verty, testx, testy)>0)
        {
            return id1;
        }
        delete vertx;
        delete verty;


    }
    return std::string("none");

};
*/

#endif
