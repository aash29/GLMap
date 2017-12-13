//
// Created by aash29 on 05.12.17.
//

#ifndef GLMAP_AGENT_H
#define GLMAP_AGENT_H

#include <string>
#include "pddltree.hpp"

struct action {
  std::string name;
  std::string params;
};

class agent {
public:
    int x;
    int y;
    std::string id;

    std::vector<action> plan;

    void update();

    void getAgentPos(pddlTreeNode* state);
};


#endif //GLMAP_AGENT_H
