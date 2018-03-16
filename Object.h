#pragma once

#include "head.h"
#include "Vertex.h"
#include "Heap.h"

class Object
{
private:
	int num;
	vector<Vertex> vertices;
	vector<Face> faces;
	vector<Edge> edges;
	Heap heap;

	// Step 0
	void addNearE(int from, int to);

	// Step 1
	MatrixXf getMatrixV(int iterNum, MatrixXf& oldMatrixV);
	float computeArea(int iterNum, MatrixXf matrixV);
	SparseMatrixType getMatrixL();
	SparseMatrixType getMatrixWL(int iterNum, SparseMatrixType& oldMatrixWL, float avgArea);
	SparseMatrixType getMatrixWH(int iterNum, SparseMatrixType& oldMatrixWH);
	SparseMatrixType getMatrixA(SparseMatrix<float>& matrixL, SparseMatrix<float>& matrixWL, SparseMatrix<float>& matrixWH);
	MatrixXf getMatrixb(SparseMatrix<float>& matrixWH, MatrixXf& matrixV);
	MatrixXf solve(SparseMatrix<float>& matrixA, MatrixXf& matrixb);


	// Step 2
	void computeMatrixQ();
	void computeSumEdge();
	void computeF();
	void rmAfromB(Vertex* from, Vertex* to);
	void rmAfromB(Edge* e);
	void addAinB(Vertex* a, Vertex* b);
	void collapse();


	// Step 3
	void refine1();
	void setPartNum(Vertex* start, int partNum);
	void refine2();

public:
	Object(string file);
	Object(Object& object);
	void executeStep1();
	void executeStep2();
	
};