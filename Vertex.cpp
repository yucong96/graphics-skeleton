#include "head.h"
#include "Vertex.h"

Vertex::Vertex(float x, float y, float z)
{
	this->x = x;
	this->y = y;
	this->z = z;
}

Vector3f Vertex::getVector()
{
	Vector3f v;
	v << x, y, z;
	return v;
}

void Vertex::computeOneRingArea()
{
	oneRingArea = 0;
	for (int j = 0; j < nearF.size(); j++)
		oneRingArea += nearF[j]->area;
}

void Vertex::computeMatrixQ()
{
	matrixQ = Matrix<float, 4, 4>();
	matrixQ.setZero();
	for (vector<Vertex*>::iterator v_near = nearV.begin(); v_near != nearV.end(); v_near++)
	{
		Matrix<float, 3, 4> matrixK = Matrix<float, 3, 4>();
		matrixK.setZero();
		Vector3f a = this->getVector() - (*v_near)->getVector();
		a = a / a.norm();
		Vector3f b = a.cross(this->getVector());
		matrixK(0, 1) = -a(2);	matrixK(0, 2) = a(1);	matrixK(0, 3) = -b(0);
		matrixK(1, 0) = a(2);	matrixK(1, 2) = -a(0);	matrixK(1, 3) = -b(1);
		matrixK(2, 0) = -a(1);	matrixK(2, 1) = a(0);	matrixK(2, 3) = -b(2);
		matrixQ += matrixK.transpose() * matrixK;
	}
}

void Vertex::computeSumEdges()
{
	sumEdges = 0;
	for (vector<Edge*>::iterator e = nearE.begin(); e != nearE.end(); e++)
		sumEdges += (*e)->length();
}
