//
// Created by aash29 on 05.12.17.
//

#include "agent.h"
#include "utils.hpp"
#include <sstream>

using namespace std;

void agent::getAgentPos(pddlTreeNode* r1)
{
    pddlTreeNode* pos = traverseTree(r1,"at",id);

    string loc = pos->children[1].data;

    vector<string> coords = utils::tokenize(loc,'_');


    std::stringstream ss1(coords[1]);
    ss1 >> x;
    std::stringstream ss2(coords[2]);
    ss2 >> y;


}
