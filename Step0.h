#pragma once

#include "head.h"
#include "Vertex.h"
#include "Face.h"
#include "Edge.h"

class Step0
{
public:
	int num;
	vector<Vertex> vertices;
	vector<Face> faces;
	vector<Edge> edges;

	void addNearEandNearV(int from, int to, int& eIndex);

	Step0(string file);
	void output();
	void show();
};