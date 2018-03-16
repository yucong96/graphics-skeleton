#pragma once

#include "head.h"
#include "Vertex.h"
#include "Edge.h"

class Face
{
public:
	int index;
	vector<Vertex*> nearV;
	float area;

	Face(Vertex* v1, Vertex* v2, Vertex* v3);
	void computeArea();
};