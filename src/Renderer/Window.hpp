#ifndef WINDOW_HPP
#define WINDOW_HPP

#include <string>
#include <FastDelegate.h>

namespace Window
{
	struct color
	{
		unsigned char b, g, r;
	};

	struct Settings
	{
		Settings() : width(800), height(600), fullscreen(false), title("Window"), multiplier(1){}
		Settings(int w, int h, bool fs, const std::string & t, int multiplier = 1):
			width(w), height(h), fullscreen(fs), title(t), multiplier(multiplier){}
		~Settings(){}

		int width;
		int height;
		bool fullscreen;
		std::string title;
		int multiplier;
	};

	bool create(const Settings & s);
	void destroy();

	typedef fastdelegate::FastDelegate2<int, int, void> onMouseMoveDelegate;
	typedef fastdelegate::FastDelegate2<int, bool, void> onKeyDelegate;
	void setOnMouseMove(onMouseMoveDelegate delegate);
	void setOnKey(onKeyDelegate delegate);

	bool draw(color * data);
}

#endif //WINDOW_HPP
