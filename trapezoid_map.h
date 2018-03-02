#include "structures.h"
#include <vector>
#include <set>

class TrapezoidMap
{
public:
	TrapezoidMap();
	~TrapezoidMap();

	//segments are expected to be non-intersecting and have no common X-coordinates of their ends
	void			 buildMap(std::vector<Segment>& segments);

	//call this if you are unsure about the property above
	bool			 validateSegments(std::vector<Segment>& segments);

	//localization in the map
	const Trapezoid* localize(Point pt);

	void 			 getTrapezoids(std::vector<Trapezoid*>& trapezoids);
	void 			 clear();

private:
	void 		addSegment(Segment* segment);
	GraphNode* 	mapQuery(Point pTarget, Point pExtra);

	void 		simpleCase(GraphNode* tpNode, Segment* segment);
	void		hardCase(GraphNode* pLeft, GraphNode* pRight, Segment* segment);

	GraphNode* 				_rootNode;
	std::vector<Segment> 	_segments;
	bool					_mapReady;
};
