#include "Step0.h"

Step0::Step0(string file)
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

	output();
	cout << "------Step0 finish.------" << endl;
}

void Step0::addNearEandNearV(int from, int to, int& eIndex)
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

void Step0::output()
{
	ofstream out;
	out.open("Step0.obj");
	out << "# This is the original obj." << endl;
	for (vector<Vertex>::iterator v = vertices.begin(); v != vertices.end(); v++)
		out << "v " << v->x << " " << v->y << " " << v->z << endl;
	out << "# There are " << num << " vertices in total." << endl;
	for (vector<Face>::iterator f = faces.begin(); f != faces.end(); f++)
		out << "f " << f->nearV[0]->index + 1 << " " << f->nearV[1]->index + 1 << " " << f->nearV[2]->index + 1 << endl;
	out << "# There are " << faces.size() << " faces in total." << endl;
}