//
// Created by aash29 on 05.12.17.
//

#include "agent.h"
#include "utils.hpp"
#include "appLog.h"


using namespace std;


void agent::update(){
    for (auto act1: effects){
        act1();
    }
    debug_log().AddLog("agent %s temp: %d \n", id.c_str(), heat);
}



