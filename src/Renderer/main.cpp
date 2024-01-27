#include "Demo.hpp"
#include "Timer.hpp"

#include <iostream>

using namespace std;

const int timeSmoothing = 100;

int main(int argc, char * argv[])
{
	if(argc != 2)
	{
		cout << "Put the scene file (*.scn) as argument!\n";
		return 0;
	}

	Demo demo;
	if(!demo.initialize(argv[1]))
	{
		cout << "Error during initialization!\n";
		return 0;
	}

	Timer timer;
	timer.update();

	bool run = true;
	while(run)
	{
		for(int i = 0; i < timeSmoothing && run; i++)
			run = demo.update();
		timer.update();
		cout << "FPS: " << timer.getFPS() * timeSmoothing 
			<< " DT: " << timer.getFrameTime() / timeSmoothing << endl;
	}

	demo.finalize();

	return 0;
}
