//
// Created by aash29 on 20.09.18.
//

#ifndef GLMAP_ENTITY_H
#define GLMAP_ENTITY_H

#include "Box2D/Box2D.h"
#include <map>

enum entityType {player = 0, POI = 1};

class entity {
public:
    int id;
    b2Body* body;
    entityType type;
	std::string desc;
	std::string name;
	int64_t node;
};

extern std::map <unsigned int, entity> things;

#endif //GLMAP_ENTITY_H
