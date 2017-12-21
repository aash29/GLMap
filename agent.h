//
// Created by aash29 on 05.12.17.
//

#ifndef GLMAP_AGENT_H
#define GLMAP_AGENT_H

#include <string>
#include <vector>
#include "pddltree.hpp"

using namespace std;

struct action {
  std::string name;
  std::string params;
};

class actionPrefab {
	string name;
	vector<string> parNames;
	vector<string> precondsWithVars;
	vector<string> posEffectsWithVars;
	vector<string> negEffectsWithVars;
private:
	vector<string> subsParams(vector<string> expr, vector<string> values);

public :
	actionPrefab(pddlTreeNode* action);
	vector<string> getPreconditions(string parameters);
	vector<string> getPosEffects(string parameters);
	vector<string> getNegEffects(string parameters);
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
