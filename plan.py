#!/usr/bin/python

import random
import copy
from pprint import pprint

distToWork=1;
distToShop=1;
workDifficulty=1;
workTemperature=1;
workStart=0;
workDuration=24;


class stateClass:
    def __hash__(self):
        return hash((self.__dict__.values()))
    def __str__(self):
        return 'energy: '+ str(self.energy) +',' + 'utime:' + str(self.utime) 

    
state0 = stateClass();


state0.sat=32;
state0.heatHome=0;
state0.inv=['foodStamp','foodStamp'];
state0.invHome=['wood','food'];
state0.energy=256;
state0.absent=0;
state0.workDays=0;
state0.isHomeHeated=0;
state0.loc="atHome"; #atWork, atHome, underway
state0.utime=0;  #universal time
state0.dayTime=lambda: state.utime%96;

#def dayTime():
#    return (utime%96);


#class action():
#    duration = 1;
#    def pre():
#        return (energy>0)&(sat>0)
#    def eff():
#        global utime
#        utime += action.duration;
#        print(action.duration) 

class eat():
    duration = 1;
    name = 'eat'
    #@staticmethod
    def pre(state):
        #result = super(eat,eat).pre();
        result = (state.energy>0)&(state.sat>0)&(state.loc == "atHome")&('food' in state.invHome);
        return result;
    #@staticmethod
    def eff(state):
        #global utime
        #super(eat,eat).eff();
        
        s1 = copy.deepcopy(state);

        s1.utime += eat.duration;
        s1.sat=32;
        s1.energy -= eat.duration;
        s1.invHome.remove('food')

        
        
        #print ('eating');
        return s1;
        
    
class gotoShop():
    name = 'go to shop'
    duration = 2*distToShop + 1
    #@staticmethod
    def pre(state):
        #result = super(gotoShop,gotoShop).pre();
        return (state.energy>0)&(state.sat>0)&(state.loc=="atHome")&('foodStamp' in state.inv)
    #@staticmethod
    def eff(state):
        s1 = copy.deepcopy(state);
        #super(gotoShop,gotoShop).eff();
        s1.inv.remove('foodStamp')
        s1.invHome.append('food')
        #print ('going to Shop');
        s1.energy -= gotoShop.duration;
        return s1;

class wait():
    name = 'wait'
    duration = 1;
    def __str__(self):
        return 'wait'
    def pre(state):
        return (state.energy>0)&(state.sat>0);
    def eff(state):
        s1 = copy.deepcopy(state);
        s1.utime += wait.duration;
        s1.energy -= wait.duration;
        s1.sat -= wait.duration;
        
        #print('Waiting');
        return s1;

    

def goal(state):
    return (state.utime>90)&(state.energy>0)&(state.sat>0)


actions = [eat, gotoShop, wait];

start = state0


stack=[(start,[])];
visited = set([start]);


def planDay():
    while stack:
        s0, path = stack.pop()
        pprint(vars(s0))
        #print(s0)
        #print(path)
        
        for a1 in actions:
            if (a1.pre(s0)):
                s1 = a1.eff(s0);
                if goal(s1):
                    plan = path + [a1]
                    #state = s1;
                    return s1, plan
                
            #print(s1 in visited)
        # and (not s1 in visited):
                stack.append((s1,path + [a1]));
                visited.add(s1);

s1, plan = planDay();

    
pprint(plan)
pprint(vars(s1))
    #print(stack)

            
    #print("Next action:");
    #a1 = random.choice(actions);
    
    #    a1.eff(state);
    


