#ifndef PDDLTREE_HPP
#define PDDLTREE_HPP

#include <string>
#include <vector>
#include <regex>

#include "appLog.h"


struct pddlTreeNode
{
  std::string data;
  std::vector <pddlTreeNode>  children;

  void insert_back(pddlTreeNode n1)
  {
    children.push_back(n1);
  }

  pddlTreeNode(char* initData)
  {
    data = std::string(initData);
  }

  pddlTreeNode(std::string initData)
  {
    data = initData;
  }

  std::vector<pddlTreeNode*> search (std::string match)
  {
    std::vector<pddlTreeNode* > stack;
    std::vector<pddlTreeNode* > result;
    stack.push_back(this);
    pddlTreeNode* cn;
        
    while (stack.size()>0)
      {
	cn = stack.back();
	stack.pop_back();

	std::string wholeString(cn->data);
	
	for (auto it1 = cn->children.begin(); it1 != cn->children.end(); it1++)
	  {
	    stack.insert(stack.begin(),&(*it1));
	    wholeString.append(it1->data);
	    wholeString.append(" ");	    
	  }

	debug_log().AddLog(wholeString.c_str());
	
	const std::regex re("at.*");
	

	//if (match==cn->data)
	if (std::regex_match(wholeString, re))
	  {
	    //return cn;
	    result.push_back(cn);
	    debug_log().AddLog("found at node \n");
	    
	  }

	
	
      }
    return result;
  }

  
};

#endif
