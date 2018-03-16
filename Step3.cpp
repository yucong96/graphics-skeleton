#include "Step3.h"

Step3::Step3(Step0* step0, Step1* step1, Step2* step2)
{
	vertices.reserve(MAXVEXTICES);
	faces.reserve(MAXFACES);
	edges.reserve(MAXEDGES);

	obj0 = step0;
	obj1 = step1;
	obj2 = step2;
	num = obj2->num;
	vertices = obj2->vertices;
	faces = obj2->faces;
	edges = obj2->edges;

	for (vector<Vertex>::iterator v = vertices.begin(); v != vertices.end(); v++)
	{
		v->oldX = obj0->vertices[v->index].x;
		v->oldY = obj0->vertices[v->index].y;
		v->oldZ = obj0->vertices[v->index].z;
		v->partNum = 0;
		v->stdDev = 0;
		v->oldNearV = obj0->vertices[v->index].nearV;
		for (vector<Vertex*>::iterator v_near = v->oldNearV.begin(); v_near != v->oldNearV.end(); v_near++)
			(*v_near) = &vertices[(*v_near)->index];
		v->nearV = obj2->vertices[v->index].nearV;
		for (vector<Vertex*>::iterator v_near = v->nearV.begin(); v_near != v->nearV.end(); v_near++)
			(*v_near) = &vertices[(*v_near)->index];
		v->nearE = obj2->vertices[v->index].nearE;
		for (vector<Edge*>::iterator e_near = v->nearE.begin(); e_near != v->nearE.end(); e_near++)
			(*e_near) = &edges[(*e_near)->index];
		v->victimV = obj2->vertices[v->index].victimV;
		for (vector<Vertex*>::iterator victim = v->victimV.begin(); victim != v->victimV.end(); victim++)
			(*victim) = &vertices[(*victim)->index];
	}
	for (vector<Edge>::iterator e = edges.begin(); e != edges.end(); e++)
	{
		e->fromV = &vertices[e->fromV->index];
		e->toV = &vertices[e->toV->index];
	}
}

void Step3::execute()
{
	refine1();
	refine2();

	cout << "------Step3 finish.------" << endl;
}

void Step3::refine1()
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
		for (vector<Vertex*>::iterator v_near = v->oldNearV.begin(); v_near != v->oldNearV.end(); v_near++)
			if (v->center != (*v_near)->center)
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
			if (v->victimV.size() == 1)
			{
				v->partTotal = 1;
				v->partNum = 1;
				v->boundLen = 0;
				continue;
			}
			for (vector<Vertex*>::iterator victim = v->victimV.begin(); victim != v->victimV.end(); victim++)
				if ((*victim)->isBound && (*victim)->partNum == 0)
				{
					v->partTotal++;
					setPartNum((*victim), v->partTotal);
				}
			for (vector<Vertex*>::iterator victim = v->victimV.begin(); victim != v->victimV.end(); victim++)
				if ((*victim)->partNum != 0)
				{
					float len[2] = { 1e10, 1e10 };
					for (vector<Vertex*>::iterator v_near = (*victim)->oldNearV.begin(); v_near != (*victim)->oldNearV.end(); v_near++)
						if ((*v_near)->center == (*victim)->center && (*v_near)->partNum == (*victim)->partNum)
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
			for (int i = 1; i <= v->partTotal; i++)
			{
				Vector3f numerator;
				numerator.setZero();
				float denominator = 0;
				for (vector<Vertex*>::iterator victim = v->victimV.begin(); victim != v->victimV.end(); victim++)
					if ((*victim)->partNum == i)
					{
						denominator += (*victim)->boundLen;
						numerator += (*victim)->boundLen * (obj0->vertices[(*victim)->index].getVector() - (*victim)->getVector());
					}
				res += numerator / (denominator + 1e-10);
			}
			Vector3f displace = res / (v->partTotal + 1e-10);
			v->x -= displace(0);
			v->y -= displace(1);
			v->z -= displace(2);
		}
	cout << "Step3: compute displacement finish." << endl;
	cout << "Step3: refine1() finish." << endl;

	output();
}

void Step3::setPartNum(Vertex* start, int partNum)
{
	start->partNum = partNum;
	for (vector<Vertex*>::iterator v_near = start->oldNearV.begin(); v_near != start->oldNearV.end(); v_near++)
		if ((*v_near)->isBound && (*v_near)->center == start->center && (*v_near)->partNum == 0)
			setPartNum((*v_near), partNum);
}

void Step3::refine2()
{
	for (vector<Vertex>::iterator v = vertices.begin(); v != vertices.end(); v++)
		if (v->isValid) computeStdDev(&(*v));
	cout << "Step3: compute standard deviation finish." << endl;

	bool flag = true;
	while (flag)
	{
		flag = false;
		for (vector<Vertex>::iterator v = vertices.begin(); v != vertices.end(); v++)
			if (v->isValid)
				for (vector<Edge*>::iterator e = v->nearE.begin(); e != v->nearE.end(); e++)	// obj2的nearE
					if (computeStdDev(&(*v), (*e)->toV) < 0.9 * v->stdDev)
					{
						flag = true;
						Vertex* from = &(*v);
						Vertex* to = (*e)->toV;
						Edge* collapseE = *e;
						for (vector<Edge*>::iterator ee = from->nearE.begin(); ee != from->nearE.end(); )
						{
							if (*ee != collapseE)
							{
								bool flag = true;
								for (vector<Vertex*>::iterator v = to->nearV.begin(); v != to->nearV.end(); v++)
									if (*v == (*ee)->toV)
									{
										flag = false;
										break;
									}
								if (flag)
								{
									addAinB((*ee)->toV, to);
									addAinB(to, (*ee)->toV);
								}
								rmAfromB((*ee)->toV, (*ee)->fromV);
								ee = rmAfromB((*ee));
							}
							else
							{
								for (vector<Vertex*>::iterator victim = from->victimV.begin(); victim != from->victimV.end(); victim++)
									to->victimV.push_back(*victim);
								rmAfromB((*ee)->toV, (*ee)->fromV);
								ee = rmAfromB((*ee));
							}
						}
						from->isValid = false;
						break;
					}
	}

	cout << "Step3: refine2() finish." << endl;
	output();
}

void Step3::computeStdDev(Vertex* v)
{
	Vector3f meanPos;
	meanPos.setZero();
	int number = 0;
	for (vector<Vertex*>::iterator victim = v->victimV.begin(); victim != v->victimV.end(); victim++)
	{
		meanPos += obj0->vertices[(*victim)->index].getVector();
		number++;
	}
	meanPos /= number;
	for (vector<Vertex*>::iterator victim = v->victimV.begin(); victim != v->victimV.end(); victim++)
	{
		v->stdDev += (obj0->vertices[(*victim)->index].getVector() - meanPos).norm() * (obj0->vertices[(*victim)->index].getVector() - meanPos).norm();
	}
	v->stdDev /= number;
	v->stdDev = sqrt(v->stdDev);
}

float Step3::computeStdDev(Vertex* v1, Vertex* v2)
{
	Vector3f meanPos;
	meanPos.setZero();
	int number = 0;
	for (vector<Vertex*>::iterator victim = v1->victimV.begin(); victim != v1->victimV.end(); victim++)
	{
		meanPos += obj0->vertices[(*victim)->index].getVector();
		number++;
	}
	for (vector<Vertex*>::iterator victim = v2->victimV.begin(); victim != v2->victimV.end(); victim++)
	{
		meanPos += obj0->vertices[(*victim)->index].getVector();
		number++;
	}
	meanPos /= number;
	float stdDev = 0;
	for (vector<Vertex*>::iterator victim = v1->victimV.begin(); victim != v1->victimV.end(); victim++)
		stdDev += (obj0->vertices[(*victim)->index].getVector() - meanPos).norm() * (obj0->vertices[(*victim)->index].getVector() - meanPos).norm();
	for (vector<Vertex*>::iterator victim = v2->victimV.begin(); victim != v2->victimV.end(); victim++)
		stdDev += (obj0->vertices[(*victim)->index].getVector() - meanPos).norm() * (obj0->vertices[(*victim)->index].getVector() - meanPos).norm();
	stdDev /= number;
	stdDev = sqrt(stdDev);
	return stdDev;
}

vector<Edge*>::iterator Step3::rmAfromB(Vertex* from, Vertex* to)
{
	for (vector<Edge*>::iterator e = from->nearE.begin(); e != from->nearE.end(); e++)
		if ((*e)->toV == to)	// 找到对应的边, 调用边的删除函数
		{
			e = rmAfromB((*e)); 
			return e;
		}
}

vector<Edge*>::iterator Step3::rmAfromB(Edge* e)
{
	Vertex* from = e->fromV;
	Vertex* to = e->toV;
	for (vector<Edge*>::iterator iterE = from->nearE.begin(); iterE != from->nearE.end(); iterE++)
		if (*iterE == e)
		{
			iterE = from->nearE.erase(iterE);					// 移出邻边集合
			return iterE;
		}
}

void Step3::addAinB(Vertex* from, Vertex* to)
{
	Edge e(from, to);
	edges.push_back(e);									// 加入边的集合
	from->nearE.push_back(&edges[edges.size() - 1]);	// 加入邻边集合
}

void Step3::output()
{
	int numV = 0, numF = 0;
	ofstream out;
	out.open("Step3.obj");
	out << "# This is the result after step3." << endl;
	for (vector<Vertex>::iterator v = vertices.begin(); v != vertices.end(); v++)
		if (v->isValid)
		{
			out << "v " << v->x << " " << v->y << " " << v->z << endl;
			out << "v " << v->x << " " << v->y << " " << v->z << endl;
			v->validIndex = numV;
			numV++;
		}
	out << "# There are " << numV << " vertices in total." << endl;
	for (vector<Vertex>::iterator v = vertices.begin(); v != vertices.end(); v++)
		if (v->isValid)
			for (vector<Edge*>::iterator e = v->nearE.begin(); e != v->nearE.end(); e++)
			{
				out << "f " << 2 * v->validIndex + 1 << " " << 2 * v->validIndex + 2 << " " << 2 * (*e)->toV->validIndex + 1 << endl;
				numF++;
			}
	out << "# There are " << numF << " faces in total." << endl;
}
