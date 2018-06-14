#!/usr/bin/python
import random




distToWork=1;
distToShop=1;
workDifficulty=1;
workTemperature=1;
workStart=0;
workDuration=24;


class stateClass:
    pass

state = stateClass();


state.sat=32;
state.heatHome=0;
state.inv=['foodStamp','foodStamp'];
state.invHome=['wood','food'];
state.energy=256;
state.absent=0;
state.workDays=0;
state.isHomeHeated=0;
#foodStamps=random.randint(5,10);
#wood=random.randint(1,5);
state.loc="atHome"; #atWork, atHome, underway
state.utime=0;  #universal time
#dtime=0;  # day time
#day=0;
state.dayTime=lambda: state.utime%96;
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
    #@staticmethod
    def pre():
        #result = super(eat,eat).pre();
        result = (energy>0)&(sat>0)&(loc == "atHome")&('food' in invHome);
        return result;
    #@staticmethod
    def eff(state):
        #global utime
        #super(eat,eat).eff();
        state.utime += eat.duration;
        state.sat=32;
        print ('eating');
        return state;
        
    
class gotoShop():
    duration = 2*distToShop + 1
    #@staticmethod
    def pre():
        #result = super(gotoShop,gotoShop).pre();
        return (energy>0)&(sat>0)&(loc=="atHome")&('foodStamp' in inv)
    @staticmethod
    def eff(state):
        #super(gotoShop,gotoShop).eff();
        state.inv.remove('foodStamp')
        print ('goind to Shop');
        return state;

class wait():
    duration = 1;
    def pre():
        return True;
    def eff(state):
        state.utime += wait.duration;
        print('Waiting');
        return state;

    

def goal(state):
    return (state.dayTime()>90)&(state.energy>0)&(state.sat>0)


actions = [eat, gotoShop, wait];

start = state


stack=[(start,[start])];
visited = set(state);

while stack:
    state, path = 
    for a1 in actions:
        s1 = a1.eff(state);
        if (a1.pre()) and (not s1 in visited):
            stack.append((s1, );
            visited.add(s1);
            
    #print("Next action:");
    #a1 = random.choice(actions);
    
    #    a1.eff(state);
    



    
print(eatPre())


