#pragma once
#include "head.h"
#include "Step0.h"

class Step1 {
public:
	int num;
	vector<Vertex> vertices;
	vector<Face> faces;
	vector<Edge> edges;
	Step0* obj0;

	void addNearEandNearV(int from, int to, int& eIndex);

	MatrixXf getMatrixV(int iterNum, MatrixXf& oldMatrixV);
	float computeArea(int iterNum);
	SparseMatrixType getMatrixL();
	SparseMatrixType getMatrixWL(int iterNum, SparseMatrixType& oldMatrixWL, float avgArea);
	SparseMatrixType getMatrixWH(int iterNum, SparseMatrixType& oldMatrixWH);
	SparseMatrixType getMatrixA(SparseMatrixType& matrixL, SparseMatrixType& matrixWL, SparseMatrixType& matrixWH);
	MatrixXf getMatrixb(SparseMatrix<float>& matrixWH, MatrixXf& matrixV);
	MatrixXf solve(SparseMatrix<float>& matrixA, MatrixXf& matrixb);
	float volume();

	Step1(string file);
	Step1(Step0* step0);
	void execute();
	void output(int iterNum = -1);
	void show();
};