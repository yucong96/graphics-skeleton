#include "Heap.h"

Heap::Heap()
{
	size = 0;
	heap.push_back(NULL);
}

void Heap::insert(Edge* item)
{
	size++;
	item->setHeapPos(size);
	heap.push_back(item);
	upcascade(size);
}

void Heap::deleteItem(int index)
{
	if (index == size)
	{
		heap[size]->setHeapPos(-1);
		heap.pop_back();
		size--;
	}
	else
	{
		swap(index, size);
		heap[size]->setHeapPos(-1);
		heap.pop_back();
		size--;
		update(index);
	}
}

void Heap::update(int index)
{
	upcascade(index);
	downcascade(index);
}

void Heap::upcascade(int index)
{
	int num = index;
	while (num / 2 > 0 && getValue(num) < getValue(num / 2))
	{
		swap(num, num / 2);
		num /= 2;
	}
}

void Heap::downcascade(int index)
{
	int num = index;
	while (1)
	{
		if (num * 2 == size)
		{
			if (getValue(num) > getValue(num * 2))
			{
				swap(num, num * 2);
				num = 2 * num;
			}
			else break;
		}
		else if (num * 2 + 1 <= size)
		{
			if (getValue(num) > getValue(num * 2) || getValue(num) > getValue(num * 2 + 1))
			{
				if (getValue(num * 2 + 1) > getValue(num * 2))
				{
					swap(num, num * 2);
					num = 2 * num;
				}
				else
				{
					swap(num, num * 2 + 1);
					num = 2 * num + 1;
				}
			}
			else break;
		}
		else break;
	}
}

void Heap::swap(int index1, int index2)
{
	Edge* temp = heap[index1];
	heap[index1] = heap[index2];
	heap[index2] = temp;
	heap[index1]->setHeapPos(index1);
	heap[index2]->setHeapPos(index2);
}

float Heap::getValue(int index)
{
	return heap[index]->getValueF();
}