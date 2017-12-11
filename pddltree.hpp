#ifndef PDDLTREE_HPP
#define PDDLTREE_HPP

#include <string>
#include <vector>
#include <regex>

#include "appLog.h"

using namespace std;


struct pddlTreeNode
{
  std::string data;
  std::vector <pddlTreeNode>  children;

  void insert_back(pddlTreeNode n1);

  pddlTreeNode(char* initData);

  pddlTreeNode(std::string initData);


  pddlTreeNode* findFirst(std::string  name, std::string filter = ".*");

  pddlTreeNode* findFirstRegex(std::regex rn, std::regex rf);

  pddlTreeNode* findFirstName(std::string name);

  pddlTreeNode* findFirstExact1stChild(std::string  name, std::string filter);

  pddlTreeNode* findFirstExact(std::string  name, std::string filter);
  
  std::vector<pddlTreeNode*> search (std::string  name, std::string filter = ".*");

  
  std::vector<pddlTreeNode*> searchRegex (std::regex rn, std::regex rf);
  std::string flattenChildren();
};



void visitNodes(pddlTreeNode* node);



#endif

