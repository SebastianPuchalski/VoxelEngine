/*
Module: SimpleObjectPool
Author: Sebastian Puchalski
Date: 29.05.2014
*/

#ifndef SIMPLE_OBJECT_POOL_HPP_
#define SIMPLE_OBJECT_POOL_HPP_

#include "../common.hpp"

#include <vector>

template <typename T>
class SimpleObjectPool
{
public:
	SimpleObjectPool(uint pageSize);
	virtual ~SimpleObjectPool();

	T * create();
	T * create(uint number);

private:
	const uint pageSize;
	std::vector<T *> pages;
	int pointer;
};

template <typename T>
SimpleObjectPool<T>::SimpleObjectPool(uint pageSize) : pageSize(pageSize), pointer(pageSize)
{
}

template <typename T>
SimpleObjectPool<T>::~SimpleObjectPool()
{
	for(uint i = 0; i < pages.size(); i++)
		delete [] pages[i];
}

template <typename T>
T * SimpleObjectPool<T>::create()
{
	if(pointer == pageSize)
	{
		pages.push_back(new T[pageSize]);
		pointer = 0;
	}
	T * object = pages[pages.size()-1] + pointer;
	pointer++;
	return object;
}

template <typename T>
T * SimpleObjectPool<T>::create(uint number)
{
	if(number > pageSize)
		return 0;
	if(pointer + number > pageSize)
	{
		pages.push_back(new T[pageSize]);
		pointer = 0;
	}
	T * object = pages[pages.size()-1] + pointer;
	pointer += number;
	return object;
}

#endif //SIMPLE_OBJECT_POOL_HPP_
