#pragma once

#include "head.h"
#include "Face.h"
#include "Edge.h"

class Vertex
{
public:
	int index;
	float x, y, z;
	vector<Vertex*> nearV;
	vector<Face*> nearF;
	vector<Edge*> nearE;
	float oneRingArea0;
	float oneRingArea;
	MatrixXf matrixQ;
	float sumEdges;
	bool isValid;
	int validIndex;
	vector<Vertex*> victimV;
	float oldX, oldY, oldZ;
	vector<Vertex*> oldNearV;
	Vertex* center;
	bool isBound;
	float boundLen;
	int partTotal;
	int partNum;
	float stdDev;

	Vertex(float x, float y, float z);
	Vertex() { };

	void computeOneRingArea();
	void computeMatrixQ();
	void computeSumEdges();

	Vector3f getVector();
};