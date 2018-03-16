#pragma once

#include "head.h"
#include "Vertex.h"
#include "Face.h"

class Edge
{
public:
	int index;
	Vertex* fromV;
	Vertex* toV;
	float valueF;
	int heapPos;

	Edge(Vertex* v1, Vertex* v2);
	float length();
	void setHeapPos(int pos) { heapPos = pos; };
	void setValueF(float f) { valueF = f; }
	float getValueF() { return valueF; }
};