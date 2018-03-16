#include "Step2.h"

Step2::Step2(Step0* step0, Step1* step1)
{
	vertices.reserve(MAXVEXTICES);
	faces.reserve(MAXFACES);
	edges.reserve(MAXEDGES);

	obj0 = step0;
	obj1 = step1;
	num = obj1->num;
	vertices = obj1->vertices;
	faces = obj1->faces;
	edges = obj1->edges;
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

void Step2::execute()
{
	// victimV的初始化
	for (vector<Vertex>::iterator v = vertices.begin(); v != vertices.end(); v++)
		v->victimV.push_back(&(*v));
	computeMatrixQ();
	computeSumEdge();
	computeF();
	for (int i = 0; i < num - 10; i++)
	{
		collapse();
	}
	output();

	cout << "------Step2 finish.------" << endl;
}

void Step2::computeMatrixQ()
{
	for (vector<Vertex>::iterator v = vertices.begin(); v != vertices.end(); v++)
		v->computeMatrixQ();
	cout << "Step2: computeMatrixQ() finish." << endl;
}

void Step2::computeSumEdge()
{
	for (vector<Vertex>::iterator v = vertices.begin(); v != vertices.end(); v++)
		v->computeSumEdges();
	cout << "Step2: computeSumEdge() finish." << endl;
}

void Step2::computeF()
{
	for (vector<Vertex>::iterator v_self = vertices.begin(); v_self != vertices.end(); v_self++)
	{
		for (vector<Edge*>::iterator e = v_self->nearE.begin(); e != v_self->nearE.end(); e++)
		{
			Vertex* v_near = (*e)->toV;
			Vector4f v_near_vec;
			v_near_vec << v_near->x, v_near->y, v_near->z, 1.0;
			float Fa = (v_near_vec.transpose() * (v_self->matrixQ + v_near->matrixQ) * v_near_vec)(0, 0);
			float Fb = (*e)->length() * v_self->sumEdges;
			float wa = 1.0;
			float wb = 0.1;
			(*e)->valueF = wa*Fa + wb*Fb;
			heap.insert(*e);
		}
	}
	cout << "Step2: computeF() finish." << endl;
}

vector<Edge*>::iterator Step2::rmAfromB(Vertex* from, Vertex* to)
{
	for (vector<Edge*>::iterator e = from->nearE.begin(); e != from->nearE.end(); e++)
		if ((*e)->toV == to)	// 找到对应的边, 调用边的删除函数
		{
			e = rmAfromB((*e));
			return e;
		}
}

vector<Edge*>::iterator Step2::rmAfromB(Edge* e)
{
	Vertex* from = e->fromV;
	Vertex* to = e->toV;
	to->sumEdges -= e->length();						// 更新顶点边长和
	heap.deleteItem(e->heapPos);						// 删除堆的元素
	for (vector<Edge*>::iterator iterE = from->nearE.begin(); iterE != from->nearE.end(); iterE++)
		if (*iterE == e)
		{
			iterE = from->nearE.erase(iterE);					// 移出邻边集合
			return iterE;
		}
}

void Step2::addAinB(Vertex* from, Vertex* to)
{
	Edge e(from, to);
	e.index = edges.size();								// 初始化index
	e.valueF = 100;										// 任意初始化新边的F值(方便插入堆)
	edges.push_back(e);									// 加入边的集合
	to->sumEdges += e.length();							// 更新顶点边长和
	heap.insert(&edges[edges.size() - 1]);				// 插入堆
	from->nearE.push_back(&edges[edges.size() - 1]);	// 加入邻边集合
	from->nearV.push_back(to);							// 加入邻点集合
}

void Step2::collapse()
{
	Edge* collapseE = heap.heap[1];
	Vertex* from = collapseE->fromV;
	Vertex* to = collapseE->toV;

	for (vector<Edge*>::iterator e = from->nearE.begin(); e != from->nearE.end();)
	{
		if (*e != collapseE)
		{
			bool flag = true;
			for (vector<Vertex*>::iterator v = to->nearV.begin(); v != to->nearV.end(); v++)
				if (*v == (*e)->toV)
				{
					flag = false;
					break;
				}
			if (flag)
			{
				addAinB((*e)->toV, to);
				addAinB(to, (*e)->toV);
			}
			// 先加上新边, 再删除旧边. 否则erase迭代器后将无法使用
			rmAfromB((*e)->toV, (*e)->fromV);
			e = rmAfromB((*e));	// 得到erase后的迭代器
		}
		else
		{
			to->matrixQ += from->matrixQ;
			for (vector<Vertex*>::iterator victim = from->victimV.begin(); victim != from->victimV.end(); victim++)
				to->victimV.push_back(*victim);
			rmAfromB((*e)->toV, (*e)->fromV);
			e = rmAfromB((*e));	// 得到erase后的迭代器
		}
	}
	from->isValid = false;

	for (vector<Vertex*>::iterator v = from->nearV.begin(); v != from->nearV.end(); v++)
		if ((*v)->isValid)
		{
			for (vector<Edge*>::iterator e = (*v)->nearE.begin(); e != (*v)->nearE.end(); e++)
			{
				Vertex* v_near = (*e)->toV;
				Vector4f v_near_vec;
				v_near_vec << v_near->x, v_near->y, v_near->z, 1.0;
				float Fa = (v_near_vec.transpose() * ((*v)->matrixQ + v_near->matrixQ) * v_near_vec)(0, 0);
				float Fb = (*e)->length() * (*v)->sumEdges;
				float wa = 1.0;
				float wb = 0.1;
				(*e)->valueF = wa*Fa + wb*Fb;
				heap.update((*e)->heapPos);
			}
		}
}

void Step2::output()
{
	int numV = 0, numF = 0;
	ofstream out;
	out.open("Step2.obj");
	out << "# This is the result after step2." << endl;
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
				out << "f " << 2*v->validIndex + 1 << " " << 2*v->validIndex + 2 << " " << 2*(*e)->toV->validIndex + 1 << endl;
				numF++;
			}
	out << "# There are " << numF << " faces in total." << endl;
}