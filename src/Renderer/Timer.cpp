#include "Timer.hpp"

#include <windows.h>

Timer::Timer() : frameTime(0), totalTime(0)
{
	LARGE_INTEGER ticksPerSecond, lastCounterState;
	if(QueryPerformanceFrequency(&ticksPerSecond))
	{
		frequency = ticksPerSecond.QuadPart;
	}
	else
	{
		//Blad!
	}
	QueryPerformanceCounter(&lastCounterState);
	lastState = lastCounterState.QuadPart;
}

Timer::~Timer()
{
}

void Timer::update()
{
	LARGE_INTEGER currentCounterState;
	QueryPerformanceCounter(&currentCounterState);
	frameTime = (currentCounterState.QuadPart - lastState)
		/ static_cast<double>(frequency);
	lastState = currentCounterState.QuadPart;
	totalTime += frameTime;
}
