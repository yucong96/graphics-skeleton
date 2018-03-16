#pragma once

#define FREEGLUT_STATIC
#include "GL/freeglut.h"
#include <Eigen/Sparse>
#include <Eigen/Geometry>
//#include <Eigen/Cholesky>
//#include <Eigen/LU>

#include <iostream>
#include <iomanip>
#include <vector>
#include <fstream>

#define PI 3.1415926535

using namespace std;
using namespace Eigen;

typedef SparseMatrix<float> SparseMatrixType;

#define MAXVEXTICES 25000
#define MAXFACES 50000
#define MAXEDGES (4*MAXFACES)

class Vertex;
class Face;
class Edge;



