#include "trapezoid_map.h"
#include <cassert>
#include <limits>
#include <iostream>
#include <unordered_set>
#include <functional>
#include <iterator>

TrapezoidMap::TrapezoidMap():
	_rootNode(nullptr),
	_mapReady(false)
{}

TrapezoidMap::~TrapezoidMap()
{
	//std::cerr << "cleaning up\n";
	this->clear();
	//std::cerr << "done\n";
}

namespace
{
	std::unordered_set<GraphNode*> g_visited;
	void recHelper(GraphNode* node, std::function<void(GraphNode*)> fun)
	{
		if (g_visited.find(node) != g_visited.end()) return;
		g_visited.insert(node);

		if (node->_left) recHelper(node->_left, fun);
		if (node->_right) recHelper(node->_right, fun);
		fun(node);
	}
}

void TrapezoidMap::clear()
{
	if (!_mapReady) return;

	_segments.clear();
	_mapReady = false;

	std::unordered_set<GraphNode*> toDelete;
	auto f = [&toDelete](GraphNode* node){toDelete.insert(node);};

	g_visited.clear();
	recHelper(_rootNode, f);

	for (auto node : toDelete) delete node;
	_rootNode = nullptr;
}

void TrapezoidMap::getTrapezoids(std::vector<Trapezoid*>& tpzds)
{
	std::unordered_set<Trapezoid*> trapSet;

	auto f = [&trapSet](GraphNode* node)
	{
		if (node->getTrapezoid()) 
			trapSet.insert(node->getTrapezoid());
	};

	g_visited.clear();
	recHelper(_rootNode, f);
	std::copy(trapSet.begin(), trapSet.end(), std::back_inserter(tpzds));
}

GraphNode* TrapezoidMap::mapQuery(Point pTarget, Point pExtra)
{
	assert(_rootNode);
	GraphNode* curNode = _rootNode;
	while (!curNode->getTrapezoid())
	{
		curNode = curNode->nextNode(pTarget, pExtra);
	}
	return curNode;
}

const Trapezoid* TrapezoidMap::localize(Point pt)
{
	return this->mapQuery(pt, pt)->getTrapezoid();
}

bool TrapezoidMap::validateSegments(std::vector<Segment>& segments)
{
	if (segments.size() < 2) return false;

	for (auto seg1 = segments.begin(); seg1 != segments.end(); ++seg1)
	{
		for (auto seg2 = seg1; seg2 != segments.end(); ++seg2)
		{
			if (seg1 == seg2) continue;

			if (seg1->intersects(*seg2)) return false;
		}
	}
	return true;
}

void TrapezoidMap::buildMap(std::vector<Segment>& segments)
{
	if (_mapReady) this->clear();
	if (segments.size() < 2) return;

	_mapReady = true;
	_segments = segments;

	//bounding box
	float minX = std::numeric_limits<float>::lowest();
	float minY = std::numeric_limits<float>::lowest();
	float maxX = std::numeric_limits<float>::max();
	float maxY = std::numeric_limits<float>::max();

	Trapezoid* tp = new Trapezoid;
	_segments.push_back(Segment(Point(minX, maxY), Point(maxX, maxY)));
	tp->top = &_segments.back();
	_segments.push_back(Segment(Point(minX, minY), Point(maxX, minY)));
	tp->bot = &_segments.back();
	tp->left = Point(minX, maxY);
	tp->right = Point(maxX, maxY);

	_rootNode = new TerminalNode(tp);
	//end bounding box
	
	for (auto &segment : segments)
	{
		this->addSegment(&segment);
	}
}

void TrapezoidMap::addSegment(Segment* segment)
{
	GraphNode* node1 = this->mapQuery(segment->ptLeft, segment->ptRight);
	GraphNode* node2 = this->mapQuery(segment->ptRight, segment->ptLeft);
	Trapezoid* tp1 = node1->getTrapezoid();
	Trapezoid* tp2 = node2->getTrapezoid();

	if (tp1 == tp2)
	{
		this->simpleCase(node1, segment);
	}
	else
	{
		this->hardCase(node1, node2, segment);
	}
}

void TrapezoidMap::simpleCase(GraphNode* tpNode, Segment* segment)
{
	///new trapezoids
	Trapezoid* trLeft = new Trapezoid(*tpNode->getTrapezoid());
	trLeft->right = segment->ptLeft;

	Trapezoid* trRight = new Trapezoid(*tpNode->getTrapezoid());
	trRight->left = segment->ptRight;

	Trapezoid* trBot = new Trapezoid(*tpNode->getTrapezoid());
	trBot->top = segment;
	trBot->left = segment->ptLeft;
	trBot->right = segment->ptRight;

	Trapezoid* trTop = new Trapezoid(*tpNode->getTrapezoid());
	trTop->bot = segment;
	trTop->left = segment->ptLeft;
	trTop->right = segment->ptRight;
	///

	///updating neighbours
	trLeft->trRightTop = trTop;
	trLeft->trRightBot = trBot;
	trRight->trLeftTop = trTop;
	trRight->trLeftBot = trBot;

	trTop->setOneRight(trRight);
	trTop->setOneLeft(trLeft);
	trBot->setOneRight(trRight);
	trBot->setOneLeft(trLeft);

	tpNode->getTrapezoid()->changeLeftWith(trLeft);
	tpNode->getTrapezoid()->changeRightWith(trRight);
	///
	
	///updating graph
	GraphNode* newRoot = new XNode(segment->ptLeft.x);
	newRoot->attachLeft(new TerminalNode(trLeft));

	GraphNode* x2 = new XNode(segment->ptRight.x);
	newRoot->attachRight(x2);
	x2->attachRight(new TerminalNode(trRight));

	GraphNode* y1 = new YNode(segment);
	x2->attachLeft(y1);
	y1->attachLeft(new TerminalNode(trTop));
	y1->attachRight(new TerminalNode(trBot));
	///

	if (tpNode == _rootNode)
	{
		_rootNode = newRoot;
	}
	else
	{
		tpNode->replaceWith(newRoot);
	}
	delete tpNode;
}

namespace
{
	Trapezoid* getNextIntersecting(Segment* segment, Trapezoid* tr)
	{
		assert(tr->trRightTop || tr->trRightBot);

		Trapezoid* trNext;
		if (tr->trRightTop)
		{
			if (tr->trRightBot)
			{
				Point pt = segment->ptWithX(tr->trRightTop->left.x);
				if (tr->trRightTop->bot->isAbove(pt, segment->ptLeft))
				{
					trNext = tr->trRightTop;
				}
				else
				{ 
					trNext = tr->trRightBot;
				}
			}
			else
			{
				trNext = tr->trRightTop;
			}
		}
		else
		{
			trNext = tr->trRightBot;
		}
		return trNext;
	}
}

void TrapezoidMap::hardCase(GraphNode* pLeft, GraphNode* pRight, Segment* segment)
{
	Trapezoid* trBegin = pLeft->getTrapezoid();
	Trapezoid* trEnd = pRight->getTrapezoid();
	std::vector<Trapezoid*> toDelete;

	//leftmost one
	Trapezoid* trLeftmost = new Trapezoid(*trBegin);
	trLeftmost->right = segment->ptLeft;

	Trapezoid* trMergeTop = new Trapezoid(*trBegin);
	trMergeTop->left = segment->ptLeft;
	trMergeTop->bot = segment;
	GraphNode* terminalTop = new TerminalNode(trMergeTop);

	Trapezoid* trMergeBot = new Trapezoid(*trBegin);
	trMergeBot->left = segment->ptLeft;
	trMergeBot->top = segment;
	GraphNode* terminalBot = new TerminalNode(trMergeBot);

	//neighbors
	trLeftmost->trRightTop = trMergeTop;
	trLeftmost->trRightBot = trMergeBot;
	trBegin->changeLeftWith(trLeftmost);
	trMergeTop->setOneLeft(trLeftmost);
	trMergeBot->setOneLeft(trLeftmost);
	//
	
	//graph
	GraphNode* newLeft = new XNode(segment->ptLeft.x);
	GraphNode* newSplit = new YNode(segment);
	newLeft->attachLeft(new TerminalNode(trLeftmost));
	newLeft->attachRight(newSplit);
	newSplit->attachLeft(terminalTop);
	newSplit->attachRight(terminalBot);
	//

	Trapezoid* trPrev = trBegin;
	Trapezoid* trCurrent = getNextIntersecting(segment, trBegin);
	trBegin->graphNode->replaceWith(newLeft);
	toDelete.push_back(trBegin);

	//middle intersecting
	for (;;)
	{
		bool renewTop = false;
		if (trPrev->trRightBot && trPrev->trRightTop)
		{
			renewTop = (trPrev->trRightBot == trCurrent);
		}
		else
		{
			assert(trCurrent->trLeftBot == trPrev || trCurrent->trLeftTop == trPrev);
			renewTop = (trCurrent->trLeftBot == trPrev);
		}
		if (renewTop)
		{
			trMergeTop->right = trCurrent->left;
			Trapezoid* oldMergeTop = trMergeTop;
			trMergeTop = new Trapezoid(*trCurrent);
			terminalTop = new TerminalNode(trMergeTop);
			trMergeTop->bot = segment;

			if (trCurrent->trLeftBot && trCurrent->trLeftTop)
			{
				oldMergeTop->setOneRight(trMergeTop);
				trMergeTop->trLeftBot = oldMergeTop;
				trMergeTop->trLeftTop = trCurrent->trLeftTop;
				trCurrent->trLeftTop->setOneRight(trMergeTop);
				assert(trMergeTop->trLeftTop);
			}
			else
			{
				oldMergeTop->trRightBot = trMergeTop;
				oldMergeTop->trRightTop = trPrev->trRightTop;
				trPrev->trRightTop->setOneLeft(oldMergeTop);
				trMergeTop->setOneLeft(oldMergeTop);
			}
		}
		else
		{
			assert(trPrev->trRightTop == trCurrent);
			trMergeBot->right = trCurrent->left;
			Trapezoid* oldMergeBot = trMergeBot;
			trMergeBot = new Trapezoid(*trCurrent);
			terminalBot = new TerminalNode(trMergeBot);
			trMergeBot->top = segment;

			if (trCurrent->trLeftBot && trCurrent->trLeftTop)
			{
				oldMergeBot->setOneRight(trMergeBot);
				trMergeBot->trLeftTop = oldMergeBot;
				trMergeBot->trLeftBot = trCurrent->trLeftBot;
				trCurrent->trLeftBot->setOneRight(trMergeBot);
				assert(trMergeBot->trLeftBot);
			}
			else
			{
				oldMergeBot->trRightTop = trMergeBot;
				oldMergeBot->trRightBot = trPrev->trRightBot;
				trPrev->trRightBot->setOneLeft(oldMergeBot);
				trMergeBot->setOneLeft(oldMergeBot);
			}
		}

		if (trCurrent == trEnd) break;

		newSplit = new YNode(segment);
		newSplit->attachLeft(terminalTop);
		newSplit->attachRight(terminalBot);
		trCurrent->graphNode->replaceWith(newSplit);

		toDelete.push_back(trCurrent);
		trPrev = trCurrent;
		trCurrent = getNextIntersecting(segment, trCurrent);
	}

	//rightmost one
	trMergeTop->right = segment->ptRight;
	trMergeBot->right = segment->ptRight;

	Trapezoid* trRightmost = new Trapezoid(*trEnd);
	trRightmost->left = segment->ptRight;

	//neighbors
	trMergeTop->setOneRight(trRightmost);
	trMergeBot->setOneRight(trRightmost);
	trRightmost->trLeftTop = trMergeTop;
	trRightmost->trLeftBot = trMergeBot;
	trEnd->changeRightWith(trRightmost);
	//

	GraphNode* newRight = new XNode(segment->ptRight.x);
	newSplit = new YNode(segment);
	newRight->attachRight(new TerminalNode(trRightmost));
	newRight->attachLeft(newSplit);

	newSplit->attachLeft(terminalTop);
	newSplit->attachRight(terminalBot);
	trEnd->graphNode->replaceWith(newRight);
	toDelete.push_back(trEnd);

	for (auto tp : toDelete) {delete tp->graphNode;}
}
