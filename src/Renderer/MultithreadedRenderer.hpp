/*
Module: MultithreadedRenderer
Author: Sebastian Puchalski
Date: 01.12.2013
*/

#ifndef MULTITHREADED_RENDERER_HPP_
#define MULTITHREADED_RENDERER_HPP_

#include "Renderer.hpp"
#include "../vector2.hpp"

#include <vector>
#include <FastDelegate.h>
#include <windows.h>

class MultithreadedRenderer
{
public:
	MultithreadedRenderer(int width, int height, bool switchRB, 
		vector2i tileNumber, int threadNumber = 0);
	virtual ~MultithreadedRenderer();

	void setSceneData(uint8 * data);
	void setCamera(Camera * camera);
	void setPerspective(float fieldOfView = 90);
	void setClearColor(color24 color);
	void setAA(bool enabled);

	color24 * render();

private:
	int width, height;
	color24 * buffer1;
	color24 * buffer2;
	bool switchRB;

	class Thread
	{
	public:
		typedef fastdelegate::FastDelegate1<int, void> ThreadDelegate;

		Thread(ThreadDelegate delegate, int value) : delegate(delegate), value(value)
		{
			threadHandle = CreateThread(
				0, 0, &(Thread::threadProc), this, 0, &id);
			eventHandle = CreateEvent(NULL, FALSE, FALSE, NULL);
		}
		~Thread()
		{
			TerminateThread(threadHandle, 0);
		}

		void wait(uint milliseconds = INFINITE) //should be called from the thread
		{
			WaitForSingleObject(eventHandle, milliseconds);
		}
		void setEvent() //should be called from another thread
		{
			SetEvent(eventHandle);
		}

	private:
		HANDLE threadHandle;
		DWORD id;
		HANDLE eventHandle;

		ThreadDelegate delegate;
		int value;

		static DWORD WINAPI threadProc(void * arg)
		{
			Thread * thread = (Thread *)arg;
			thread->wait();
			thread->delegate(thread->value);
			return 0;
		}
	};
	std::vector<Thread *> threads;

	class Tile
	{
	public:
		Tile(vector2i position, vector2i size) : position(position), size(size)
		{
			InitializeCriticalSection(&tileSection);
			renderer = new Renderer(size.x, size.y);
			renderer->setCamera(&camera);
		}
		~Tile()
		{
			DeleteCriticalSection(&tileSection);
			delete renderer;
		}

		vector2i getPosition() const { return position; }
		vector2i getSize() const { return size; }

		void setSceneData(uint8 * data)
		{
			EnterCriticalSection(&tileSection);
			renderer->setSceneData(data);
			LeaveCriticalSection(&tileSection);
		}
		void setCamera(Camera camera)
		{
			EnterCriticalSection(&tileSection);
			this->camera = camera;
			LeaveCriticalSection(&tileSection);
		}
		void setFrameBuffer(vector2f position, vector2f size)
		{
			EnterCriticalSection(&tileSection);
			renderer->setFrameBuffer(position, size);
			LeaveCriticalSection(&tileSection);
		}
		void setClearColor(color24 color)
		{
			EnterCriticalSection(&tileSection);
			renderer->setClearColor(color);
			LeaveCriticalSection(&tileSection);
		}
		void setAA(bool enabled)
		{
			EnterCriticalSection(&tileSection);
			renderer->setAA(enabled);
			LeaveCriticalSection(&tileSection);
		}

		bool render(color24 * colorBuffer, bool switchRB, int width)
		{
			EnterCriticalSection(&tileSection);
			colorBuffer += position.y * width + position.x;
			int spaceNumber = width - size.x;
			bool result = renderer->render(colorBuffer, switchRB, spaceNumber);
			LeaveCriticalSection(&tileSection);
			return result;
		}

	private:
		const vector2i position;
		const vector2i size;
		Renderer * renderer;
		Camera camera;
		CRITICAL_SECTION tileSection;
	};
	std::vector<Tile *> tiles;

	uint8 * sceneData;
	Camera * cameraPtr;
	float fieldOfView;
	color24 clearColor;
	bool aa;
	bool sceneDataFlag, fieldOfViewFlag, clearColorFlag, aaFlag;

	bool firstActive;
	int firstFreeTile;
	int workingThreads;
	HANDLE work;
	CRITICAL_SECTION mainSection;

	void createThreads(int threadNumber);
	void destroyThreads();
	void createTiles(vector2i number);
	void destroyTiles();

	void changeSettings();

	void threadMethod(int threadIndex);
};

#endif //MULTITHREADED_RENDERER_HPP_
