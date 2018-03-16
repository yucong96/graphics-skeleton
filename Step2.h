#pragma once
#include "head.h"
#include "Heap.h"
#include "Step0.h"
#include "Step1.h"

class Step2 {
public:
	int num;
	vector<Vertex> vertices;
	vector<Face> faces;
	vector<Edge> edges;
	Heap heap;
	Step0* obj0;
	Step1* obj1;

	void computeMatrixQ();
	void computeSumEdge();
	void computeF();
	vector<Edge*>::iterator rmAfromB(Vertex* from, Vertex* to);
	vector<Edge*>::iterator rmAfromB(Edge* e);
	void addAinB(Vertex* a, Vertex* b);
	void collapse();

	Step2(Step0* step0, Step1* step1);
	void execute();
	void output();
	void show();
};