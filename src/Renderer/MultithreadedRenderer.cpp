#include "MultithreadedRenderer.hpp"

#include <cassert>

MultithreadedRenderer::MultithreadedRenderer(int width, int height, bool switchRB, 
	vector2i tileNumber, int threadNumber) : 
	width(width), height(height), switchRB(switchRB), 
	sceneData(0), cameraPtr(0), fieldOfView(0), clearColor(0, 0, 0), aa(true), 
	sceneDataFlag(false), fieldOfViewFlag(false), clearColorFlag(false), aaFlag(false), 
	firstActive(true)
{
	buffer1 = new color24[width * height];
	buffer2 = new color24[width * height];
	for(int i = 0; i < width * height; i++)
		buffer1[i] = buffer2[i] = clearColor;

	work = CreateEvent(NULL, FALSE, TRUE, NULL);
	InitializeCriticalSection(&(mainSection));

	createTiles(tileNumber);
	createThreads(threadNumber);
}

MultithreadedRenderer::~MultithreadedRenderer()
{
	destroyThreads();
	destroyTiles();

	DeleteCriticalSection(&(mainSection));

	delete [] buffer1;
	delete [] buffer2;
}

void MultithreadedRenderer::setSceneData(uint8 * data)
{
	sceneData = data;
	sceneDataFlag = true;
}

void MultithreadedRenderer::setCamera(Camera * camera)
{
	this->cameraPtr = camera;
}

void MultithreadedRenderer::setPerspective(float fieldOfView)	
{
	this->fieldOfView = fieldOfView;
	fieldOfViewFlag = true;
}

void MultithreadedRenderer::setClearColor(color24 color)
{
	clearColor = color;
	clearColorFlag = true;
}

void MultithreadedRenderer::setAA(bool enabled)
{
	aa = enabled;
	aaFlag = true;
}

color24 * MultithreadedRenderer::render()
{
	if(threads.size() == 0)
	{
		changeSettings();
		for(int i = 0; i < tiles.size(); i++)
			if(!tiles[i]->render(buffer1, switchRB, width))
				assert(!"Rendering error!");
		return buffer1;
	}
	else
	{
		WaitForSingleObject(work, INFINITE);

		bool firstFilled;

		EnterCriticalSection(&(mainSection));
		firstFilled = firstActive;
		firstActive = !firstActive;
		changeSettings();
		firstFreeTile = 0;
		workingThreads = threads.size();
		LeaveCriticalSection(&(mainSection));

		for(int i = 0; i < threads.size(); i++)
			threads[i]->setEvent();

		return firstFilled ? buffer1 : buffer2;
	}
}

void MultithreadedRenderer::createThreads(int threadNumber)
{
	for(int i = 0; i < threadNumber; i++)
		threads.push_back(new Thread(Thread::ThreadDelegate(
			this, &MultithreadedRenderer::threadMethod), i));
	for(int i = 0; i < threadNumber; i++)
		threads[i]->setEvent();
}

void MultithreadedRenderer::destroyThreads()
{
	for(int i = 0; i < threads.size(); i++)
		delete threads[i];
	threads.clear();
}

void MultithreadedRenderer::createTiles(vector2i number)
{
	int py = 0;
	for(int y = 0; y < number.y; y++)
	{
		int sy = height / number.y;
		if(y == 0)
			sy += height % number.y;
		int px = 0;
		for(int x = 0; x < number.x; x++)
		{
			int sx = width / number.x;
			if(x == 0)
				sx += width % number.x;
			tiles.push_back(new Tile(vector2i(px, py), vector2i(sx, sy)));
			px += sx;
		}
		py += sy;
	}
}

void MultithreadedRenderer::destroyTiles()
{
	for(int i = 0; i < tiles.size(); i++)
		delete tiles[i];
	tiles.clear();
}

void MultithreadedRenderer::changeSettings()
{
	if(cameraPtr != 0)
		for(int i = 0; i < tiles.size(); i++)
			tiles[i]->setCamera(*cameraPtr);

	if(sceneDataFlag)
	{
		for(int i = 0; i < tiles.size(); i++)
			tiles[i]->setSceneData(sceneData);
		sceneDataFlag = false;
	}
	if(fieldOfViewFlag)
	{
		float hs = tan(fieldOfView*PI/360.0f);
		float vs = (hs * height) / width;
		for(int i = 0; i < tiles.size(); i++)
		{
			vector2f position(
				((tiles[i]->getPosition().x / (float)width * 2.0f) - 1.0f) * hs, 
				((tiles[i]->getPosition().y / (float)height * 2.0f) - 1.0f) * vs);
			vector2f size(
				tiles[i]->getSize().x / (float)width * 2.0f * hs, 
				tiles[i]->getSize().y / (float)height * 2.0f * vs);
			tiles[i]->setFrameBuffer(position, size);
		}
		fieldOfViewFlag = false;
	}
	if(clearColorFlag)
	{
		for(int i = 0; i < tiles.size(); i++)
			tiles[i]->setClearColor(clearColor);
		clearColorFlag = false;
	}
	if(aaFlag)
	{
		for(int i = 0; i < tiles.size(); i++)
			tiles[i]->setAA(aa);
		aaFlag = false;
	}
}

void MultithreadedRenderer::threadMethod(int threadIndex)
{
	while(true)
	{
		threads[threadIndex]->wait();

		bool exit = false;
		while(!exit)
		{
			Tile * tile = 0;
			color24 * buffer = 0;

			EnterCriticalSection(&(mainSection));
			if(firstFreeTile < tiles.size())
			{
				tile = tiles[firstFreeTile];
				firstFreeTile++;
				buffer = firstActive ? buffer1 : buffer2;
			}
			else
			{
				workingThreads--;
				if(workingThreads == 0)
					SetEvent(work);
				exit = true;
			}
			LeaveCriticalSection(&(mainSection));

			if(buffer)
				if(!tile->render(buffer, switchRB, width))
					assert(!"Rendering error!");
		}
	}
}
