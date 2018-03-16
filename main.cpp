#include "head.h"
#include "Vertex.h"
#include "Face.h"
#include "Edge.h"
#include "Step0.h"
#include "Step1.h"
#include "Step2.h"
#include "Step3.h"

int main()
{
	cout << setprecision(10);
	Step0 obj0("bighead.obj");
	Step1 obj1(&obj0);
	obj1.execute();
	Step1 obj1("Step1_18.obj");
	Step2 obj2(&obj0, &obj1);
	obj2.execute();
	Step3 obj3(&obj0, &obj1, &obj2);
	obj3.execute();
	system("pause");
}