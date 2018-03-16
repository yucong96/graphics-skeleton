#pragma once
#include "head.h"
#include "Step0.h"
#include "Step1.h"
#include "Step2.h"

class Step3 {
public:
	int num;
	vector<Vertex> vertices;
	vector<Face> faces;
	vector<Edge> edges;
	Step0* obj0;
	Step1* obj1;
	Step2* obj2;

	void refine1();
	void setPartNum(Vertex* start, int partNum);
	void refine2();
	void computeStdDev(Vertex* v);
	float computeStdDev(Vertex* v1, Vertex* v2);
	vector<Edge*>::iterator rmAfromB(Vertex* from, Vertex* to);
	vector<Edge*>::iterator rmAfromB(Edge* e);
	void addAinB(Vertex* from, Vertex* to);

	Step3(Step0* step0, Step1* step1, Step2* step2);
	void execute();
	void output();
	void show();
};