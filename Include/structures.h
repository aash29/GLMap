#include <algorithm>
#include <cassert>
#include <list>

struct Point
{
	Point(float x = 0.0f, float y = 0.0f): x(x), y(y) {}
	float x;
	float y;
};

namespace
{
	inline bool CCW(const Point& a, const Point& b, const Point& c)
	{
  		return (c.y - a.y) * (b.x - a.x) > (b.y - a.y) * (c.x - a.x);
	}
}

struct Segment
{
	Segment(Point pt1, Point pt2): ptLeft(pt1), ptRight(pt2) 
	{
		if  (ptLeft.x >  ptRight.x || 
			(ptLeft.x == ptRight.x && ptLeft.y > ptRight.y))
				std::swap(ptLeft, ptRight);
	}
	bool isAbove(Point pTarget, Point pGuide)
	{
		float EPS = 0.1;
		float det = this->detHelper(pTarget);

		return (fabsf(det) > EPS) ? det > 0 : this->detHelper(pGuide) > 0;
	}
	Point ptWithX(float x)
	{
		float y = ptLeft.y + (ptRight.y - ptLeft.y) / (ptRight.x - ptLeft.x) * (x - ptLeft.x);
		return Point(x, y);
	}
	bool intersects(const Segment& other)
	{
		return (CCW(ptLeft, other.ptLeft, other.ptRight) != 
				CCW(ptRight, other.ptLeft, other.ptRight)) && 
			   (CCW(ptLeft, ptRight, other.ptLeft) != 
			    CCW(ptLeft, ptRight, other.ptRight));
	}

	float 	minY() 	{return std::min(ptLeft.y, ptRight.y);}
	float 	maxY() 	{return std::max(ptLeft.y, ptRight.y);}

	Point ptLeft;
	Point ptRight;

private:
	float detHelper(Point p)
	{
		return ((ptRight.x - ptLeft.x) * (p.y - ptLeft.y) - 
				(ptRight.y - ptLeft.y) * (p.x - ptLeft.x));
	}
};

struct GraphNode;

struct Trapezoid
{
	Trapezoid(): trRightBot(nullptr), trRightTop(nullptr),
				 trLeftTop(nullptr), trLeftBot(nullptr),
				 graphNode(nullptr){}

	void setOneRight(Trapezoid* tp) {trRightTop = tp; trRightBot = nullptr;}
	void setOneLeft(Trapezoid* tp) {trLeftTop = tp; trLeftBot = nullptr;}

	void changeLeftWith(Trapezoid* tp)
	{
		if (trLeftTop)
		{
			assert(trLeftTop->trRightTop == this || trLeftTop->trRightBot == this);
			if (trLeftTop->trRightTop == this) trLeftTop->trRightTop = tp;
			else trLeftTop->trRightBot = tp;
		}
		if (trLeftBot)
		{
			assert(trLeftBot->trRightTop == this || trLeftBot->trRightBot == this);
			if (trLeftBot->trRightTop == this) trLeftBot->trRightTop = tp;
			else trLeftBot->trRightBot = tp;
		}
	}

	void changeRightWith(Trapezoid* tp)
	{
		if (trRightTop)
		{
			assert(trRightTop->trLeftTop == this || trRightTop->trLeftBot == this);
			if (trRightTop->trLeftTop == this) trRightTop->trLeftTop = tp;
			else trRightTop->trLeftBot = tp;
		}
		if (trRightBot)
		{
			assert(trRightBot->trLeftTop == this || trRightBot->trLeftBot == this);
			if (trRightBot->trLeftTop == this) trRightBot->trLeftTop = tp;
			else trRightBot->trLeftBot = tp;
		}
	}

	Segment* top;
	Segment* bot;
	Point left;
	Point right;

	Trapezoid* trRightBot;
	Trapezoid* trRightTop;
	Trapezoid* trLeftTop;
	Trapezoid* trLeftBot;

	GraphNode* graphNode;
};

///////////////////////////////////////

class GraphNode 
{
public:
	GraphNode(): _left(nullptr), _right(nullptr) {}
	virtual ~GraphNode() {}

	virtual Trapezoid* 	getTrapezoid() 			{return nullptr;}
	virtual GraphNode* 	nextNode(Point, Point) 	{return nullptr;}
	
	void attachLeft(GraphNode* node) 
	{
		_left = node;
		node->_parents.push_back(this);
	}
	
	void attachRight(GraphNode* node)
	{
		_right = node;
		node->_parents.push_back(this);
	}
	
	void replaceWith(GraphNode* node)
	{
		assert(!_parents.empty());
		for (auto parent : _parents)
		{
			if (parent->_left == this)
			{
				parent->_left = node;
			}
			else
			{
				assert(parent->_right == this);
				parent->_right = node;
			}
		}
	}

	GraphNode* _left;
	GraphNode* _right;

private:
	std::list<GraphNode*> _parents;
};

class XNode: public GraphNode
{
public:
	XNode(float p): _point(p) {}

	virtual GraphNode* nextNode(Point p, Point)
	{
		return (p.x < _point) ? _left : _right;
	}

private:
	float _point;
};

class YNode: public GraphNode
{
public:
	YNode(Segment* s): _segment(s) {}

	virtual GraphNode* nextNode(Point pTarget, Point pGuide)
	{
		return _segment->isAbove(pTarget, pGuide) ? _left : _right;
	}
private:
	Segment* _segment;
};

class TerminalNode: public GraphNode
{
public:
	TerminalNode(Trapezoid* tp): _trapezoid(tp) 
	{
		tp->graphNode = this;
	}
	virtual ~TerminalNode() 			{delete _trapezoid;}
	virtual Trapezoid* getTrapezoid() 	{return _trapezoid;}
private:
	Trapezoid* _trapezoid;
};
