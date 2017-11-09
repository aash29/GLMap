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

};

#endif
