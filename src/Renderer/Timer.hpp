/*
Module: Timer
Author: Sebastian Puchalski
Date: 21.11.2012
*/

#ifndef TIMER_HPP_
#define TIMER_HPP_

#include "../common.hpp"

class Timer
{
public:
	Timer();
	virtual ~Timer();

	void update();
	
	double getFrameTime() const { return frameTime; }
	double getFPS() const { return 1/frameTime; }
	double getTotalTime() const { return totalTime; }

private:
	double frameTime;
	double totalTime;

	int64 frequency;
	int64 lastState;
};

#endif //TIMER_IMPL_HPP_
