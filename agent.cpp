//
// Created by aash29 on 05.12.17.
//

#include "agent.h"
#include "utils.hpp"
#include "appLog.h"
#include <sstream>


using namespace std;


void agent::update(){
    for (auto act1: effects){
        act1();
    }
    debug_log().AddLog("agent %s temp: %d \n", id.c_str(), heat);
}


void agent::getAgentPos(unordered_set<string> setState)
{
	string agentName;
	agentName = string("at") + " " + id;
	string::size_type n = agentName.length();

	for (string p1: setState)
	{
		string::size_type i = p1.find(agentName);
		if (i != string::npos)
		{
			vector<string> words = utils::tokenize(p1, ' ');


			vector<string> coords = utils::tokenize(words[2], '_');

			x = std::stoi(coords[1]);
			y = std::stoi(coords[2]);

		}
	}



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


vector<string> actionPrefab::getNegEffects(string parameters)
{
	vector<string> result;

	std::vector<std::string> parValues = utils::tokenize(parameters, ' ');

	result = subsParams(negEffectsWithVars, parValues);
	return result;
}

