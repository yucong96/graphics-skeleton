#include "Step1.h"

Step1::Step1(Step0* step0)
{
	vertices.reserve(MAXVEXTICES);
	faces.reserve(MAXFACES);
	edges.reserve(MAXEDGES);

	obj0 = step0;
	num = obj0->num;
	vertices = obj0->vertices;
	faces = obj0->faces;
	edges = obj0->edges;
	for (vector<Vertex>::iterator v = vertices.begin(); v != vertices.end(); v++)
	{
		for (vector<Vertex*>::iterator v_near = v->nearV.begin(); v_near != v->nearV.end(); v_near++)
			(*v_near) = &vertices[(*v_near)->index];
		for (vector<Face*>::iterator f_near = v->nearF.begin(); f_near != v->nearF.end(); f_near++)
			(*f_near) = &faces[(*f_near)->index];
		for (vector<Edge*>::iterator e_near = v->nearE.begin(); e_near != v->nearE.end(); e_near++)
			(*e_near) = &edges[(*e_near)->index];
	}
	for (vector<Face>::iterator f = faces.begin(); f != faces.end(); f++)
		for (vector<Vertex*>::iterator v_near = f->nearV.begin(); v_near != f->nearV.end(); v_near++)
			(*v_near) = &vertices[(*v_near)->index];
	for (vector<Edge>::iterator e = edges.begin(); e != edges.end(); e++)
	{
		e->fromV = &vertices[e->fromV->index];
		e->toV = &vertices[e->toV->index];
	}
}

Step1::Step1(string file)
{
	vertices.reserve(MAXVEXTICES);
	faces.reserve(MAXFACES);
	edges.reserve(MAXEDGES);

	ifstream in;
	in.open(file.c_str());
	if (!in)
	{
		cout << "Error: obj file not exists." << endl;
		system("pause");
		exit(0);
	}

	string line;
	int vIndex = 0, fIndex = 0, eIndex = 0;
	while (getline(in, line))
	{
		if (line.size() == 0 || line[0] == '#') continue;

		istringstream is(line);
		string word;
		is >> word;

		if (word == "v")	// 顶点坐标
		{
			// 初始化Vertices的坐标和index
			float x, y, z;
			is >> x >> y >> z;
			Vertex v(x, y, z);
			v.index = vIndex++;
			vertices.push_back(v);
		}
		else if (word == "vt") { continue; }
		else if (word == "vn") { continue; }
		else if (word == "o" || word == "g") { continue; }
		else if (word == "f")	// 面: 顶点坐标 || 顶点坐标/ || 顶点坐标/纹理坐标/法向量
		{
			// 初始化Faces的nearV
			string str1, str2, str3;
			is >> str1 >> str2 >> str3;

			int v1, v2, v3, num;
			if (str1.find("/") == -1)
			{
				v1 = atoi(str1.c_str());
				v2 = atoi(str2.c_str());
				v3 = atoi(str3.c_str());
			}
			else
			{
				num = str1.find("/");
				v1 = atoi(string(str1.begin(), str1.begin() + num).c_str());
				num = str2.find("/");
				v2 = atoi(string(str2.begin(), str2.begin() + num).c_str());
				num = str3.find("/");
				v3 = atoi(string(str3.begin(), str3.begin() + num).c_str());
			}

			Face f(&vertices[--v1], &vertices[--v2], &vertices[--v3]);
			f.index = fIndex++;
			faces.push_back(f);

			// 初始化Vertices的nearF
			vertices[v1].nearF.push_back(&faces[faces.size() - 1]);
			vertices[v2].nearF.push_back(&faces[faces.size() - 1]);
			vertices[v3].nearF.push_back(&faces[faces.size() - 1]);

			// 初始化Edges的fromV和toV, 以及初始化Vertices的nearE和nearV
			addNearEandNearV(v1, v2, eIndex);
			addNearEandNearV(v1, v3, eIndex);
			addNearEandNearV(v2, v1, eIndex);
			addNearEandNearV(v2, v3, eIndex);
			addNearEandNearV(v3, v1, eIndex);
			addNearEandNearV(v3, v2, eIndex);
		}
		else if (word == "mtllib") { continue; }
		else if (word == "usemtl") { continue; }
	}
	in.close();
	num = vIndex;

	cout << "------Step1 finish.------" << endl;
}

void Step1::addNearEandNearV(int from, int to, int& eIndex)
{
	bool flag = true;
	for (vector<Edge*>::iterator existE = vertices[from].nearE.begin(); existE != vertices[from].nearE.end(); existE++)
		if ((*existE)->toV == &vertices[to])
		{
			flag = false;
			break;
		}
	if (flag)
	{
		Edge e = Edge(&vertices[from], &vertices[to]);
		e.index = eIndex++;
		edges.push_back(e);
		vertices[from].nearE.push_back(&edges[edges.size() - 1]);
		vertices[from].nearV.push_back(&vertices[to]);
	}
}

void Step1::execute()
{
	int iterNum = 0;
	float avgArea;
	float objVolume;
	MatrixXf matrixV;
	SparseMatrixType matrixL;
	SparseMatrixType matrixWL;
	SparseMatrixType matrixWH;
	SparseMatrixType matrixA;
	MatrixXf matrixb;
	MatrixXf matrixx;

	float volume0 = volume();
	do
	{
 		cout << "\nStep1: iter" << iterNum << endl;
		matrixV = getMatrixV(iterNum, matrixx);
		avgArea = computeArea(iterNum);

		matrixL = getMatrixL();
		matrixWL = getMatrixWL(iterNum, matrixWL, avgArea);
		matrixWH = getMatrixWH(iterNum, matrixWH);
		matrixA = getMatrixA(matrixL, matrixWL, matrixWH);
		matrixb = getMatrixb(matrixWH, matrixV);
		matrixx = solve(matrixA, matrixb);
		for (vector<Vertex>::iterator v = vertices.begin(); v != vertices.end(); v++)
		{
			v->x = matrixx(v->index, 0);
			v->y = matrixx(v->index, 1);
			v->z = matrixx(v->index, 2);
		}

		objVolume = volume();
		cout << "Volume: " << objVolume << endl;
		
		iterNum++;
		output(iterNum);
	} //while (objVolume > 0.001*volume0);
	while (1);

	output();
	cout << "------Step1 finish.------" << endl;
}

float Step1::computeArea(int iterNum)
{
	float avgArea = 0;
	for (vector<Face>::iterator f = faces.begin(); f != faces.end(); f++)
	{
		f->computeArea();
		avgArea += f->area;
	}
	avgArea /= faces.size();
	cout << "Average Area: " << avgArea << endl;

	for (vector<Vertex>::iterator v = vertices.begin(); v != vertices.end(); v++)
	{
		v->computeOneRingArea();
		if (iterNum == 0) v->oneRingArea0 = v->oneRingArea;
	}

	cout << "computeArea() finish." << endl;
	return avgArea;
}

MatrixXf Step1::getMatrixV(int iterNum, MatrixXf& oldMatrixV)
{
	MatrixXf matrixV(num, 3);
	if (iterNum == 0)
	{
		for (int i = 0; i < num; i++)
		{
			matrixV(i, 0) = vertices[i].x;
			matrixV(i, 1) = vertices[i].y;
			matrixV(i, 2) = vertices[i].z;
		}
	}
	else matrixV = oldMatrixV;

	cout << "getMatrixV() finish." << endl;
	return matrixV;
}

SparseMatrixType Step1::getMatrixL()
{
	SparseMatrixType matrixL(num, num);
	matrixL.setZero();
	for (vector<Vertex>::iterator v_self = vertices.begin(); v_self != vertices.end(); v_self++)
	{
		for (vector<Vertex*>::iterator v_near = v_self->nearV.begin(); v_near != v_self->nearV.end(); v_near++)
		{
			for (vector<Vertex*>::iterator v_self_near = v_self->nearV.begin(); v_self_near != v_self->nearV.end(); v_self_near++)
				for (vector<Vertex*>::iterator v_near_near = (*v_near)->nearV.begin(); v_near_near != (*v_near)->nearV.end(); v_near_near++)
					if (*v_self_near == *v_near_near)
					{
						Vertex* v_common = *(v_self_near);
						Vector3f edge1 = v_common->getVector() - (*v_near)->getVector();
						Vector3f edge2 = v_common->getVector() - v_self->getVector();
						float cosine = edge1.dot(edge2) / (edge1.norm() + 1e-10) / (edge2.norm() + 1e-10);
						if (cosine > 0.99999) cosine = 0.99999;
						else if (cosine < -0.99999) cosine = -0.99999;
						float theta = acos(cosine);
						matrixL.coeffRef(v_self->index, (*v_near)->index) += 1 / (tan(theta) + 1e-10);
						matrixL.coeffRef(v_self->index, v_self->index) -= 1 / (tan(theta) + 1e-10);
					}
		}
	}
	cout << "getMatrixL() finish." << endl;
	return matrixL;
}

SparseMatrixType Step1::getMatrixWL(int iterNum, SparseMatrixType& oldMatrixWL, float avgArea)
{
	SparseMatrix<float> matrixWL(num, num);
	matrixWL.setIdentity();
	if (iterNum == 0) matrixWL *= 0.001 * sqrt(avgArea);
	else matrixWL = oldMatrixWL * 2;
	cout << "getMatrixWL() finish." << endl;
	return matrixWL;
}

SparseMatrixType Step1::getMatrixWH(int iterNum, SparseMatrixType& oldMatrixWH)
{
	SparseMatrixType matrixWH(num, num);
	matrixWH.setIdentity();
	if (iterNum > 0)
		for (int i = 0; i < num; i++)
			// matrixWH.coeffRef(i, i) = vertices[i].oneRingArea0 * vertices[i].oneRingArea0 / (vertices[i].oneRingArea + 1e-10) / (vertices[i].oneRingArea + 1e-10);
			matrixWH.coeffRef(i, i) = sqrt(vertices[i].oneRingArea0 / (vertices[i].oneRingArea + 1e-10));
	cout << "getmatrixWH() finish." << endl;
	return matrixWH;
}

SparseMatrixType Step1::getMatrixA(SparseMatrixType& matrixL, SparseMatrixType& matrixWL, SparseMatrixType& matrixWH)
{
	SparseMatrixType matrixA = (matrixWL * matrixL).transpose() * (matrixWL * matrixL);
	matrixA += matrixWH.transpose() * matrixWH;
	cout << "getMatrixA() finish." << endl;
	return matrixA;
}

MatrixXf Step1::getMatrixb(SparseMatrix<float>& matrixWH, MatrixXf& matrixV)
{
	MatrixXf matrixb = matrixWH.transpose() * (matrixWH * matrixV);
	cout << "getMatrixb() finish." << endl;
	return matrixb;
}

MatrixXf Step1::solve(SparseMatrix<float>& matrixA, MatrixXf& matrixb)
{
	SimplicialCholesky<SparseMatrixType> cholesky(matrixA);
	MatrixXf matrixx = cholesky.solve(matrixb);
	
	cout << "error: " << (matrixA*matrixx - matrixb).norm() << endl;
	cout << "solve() finish." << endl;
	return matrixx;
}

float Step1::volume()
{
	float volume = 0;
	for (vector<Face>::iterator face = faces.begin(); face != faces.end(); face++)
	{
		Vector3f edge1 = face->nearV[0]->getVector();
		Vector3f edge2 = face->nearV[1]->getVector();
		Vector3f edge3 = face->nearV[2]->getVector();
		volume += edge1.cross(edge2).dot(edge3);
	}
	return volume;
}

void Step1::output(int iterNum)
{
	ofstream out;
	
	if (iterNum == -1) out.open("Step1.obj");
	else
	{
		char* buffer = new char[10];
		itoa(iterNum, buffer, 10);
		out.open("Step1_" + string(buffer) + ".obj");
	}

	out << "# This is the result after step1." << endl;
	for (vector<Vertex>::iterator v = vertices.begin(); v != vertices.end(); v++)
		out << "v " << v->x << " " << v->y << " " << v->z << endl;
	out << "# There are " << num << " vertices in total." << endl;
	for (vector<Face>::iterator f = faces.begin(); f != faces.end(); f++)
		out << "f " << f->nearV[0]->index + 1 << " " << f->nearV[1]->index + 1 << " " << f->nearV[2]->index + 1 << endl;
	out << "# There are " << faces.size() << " faces in total." << endl;
	cout << "output() finish." << endl;
}