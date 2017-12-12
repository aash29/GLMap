
#include "pddltree.hpp"

void pddlTreeNode::insert_back(pddlTreeNode n1)
{
    children.push_back(n1);
}

pddlTreeNode::pddlTreeNode(char* initData)
{
    data = std::string(initData);
}

pddlTreeNode::pddlTreeNode(std::string initData)
{
data = initData;
}




pddlTreeNode* traverseTree(pddlTreeNode* cn, std::string  name, std::string filter)
{
	pddlTreeNode* result = NULL;

	if (cn->children.size()>0)
		if ((cn->data == name) && (cn->children[0].data == filter))
		{
			return cn;
		}

		for (std::vector<pddlTreeNode>::iterator child = cn->children.begin(); child != cn->children.end(); child++)
		{
			result = traverseTree(&(*child),name,filter);
			if (result != NULL)
				return result;
		}

		return result;

}



pddlTreeNode* pddlTreeNode::findFirstExact1stChild(std::string  name, std::string filter)
{
	std::vector<pddlTreeNode* > stack;
	stack.push_back(this);
	pddlTreeNode* cn;

	while (stack.size()>0)
	{
		cn = stack.back();
		stack.pop_back();


		for (auto it1 = cn->children.begin(); it1 != cn->children.end(); it1++)
		{
			stack.insert(stack.begin(), &(*it1));
		}

		if (cn->children.size()>0)

			if ((cn->data == name) && (cn->children[0].data == filter))
			{
				return cn;
			}
		}
	return NULL;
}




pddlTreeNode* pddlTreeNode::findFirstExact(std::string  name, std::string filter)
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

		if (!wholeString.empty())
			wholeString.pop_back();

	
        if ((cn->data == name) && (wholeString == filter))
        {
            return cn;
        }
    }
    return NULL;

}


pddlTreeNode* pddlTreeNode::findFirst(std::string  name, std::string filter)
{
    const std::regex rn(name);
    const std::regex rf(filter);

    if (filter==".*")
        return findFirstName(name);
    else
        return findFirstRegex(rn, rf);
}


pddlTreeNode* pddlTreeNode::findFirstName(std::string name)
{
    std::vector<pddlTreeNode* > stack;
    stack.push_back(this);
    pddlTreeNode* cn;

    while (stack.size()>0)
    {
        cn = stack.back();
        stack.pop_back();

        for (auto it1 = cn->children.begin(); it1 != cn->children.end(); it1++)
        {
            stack.insert(stack.begin(), &(*it1));
        }

        if (cn->data == name)
        {
            return cn;
        }
    }
    return NULL;
}

pddlTreeNode* pddlTreeNode::findFirstRegex(std::regex rn, std::regex rf)
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

std::vector<pddlTreeNode*> pddlTreeNode::search (std::string  name, std::string filter)
{
    const std::regex rn(name);
    const std::regex rf(filter);
    return searchRegex (rn, rf);
};


std::vector<pddlTreeNode*> pddlTreeNode::searchRegex (std::regex rn, std::regex rf)
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
	if (!wholeString.empty())
		wholeString.pop_back();

        if (std::regex_match(cn->data, rn) && std::regex_match(wholeString, rf))
        {
            result.push_back(cn);
        }
    }
    return result;
};

std::string pddlTreeNode::flattenChildren()
{
    std::string result;
    for (auto n1: children)
    {
        result.append(n1.data);
        result.append(" ");
    };

    //result.erase(result.find_last_not_of(" \n\r\t")+1);
    result.pop_back();
    return result;
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



