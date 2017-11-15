#ifndef PDDLTREE_HPP
#define PDDLTREE_HPP

#include <string>
#include <vector>


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

  std::vector<pddlTreeNode*> search (std::string match, )
  {
    std::vector<pddlTreeNode* > stack;
    std::vector<pddlTreeNode* > result;
    stack.push_back(this);
    pddlTreeNode* cn;
        
    while (stack.size()>0)
      {
	cn = stack.back();
	stack.pop_back();

	if (match==cn->data)
	  {
	    //return cn;
	    result.push_back(cn);
	  }
	
	for (auto it1 = cn->children.begin(); it1 != cn->children.end(); it1++)
	  {
	    stack.insert(stack.begin(),&(*it1));
	  }

	
	
      }
    return result;
  }

  
};

#endif
