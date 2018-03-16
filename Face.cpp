#include "head.h"
#include "Face.h"

Face::Face(Vertex* v1, Vertex* v2, Vertex* v3)
{
	nearV.push_back(v1);
	nearV.push_back(v2);
	nearV.push_back(v3);
}

void Face::computeArea()
{
	Vector3f edge1 = nearV[1]->getVector() - nearV[0]->getVector();
	Vector3f edge2 = nearV[2]->getVector() - nearV[0]->getVector();
	area = 0.5 * edge1.cross(edge2).norm();
}