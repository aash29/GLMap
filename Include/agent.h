//
// Created by aash29 on 05.12.17.
//

#ifndef GLMAP_AGENT_H
#define GLMAP_AGENT_H

#include <string>
#include <vector>
#include "path_impl.hpp"
#include <unordered_set>
#include <functional>
#include <set>


using namespace std;

struct action {
    std::string name;
    std::string params;
};



class agent {
public:
    int x = -1;
    int y = -1;

    int heat = 100;
    int energy = 100;
    int fed = 100;

    std::multiset<string> inventory = {"food","food","food","food","food"};

    std::string id;
    std::string home;

    std::vector<std::function<int()> > planFunc;
    std::vector<std::function<int()> > effects;


    void update();
};
#endif GLMAP_AGENT_H