#include "head.h"
#include "Edge.h"

Edge::Edge(Vertex* from, Vertex* to)
{
	fromV = from;
	toV = to;
}

float Edge::length()
{
	Vector3f vec = fromV->getVector() - toV->getVector();
	return vec.norm();
}
