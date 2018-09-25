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
    int nodeId;
    b2Body* body;
    entityType type;
};

extern std::map <int, entity> things;

#endif //GLMAP_ENTITY_H
