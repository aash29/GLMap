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
        for (int x=x1; x<x2; x++) {
            walkable[at(x,y1)]=false;
            walkable[at(x,y2-1)]=false;
        }
        for (int y=y1; y<y2-1; y++) {
            walkable[at(x1,y)] = false;
            walkable[at(x2-1,y)] = false;
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



