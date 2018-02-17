//
// Created by aash29 on 05.12.17.
//

#ifndef GLMAP_AGENT_H
#define GLMAP_AGENT_H

#include <string>
#include <vector>
#include "pddltree.hpp"
#include <unordered_set>
#include <functional>


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
	void init(pddlTreeNode* action);
	vector<string> getPreconditions(string parameters);
	vector<string> getPosEffects(string parameters);
	vector<string> getNegEffects(string parameters);
};




class agent {
public:
    int x = -1;
    int y = -1;
    std::string id;
	std::string home;

    std::vector<action> plan;

    std::vector<std::function<int()> > planFunc;

    
    void update();

    void getAgentPos(unordered_set<string> setState);
};


#endif //GLMAP_AGENT_H
