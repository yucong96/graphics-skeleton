#pragma once
#include "head.h"
#include "Edge.h"

class Heap
{
public:
	vector<Edge*> heap;
	int size;

	Heap();
	void insert(Edge* item);
	void deleteItem(int index);
	void update(int index);
	void upcascade(int index);
	void downcascade(int index);
	void swap(int index1, int index2);
	float getValue(int index);
};