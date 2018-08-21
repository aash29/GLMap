#ifndef  PATHNODE_HPP
#define PATHNODE_HPP

struct pathNode {

    int id;
    float x,y;
    std::vector <int> neigh;

};

struct graphNode {
    unsigned int id;
    std::vector <short int> neigh;
};




#endif

