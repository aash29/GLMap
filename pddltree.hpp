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


  pddlTreeNode* findFirst(std::string  name, std::string filter = ".*")
  {
	  const std::regex rn(name);
	  const std::regex rf(filter);
	  return findFirstRegex(rn, rf);

  }

  pddlTreeNode* findFirstRegex(std::regex rn, std::regex rf)
  {
	  std::vector<pddlTreeNode* > stack;
	  stack.push_back(this);
	  pddlTreeNode* cn;

	  while (stack.size()>0)
	  {
		  cn = stack.back();
		  stack.pop_back();

		  std::string wholeString;

		  for (auto it1 = cn->children.begin(); it1 != cn->children.end(); it1++)
		  {
			  stack.insert(stack.begin(), &(*it1));
			  wholeString.append(it1->data);
			  wholeString.append(" ");
		  }
		  if (std::regex_match(cn->data, rn) && std::regex_match(wholeString, rf))
		  {
			  return cn;
		  }
	  }
	  return NULL;
  };

std::vector<pddlTreeNode*> search (std::string  name, std::string filter = ".*")
  {
    const std::regex rn(name);
    const std::regex rf(filter);
    return searchRegex (rn, rf);
  };

  
  std::vector<pddlTreeNode*> searchRegex (std::regex rn, std::regex rf)
  {
    std::vector<pddlTreeNode* > stack;
    std::vector<pddlTreeNode* > result;
    stack.push_back(this);
    pddlTreeNode* cn;
        
    while (stack.size()>0)
      {
	cn = stack.back();
	stack.pop_back();

	std::string wholeString;
	
	for (auto it1 = cn->children.begin(); it1 != cn->children.end(); it1++)
	  {
	    stack.insert(stack.begin(),&(*it1));
	    wholeString.append(it1->data);
	    wholeString.append(" ");	    
	  }
	if (std::regex_match(cn->data, rn) && std::regex_match(wholeString, rf))
	  {
	    result.push_back(cn);
	  }
      }
      return result;
  };

  std::string flattenChildren()
  {
    std::string result;
      for (auto n1: children)
	{
	  result.append(n1.data);
	  result.append(" ");
	};
      return result;
  };
  
};



void visitNodes(pddlTreeNode* node)
{
  if (ImGui::TreeNode(node->data.c_str()))
      {
	for (std::vector<pddlTreeNode>::iterator cn = node->children.begin(); cn != node->children.end(); cn++)
	  {
	    visitNodes(&(*cn));
	  }
	ImGui::TreePop();
      }  
};



#endif

