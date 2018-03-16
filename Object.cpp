#include "Object.h"

Object::Object(string file)
{
	ifstream in;
	in.open(file.c_str());
	if (!in)
	{
		cout << "Error: obj file not exists." << endl;
		system("pause");
		exit(0);
	}

	string line;
	int vIndex = 0;
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
			int v1, v2, v3;
			is >> v1 >> v2 >> v3;
			Face f(&vertices[--v1], &vertices[--v2], &vertices[--v3]);
			faces.push_back(f);

			// 初始化Vertices的nearF
			vertices[v1].nearF.push_back(&faces[faces.size()-1]);
			vertices[v2].nearF.push_back(&faces[faces.size()-1]);
			vertices[v3].nearF.push_back(&faces[faces.size()-1]);

			// 初始化Edges的fromV和toV, 以及初始化Vertices的nearE和nearV
			addNearE(v1, v2);
			addNearE(v1, v3);
			addNearE(v2, v1);
			addNearE(v2, v3);
			addNearE(v3, v1);
			addNearE(v3, v2);
		}
		else if (word == "mtllib") { continue; }
		else if (word == "usemtl") { continue; }
	}
	in.close();
}

void Object::addNearE(int from, int to)
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
		edges.push_back(e);
		vertices[from].nearE.push_back(&edges[edges.size() - 1]);
		vertices[from].nearV.push_back(&vertices[to]);
	}
}



void Object::executeStep1()
{
	int iterNum = 0;
	float avgArea;
	MatrixXf matrixV;
	SparseMatrixType matrixL;
	SparseMatrixType matrixWL;
	SparseMatrixType matrixWH;
	SparseMatrixType matrixA;
	MatrixXf matrixb;
	MatrixXf matrixx;

	MatrixXf historyMatrixV[2];
	float historyAvgArea[2] = { 1e10, 1e10 };

	do
	{
		cout << "Step1: iter" << iterNum << endl;
		matrixV = getMatrixV(iterNum, matrixx);
		avgArea = computeArea(iterNum, matrixV);
		historyMatrixV[iterNum % 2] = matrixV;
		historyAvgArea[iterNum % 2] = avgArea;
		if (historyAvgArea[iterNum % 2] > historyAvgArea[(iterNum + 1) % 2]) break;

		matrixL = getMatrixL();
		matrixWL = getMatrixWL(iterNum, matrixWL, avgArea);
		matrixWH = getMatrixWH(iterNum, matrixWH);
		matrixA = getMatrixA(matrixL, matrixWL, matrixWH);
		matrixb = getMatrixb(matrixWH, matrixV);
		matrixx = solve(matrixA, matrixb);
		iterNum++;
	} while (1);

	matrixV = historyMatrixV[(iterNum + 1) % 2];
	for (int i = 0; i < num; i++)
	{
		vertices[i].x = matrixV(i, 0);
		vertices[i].y = matrixV(i, 1);
		vertices[i].z = matrixV(i, 2);
	}
}

float Object::computeArea(int iterNum, MatrixXf matrixV)
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

MatrixXf Object::getMatrixV(int iterNum, MatrixXf& oldMatrixV)
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

SparseMatrixType Object::getMatrixL()
{
	SparseMatrixType matrixL(num, num);
	matrixL.setZero();
	for (int i = 0; i < num; i++)
	{
		Vertex v_self = vertices[i];
		for (int j = 0; j < v_self.nearV.size(); j++)
		{
			Vertex v_near = *(v_self.nearV[j]);
			for (int k = 0; k < v_self.nearV.size(); k++)
				for (int l = 0; l < v_near.nearV.size(); l++)
					if (v_self.nearV[k] == v_near.nearV[l])
					{
						Vertex v_common = *(v_self.nearV[k]);
						Vector3f edge1 = v_self.getVector() - v_near.getVector();
						Vector3f edge2 = v_self.getVector() - v_common.getVector();
						float theta = acos(edge1.dot(edge2) / edge1.norm() / edge2.norm());
						matrixL.coeffRef(i, j) += 1 / (tan(theta) + 1e-10);
						matrixL.coeffRef(i, j) -= 1 / (tan(theta) + 1e-10);
					}
		}
	}
	cout << "getMatrixL() finish." << endl;
	return matrixL;
}

SparseMatrixType Object::getMatrixWL(int iterNum, SparseMatrixType& oldMatrixWL, float avgArea)
{
	SparseMatrix<float> matrixWL(num, num);
	matrixWL.setIdentity();
	if (iterNum == 0) matrixWL *= 0.001 * sqrt(avgArea);
	else matrixWL = oldMatrixWL * 2;
	cout << "getMatrixWL() finish." << endl;
	return matrixWL;
}

SparseMatrixType Object::getMatrixWH(int iterNum, SparseMatrixType& oldMatrixWH)
{
	SparseMatrix<float> matrixWH(num, num);
	matrixWH.setIdentity();
	if (iterNum > 0)
		for (int i = 0; i < num; i++)
			matrixWH.coeffRef(i, i) = sqrt(vertices[i].oneRingArea0 / vertices[i].oneRingArea);
	cout << "getmatrixWH() finish." << endl;
	return matrixWH;
}

SparseMatrix<float> Object::getMatrixA(SparseMatrix<float>& matrixL, SparseMatrix<float>& matrixWL, SparseMatrix<float>& matrixWH)
{
	SparseMatrix<float> matrixA = (matrixWL * matrixL).transpose() * (matrixWL * matrixL) + matrixWH.transpose() * matrixWH;
	cout << "getMatrixA() finish." << endl;
	return matrixA;
}

MatrixXf Object::getMatrixb(SparseMatrix<float>& matrixWH, MatrixXf& matrixV)
{
	MatrixXf matrixb = matrixWH.transpose() * (matrixWH * matrixV);
	cout << "getMatrixb() finish." << endl;
	return matrixb;
}

MatrixXf Object::solve(SparseMatrix<float>& matrixA, MatrixXf& matrixb)
{
	SimplicialCholesky<SparseMatrix<float>> cholesky(matrixA);
	MatrixXf matrixx = cholesky.solve(matrixb);
	cout << "solve() finish." << endl;
	return matrixx;
}







void Object::executeStep2()
{
	for (vector<Vertex>::iterator v = vertices.begin(); v != vertices.end(); v++)
		v->victimV.push_back(&(*v));
	computeMatrixQ();
	computeSumEdge();
	computeF();
	for (int i = 0; i < num - 20; i++)
		collapse();		
}

void Object::computeMatrixQ()
{
	for (vector<Vertex>::iterator v = vertices.begin(); v != vertices.end(); v++)
		v->computeMatrixQ();
	cout << "Step2: computeMatrixQ() finish." << endl;
}

void Object::computeSumEdge()
{
	for (vector<Vertex>::iterator v = vertices.begin(); v != vertices.end(); v++)
		v->computeSumEdges();
	cout << "Step2: computeSumEdge() finish." << endl;
}

void Object::computeF()
{
	for (vector<Vertex>::iterator v_self = vertices.begin(); v_self != vertices.end(); v_self++)
	{
		for (vector<Edge*>::iterator e = v_self->nearE.begin(); e != v_self->nearE.end(); e++)
		{
			Vertex* v_near = (*e)->toV;
			float Fa = (v_near->getVector() * (v_self->matrixQ + v_near->matrixQ) * v_near->getVector().transpose())(0,0);
			float Fb = (*e)->length() * v_self->sumEdges;
			float wa = 1.0;
			float wb = 0.1;
			(*e)->valueF = wa*Fa + wb*Fb;
			heap.insert(*e);
		}
	}
	cout << "Step2: computeF() finish."  << endl;
}

void Object::rmAfromB(Vertex* from, Vertex* to)
{
	for (vector<Edge*>::iterator e = from->nearE.begin(); e != from->nearE.end(); e++)
		if ((*e)->toV == to)	// 找到对应的边, 调用边的删除函数
		{
			rmAfromB((*e));
			break;
		}
}

void Object::rmAfromB(Edge* e)
{
	Vertex* from = e->fromV;
	Vertex* to = e->toV;
	to->sumEdges -= e->length();
	heap.deleteItem(e->heapPos);
	for (vector<Edge*>::iterator iterE = from->nearE.begin(); iterE != from->nearE.end(); iterE++)
		if (*iterE == e)
			from->nearE.erase(iterE);
}

void Object::addAinB(Vertex* from, Vertex* to)
{
	Edge e(from, to);
	e.valueF = 100;		// 任意初始化新边的F值(方便插入堆)
	edges.push_back(e);	// 加入边的集合
	to->sumEdges += e.length();		// 更新顶点边长
	heap.insert(&edges[edges.size() - 1]);	// 插入堆
	from->nearE.push_back(&edges[edges.size()-1]);	// 加入邻边集合
}

void Object::collapse()
{
	Edge* collapsE = heap.heap[1];
	Vertex* from = collapsE->fromV;
	Vertex* to = collapsE->toV;

	for (vector<Edge*>::iterator e = from->nearE.begin(); e != from->nearE.end(); e++)
	{
		if (*e != collapsE)
		{
			rmAfromB((*e)->toV, (*e)->fromV);
			rmAfromB((*e));
			addAinB((*e)->toV, to);
			addAinB(to, (*e)->toV);
		}
		else
		{
			rmAfromB((*e)->toV, (*e)->fromV);
			rmAfromB((*e));
			to->matrixQ += from->matrixQ;
			to->victimV.push_back(from);
		}
	}
	from->isValid = false;

	for (vector<Vertex*>::iterator v = from->nearV.begin(); v != from->nearV.end(); v++)
		if ((*v)->isValid)
		{
			for (vector<Edge*>::iterator e = (*v)->nearE.begin(); e != from->nearE.end(); e++)
			{
				Vertex* v_near = (*e)->toV;
				float Fa = (v_near->getVector() * ((*v)->matrixQ + v_near->matrixQ) * v_near->getVector().transpose())(0,0);
				float Fb = (*e)->length() * (*v)->sumEdges;
				float wa = 1.0;
				float wb = 0.1;
				(*e)->valueF = wa*Fa + wb*Fb;
				heap.update((*e)->heapPos);
			}
		}
}



void Object::refine1()
{
	for (vector<Vertex>::iterator v = vertices.begin(); v != vertices.end(); v++)
		if (v->isValid)
			for (vector<Vertex*>::iterator victim = v->victimV.begin(); victim != v->victimV.end(); victim++)
			{
				(*victim)->center = &(*v);
			}
	cout << "Step3: get center finish." << endl;

	for (vector<Vertex>::iterator v = vertices.begin(); v != vertices.end(); v++)
	{
		v->isBound = false;
		for (vector<Vertex*>::iterator v_near = v->nearV.begin(); v_near != v->nearV.end(); v_near++)
			if (v->center == (*v_near)->center)
			{
				v->isBound = true;
				break;
			}
	}
	cout << "Step3: check boundary finish." << endl;

	for (vector<Vertex>::iterator v = vertices.begin(); v != vertices.end(); v++)
		if (v->isValid)
		{
			v->partTotal = 0;
			for (vector<Vertex*>::iterator victim = v->victimV.begin(); victim != v->victimV.end(); victim++)
				if ((*victim)->isBound && (*victim)->partNum==0)
				{
					v->partTotal++;
					setPartNum((*victim), v->partTotal);
				}
			for (vector<Vertex*>::iterator victim = v->victimV.begin(); victim != v->victimV.end(); victim++)
				if ((*victim)->partNum != 0)
				{
					int len[2] = { 1e10, 1e10 };
					for (vector<Vertex*>::iterator v_near = (*victim)->nearV.begin(); v_near != (*victim)->nearV.end(); v_near++)
						if ((*v_near)->partNum == (*victim)->partNum)
						{
							float length = ((*victim)->getVector() - (*v_near)->getVector()).norm();
							if (length < len[1]) len[1] = length;
							if (length < len[0]) {
								len[1] = len[0]; 
								len[0] = length;
							}
						}
					if (len[1] == 1e10) len[1] = len[0];
					if (len[0] == 1e10)
						while (1) cout << "BUG!!!";
					(*victim)->boundLen = len[0] + len[1];
				}
		}
	cout << "Step3: find boundary parts finish." << endl;
	cout << "Step3: compute boundary length finish." << endl;

	for (vector<Vertex>::iterator v = vertices.begin(); v != vertices.end(); v++)
		if (v->isValid)
		{
			Vector3f res;
			res.setZero();
			for (int i = 0; i < v->partTotal; i++)
			{
				Vector3f numerator;
				numerator.setZero();
				float denominator = 0;
				for (vector<Vertex*>::iterator victim = v->victimV.begin(); victim != v->victimV.end(); victim++)
					if (v->partNum == i)
					{
						denominator += (*victim)->boundLen;
						numerator += (*victim)->boundLen * (vertices_orig[(*victim)->index].getVector() - (*victim)->getVector());
					}
				res += numerator / denominator;
			}
			Vector3f displace = res / v->partTotal;
			v->x -= displace(0);
			v->y -= displace(1);
			v->z -= displace(2);
		}
	cout << "Step3: compute displacement finish." << endl;
	cout << "Step3: refine1() finish." << endl;
}

void Object::setPartNum(Vertex* start, int partNum)
{
	start->partNum = partNum;
	for (vector<Vertex*>::iterator v_near = start->nearV.begin(); v_near != start->nearV.end(); v_near++)
		if ((*v_near)->isBound && (*v_near)->center == start->center && (*v_near)->partNum==0)
			setPartNum((*v_near), partNum);
}

void Object::refine2()
{
	for (vector<Vertex>::iterator v = vertices.begin(); v != vertices.end(); v++)
		if (v->isValid)
		{
			v->meanPos.setZero();
			int number = 0;
			for (vector<Vertex*>::iterator victim = v->victimV.begin(); victim != v->victimV.end(); victim++)
			{
				v->meanPos += (*victim)->getVector();
				number++;
			}
			v->meanPos /= number;
			for (vector<Vertex*>::iterator victim = v->victimV.begin(); victim != v->victimV.end(); victim++)
			{
				v->stdDev += ((*victim)->getVector() - v->meanPos).norm() * ((*victim)->getVector() - v->meanPos).norm();
			}
			v->stdDev /= number;
			v->stdDev = sqrt(v->stdDev);
		}
	cout << "Step3: compute standard deviation finish." << endl;

	for (vector)
}