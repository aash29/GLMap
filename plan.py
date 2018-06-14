#!/usr/bin/python
import random

utime=0;  #universal time
dtime=0;  # day time
day=0;

distToWork=1;
distToShop=1;
workDifficulty=1;
workTemperature=1;
workStart=0;
workDuration=24;

sat=32;
heatHome=0;
inv=['foodStamp','foodStamp'];
invHome=['wood'];
energy=256;
absent=0;
workDays=0;
isHomeHeated=0;
#foodStamps=random.randint(5,10);
#wood=random.randint(1,5);
loc="atHome"; #atWork, atHome, underway


def eatPre():
    return (loc=="atHome")&('foodStamp' in inv)

def eatEff():
    sat=32;
    utime+=1;

print(eatPre())


