//
// Created by aash29 on 20.09.18.
//

#ifndef GLMAP_ENTITY_H
#define GLMAP_ENTITY_H


class entity {
public:
    int id;
    int nodeId;
};

extern std::map <int, entity> things;

#endif //GLMAP_ENTITY_H
