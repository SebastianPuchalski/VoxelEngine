/*
Module: HugeObjectPool
Author: Sebastian Puchalski
Date: 04.06.2014
*/

#ifndef HUGE_OBJECT_POOL_HPP_
#define HUGE_OBJECT_POOL_HPP_

#include "../common.hpp"

#include <string>
#include <sstream>
#include <vector>
#include <fstream>

struct Handle;

template <typename T>
class HugeObjectPool
{
public:
	HugeObjectPool(uint32 pageSize, uint64 memoryLimit);
	virtual ~HugeObjectPool();

	Handle create();
	Handle create(size_t number);

	T * get(Handle handle);

private:
	struct Page
	{
		T * objects;
		uint64 counter;
	};

	const uint32 pageSize; //number of objects in page
	const uint32 maxPageNumber; //maximum number of pages in memory

	std::vector<Page> pages;
	uint32 pointer; //how many objects of last page are created
	uint32 pageNumber; //current number of pages in memory
	uint64 counter;

	void restorePage(uint32 page); //load page to memory from file
	T * obtainMemory(); //allocate or take memory from another page
	std::string getFileName(uint32 pageNumber) const;
};

struct Handle
{
	Handle() : page(0), index(-1) {}
	operator bool() const { return index != -1; }

	uint32 page;
	int32 index;
};

const std::string PREFIX = "pages/page";
const std::string SUFFIX = ".data";

template <typename T>
HugeObjectPool<T>::HugeObjectPool(uint32 pageSize, uint64 memoryLimit) : 
	pageSize(pageSize), maxPageNumber((uint32)(memoryLimit / (sizeof(T) * pageSize))), 
		pointer(pageSize), pageNumber(0), counter(0)
{
}

template <typename T>
HugeObjectPool<T>::~HugeObjectPool()
{
	for(uint i = 0; i < pages.size(); i++)
		if(pages[i].objects)
			delete [] pages[i].objects;
}

template <typename T>
Handle HugeObjectPool<T>::create()
{
	if(pointer == pageSize)
	{
		Page page;
		page.objects = obtainMemory();
		page.counter = counter;
		pages.push_back(page);
		pointer = 0;
	}

	Handle handle;
	handle.page = pages.size() - 1;
	handle.index = pointer;
	pointer++;
	return handle;
}

template <typename T>
Handle HugeObjectPool<T>::create(size_t number)
{
	if(number > pageSize)
		throw "Error during creating of object table (table size is bigger than page size)!\n";

	if(pointer + number > pageSize)
	{
		Page page;
		page.objects = obtainMemory();
		page.counter = counter;
		pages.push_back(page);
		pointer = 0;
	}

	Handle handle;
	handle.page = (uint32)(pages.size() - 1);
	handle.index = pointer;
	pointer += (uint32)number;
	return handle;
}

template <typename T>
T * HugeObjectPool<T>::get(Handle handle)
{
	Page & page = pages[handle.page];

	if(page.objects == 0)
		restorePage(handle.page);

	page.counter = ++counter;
	return page.objects + handle.index;
}

template <typename T>
void HugeObjectPool<T>::restorePage(uint32 page)
{
	pages[page].objects = obtainMemory();

	std::ifstream file(getFileName(page), std::ios::binary);
	if(!file.is_open())
		throw std::string("Can not open file!\n");

	file.read((char *)pages[page].objects, sizeof(T) * pageSize);
	if(file.fail() || file.bad())
		throw std::string("Error during reading from file!\n");
}

template <typename T>
T * HugeObjectPool<T>::obtainMemory()
{
	if(pageNumber < maxPageNumber)
	{
		pageNumber++;
		return new T[pageSize];
	}

	uint32 oldPage = 0;
	uint64 maxDifference = 0;
	for(uint32 i = 0; i < pages.size(); i++)
	{
		if(pages[i].objects)
		{
			uint64 difference = counter - pages[i].counter;
			if(difference > maxDifference)
			{
				maxDifference = difference;
				oldPage = i;
			}
		}
	}
	T * memory = pages[oldPage].objects;
	pages[oldPage].objects = 0;

	std::ofstream file(getFileName(oldPage), std::ios::binary);
	if(!file.is_open())
		throw std::string("Can not open file!\n");

	file.write((char *)memory, sizeof(T) * pageSize);
	if(file.fail() || file.bad())
		throw std::string("Error during writing to file!\n");
	
	return memory;
}

template <typename T>
std::string HugeObjectPool<T>::getFileName(uint32 pageNumber) const
{
	std::stringstream ss;
	ss << PREFIX << pageNumber << SUFFIX;
	return ss.str();
}

#endif //HUGE_OBJECT_POOL_HPP_
