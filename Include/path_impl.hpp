#include <iostream>
/* RLTK (RogueLike Tool Kit) 1.00
 * Copyright (c) 2016-Present, Bracket Productions.
 * Licensed under the MIT license - see LICENSE file.
 *
 * Example 4: Now we implement a basic map, and use A* to find our way around it.
 * This example is a bit more in-depth, since it demonstrates the library's ability
 * to use templates to specialize itself around your map design - we won't force a
 * map type on you!
 */

// You need to include the RLTK header
//#include "../../rltk/rltk.hpp"
#include "path_finding.h"

// We're using a vector to represent the map
#include <vector>

// We're also going to be using a shared_ptr to a map. Why shared? Because the library
// hands it off to you and it's up to you to use it; this provides some safety that it
// will be disposed when you are done with it.
#include <memory>
#include "appLog.h"

// For convenience, import the whole rltk namespace. You may not want to do this
// in larger projects, to avoid naming collisions.
using namespace rltk;
//using namespace rltk::colors;

// A default-defined random number generator. You can specify a seed to get
// the same results each time, but for now we're keeping it simple.
//random_number_generator rng;

// For now, we always want our "dude" to be a yellow @ - so he's constexpr
/*
const vchar dude{'@', YELLOW, BLACK};
// We're also going to render our destination as a pink heart. Aww.
const vchar destination_glyph{3, MAGENTA, BLACK};
// We now need to represent walls and floors, too
const vchar wall_tile{'#', WHITE, BLACK};
const vchar floor_tile{'.', GREY, BLACK}; // Note that "floor" is taken as a name in C++!
*/
// Now we define a structure to represent a location. In this case, it's a simple
// x/y coordinate.


inline float distance2d_squared(const int &x1, const int &y1, const int &x2, const int &y2) noexcept {
    const float dx = (float)x1 - (float)x2;
    const float dy = (float)y1 - (float)y2;
    return (dx*dx) + (dy*dy);
}

struct location_t {
    int x=-1; // I like to set uninitialized values to something invalid for help with debugging
    int y=-1;

    // For convenience, we're overriding the quality operator. This gives a very
    // quick and natural looking way to say "are these locations the same?"
    bool operator==(location_t &rhs) { return (x==rhs.x && y==rhs.y); }

    location_t() {}
    location_t(const int X, const int Y) : x(X), y(Y) {}
};

// Now we define our basic map. Why a struct? Because a struct is just a class with
// everything public in it!
struct map_t {
  map_t(const int &x1, const int &x2, const int &y1, const int &y2) : xmin(x1), xmax(x2), ymin(y1), ymax(y2) {
        // Resize the vector to hold the whole map; this way it won't reallocate
        walkable.resize((x2-x1+1)*(y2-y1+1));

        // Set the entire map to walkable
        std::fill(walkable.begin(), walkable.end(), true);

        // We want the perimeter to be solid
        for (int x=x1; x<=x2; x++) {
            walkable[at(x,y1)]=false;
            walkable[at(x,y2)]=false;
        }
        for (int y=y1; y<=y2; y++) {
            walkable[at(x1,y)] = false;
            walkable[at(x2,y)] = false;
        }
    }

    // Calculate the vector offset of a grid location
    inline int at(const int &x, const int &y)
  { return ((xmax-xmin+1)*(y-ymin))+(x-xmin); }

    // The width and height of the map
  const int xmin,xmax, ymin, ymax;

    // The actual walkable storage vector
    std::vector<bool> walkable;
};

// The A* library returns a navigation path with a template specialization to our location_t.
// Store the path here. Normally, you'd use "auto" for this type, it is a lot less typing!
std::shared_ptr<navigation_path<location_t>> path;

// We're using 1024x768, with 8 pixel wide chars. That gives a console grid of
// 128 x 96. We'll go with that for the map, even though in reality the screen
// might change. Worrying about that is for a future example!
map_t map(-1,5, -2,10);



// The A* library also requires a helper class to understand your map format.
struct navigator {
    // This lets you define a distance heuristic. Manhattan distance works really well, but
    // for now we'll just use a simple euclidian distance squared.
    // The geometry system defines one for us.
    static float get_distance_estimate(location_t &pos, location_t &goal) {
        float d = distance2d_squared(pos.x, pos.y, goal.x, goal.y);
        return d;
    }

    // Heuristic to determine if we've reached our destination? In some cases, you'd not want
    // this to be a simple comparison with the goal - for example, if you just want to be
    // adjacent to (or even a preferred distance from) the goal. In this case,
    // we're trying to get to the goal rather than near it.
    static bool is_goal(location_t &pos, location_t &goal) {
        return pos == goal;
        //return (std::max(abs(pos.x-goal.x),abs(pos.y-goal.y))<=1.1f);
        //return ((abs(pos.x - goal.x)<=1)&&(abs(pos.y - goal.y)<=1));
    }

    // This is where we calculate where you can go from a given tile. In this case, we check
    // all 8 directions, and if the destination is walkable return it as an option.
    static bool get_successors(location_t pos, std::vector<location_t> &successors) {
      //std::cout << pos.x << "/" << pos.y << "\n";

        if (map.walkable[map.at(pos.x-1, pos.y-1)]) successors.push_back(location_t(pos.x-1, pos.y-1));
        if (map.walkable[map.at(pos.x, pos.y-1)]) successors.push_back(location_t(pos.x, pos.y-1));
        if (map.walkable[map.at(pos.x+1, pos.y-1)]) successors.push_back(location_t(pos.x+1, pos.y-1));

        if (map.walkable[map.at(pos.x-1, pos.y)]) successors.push_back(location_t(pos.x-1, pos.y));
        if (map.walkable[map.at(pos.x+1, pos.y)]) successors.push_back(location_t(pos.x+1, pos.y));

        if (map.walkable[map.at(pos.x-1, pos.y+1)]) successors.push_back(location_t(pos.x-1, pos.y+1));
        if (map.walkable[map.at(pos.x, pos.y+1)]) successors.push_back(location_t(pos.x, pos.y+1));
        if (map.walkable[map.at(pos.x+1, pos.y+1)]) successors.push_back(location_t(pos.x+1, pos.y+1));
        return true;
    }

    // This function lets you set a cost on a tile transition. For now, we'll always use a cost of 1.0.
    static float get_cost(location_t &position, location_t &successor) {
        return 1.0f;
    }

    // This is a simple comparison to determine if two locations are the same. It just passes
    // through to the location_t's equality operator in this instance (we didn't do that automatically)
    // because there are times you might want to behave differently.
    static bool is_same_state(location_t &lhs, location_t &rhs) {
        return lhs == rhs;
    }
};

// Lets go really fast!
constexpr double tick_duration = 1.0;
double tick_time = 0.0;

// Instead of raw ints, we'll use the location structure to represent where our
// dude is. Using C++14 initialization, it's nice and clean.
location_t dude_position {2,3};

// We'll also use a location_t to represent the intended destination.
location_t destination {5,5};

// Your main function
int path_test()
{
    // Initialize with defaults
    //init(config_simple_px("../assets"));


    path = find_path<location_t, navigator>(dude_position, destination);
    if (path->success)
    {
      debug_log().AddLog("path found \n");
      debug_log().AddLog("%d,%d", path->steps.front().x,path->steps.front().y);

    }
    return 0;
}
