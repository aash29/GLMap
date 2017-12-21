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

actionPrefab::actionPrefab(pddlTreeNode* action)
{
	pddlTreeNode* r2 = action->findFirstName(":parameters");

	for (auto n1 : r2->children)
	{
		parNames.push_back(n1.data);
	};

	pddlTreeNode* preconditions = action->findFirstName(":precondition")->findFirstName("and");

	for (auto n2 : preconditions->children) {
		precondsWithVars.push_back(n2.data +" "+ n2.flattenChildren());
	}

	vector<pddlTreeNode> effects = action->findFirstName(":effect")->children[0].children; // "and"
	for (auto n1 : effects)
	{

		if (n1.data == "not") //remove effects from state
		{
			pddlTreeNode n2 = n1.children[0];

			string effectName = n2.data;

			string effectParameters = n2.flattenChildren();

			negEffectsWithVars.push_back(effectName +" "+ effectParameters);

		}
		else //add effects to state
		{

			string effectName = n1.data;

			string effectParameters = n1.flattenChildren();

			posEffectsWithVars.push_back(effectName +" "+ effectParameters);
		}

	};


}


vector<string> actionPrefab::subsParams(vector<string> expr, vector<string> values)
{
	vector<string> result;

	for (string s1 : expr) {

		for (int i = 0; i < parNames.size(); i++) {
			utils::replaceSubstrs(s1, parNames[i], values[i]);
		};
		result.push_back(s1);

	}
	return result;

}

vector<string> actionPrefab::getPreconditions(string parameters)
{

	vector<string> result;

	std::vector<std::string> parValues = utils::tokenize(parameters, ' ');

	result = subsParams(precondsWithVars, parValues);

	return result;
}

vector<string> actionPrefab::getPosEffects(string parameters)
{
	vector<string> result;

	std::vector<std::string> parValues = utils::tokenize(parameters, ' ');

	result = subsParams(posEffectsWithVars, parValues);
	return result;
}
