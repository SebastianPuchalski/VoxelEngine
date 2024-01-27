#include "Window.hpp"

#include <windows.h>
#include <GL/gl.h>

#define GL_BGR 0x80E0

#pragma comment(lib, "opengl32.lib")

using namespace Window;

HDC hDC = NULL;
HGLRC hRC = NULL;
HWND hWnd = NULL;
HINSTANCE hInstance = NULL;

Settings settings;
GLuint texture;

onMouseMoveDelegate mouseMoveDelegate;
onKeyDelegate keyDelegate;

LRESULT CALLBACK wndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

void oglInit()
{
	glViewport(0, 0, (GLsizei)(settings.width * settings.multiplier), 
		(GLsizei)(settings.height * settings.multiplier));

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-1, 1, -1, 1, 1, -1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glClearColor(0, 0, 0, 1);

	glEnable(GL_TEXTURE_2D);
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	if(settings.multiplier == 1)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	}
	else
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	}
	glTexImage2D(GL_TEXTURE_2D, 0, 3, settings.width, settings.height,
		0, GL_BGR, GL_UNSIGNED_BYTE, 0);

	glColor4f(1, 1, 1, 1);
}

bool initMouse()
{
	RAWINPUTDEVICE rid;

	rid.usUsagePage = 1;
	rid.usUsage = 2;
	rid.dwFlags = 0;
	rid.hwndTarget = hWnd;

	if(!RegisterRawInputDevices(&rid, 1, sizeof(RAWINPUTDEVICE)))
		return false;
	return true;
}

bool Window::create(const Settings & s)
{
	destroy();

	settings = s;
	hInstance = GetModuleHandle(NULL);
	
	DWORD dwExStyle;
	DWORD dwStyle;
	RECT rect;
	rect.left=(long)0;
	rect.right=(long)(settings.width * settings.multiplier);
	rect.top=(long)0;
	rect.bottom=(long)(settings.height * settings.multiplier);
  
	WNDCLASSA wc;
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc = wndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = NULL;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = NULL;
	wc.lpszMenuName = NULL;
	wc.lpszClassName = "Window";
	
	if(RegisterClassA(&wc) == NULL)
	{
		destroy();
		return false;
	}
	
	if(settings.fullscreen == true)
	{
		DEVMODE dmScreenSettings;
		memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));
		dmScreenSettings.dmSize = sizeof(dmScreenSettings);
		dmScreenSettings.dmPelsWidth = settings.width * settings.multiplier;
		dmScreenSettings.dmPelsHeight = settings.height * settings.multiplier;
		dmScreenSettings.dmBitsPerPel = 32;
		dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

		if(ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL)
			settings.fullscreen = false;
	}
	
	if(settings.fullscreen == true)
	{
		dwExStyle = WS_EX_APPWINDOW;
		dwStyle = WS_POPUP;
	}
	else
	{
		dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
		dwStyle = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_THICKFRAME;
		//dwStyle = WS_OVERLAPPEDWINDOW;
	}
	
	AdjustWindowRectEx(&rect, dwStyle, FALSE, dwExStyle);

	hWnd = CreateWindowEx(
		dwExStyle,
		"Window",
		s.title.c_str(),
		dwStyle |
		WS_CLIPSIBLINGS |
		WS_CLIPCHILDREN,
		0, 0,
		rect.right-rect.left,
		rect.bottom-rect.top,
		NULL,
		NULL,
		hInstance,
		NULL
		);

	if(hWnd == NULL)
	{
		destroy();
		return false;
	}

	static const PIXELFORMATDESCRIPTOR pfd =
	{
		sizeof(PIXELFORMATDESCRIPTOR),
		1,
		PFD_DRAW_TO_WINDOW |
		PFD_SUPPORT_OPENGL |
		PFD_DOUBLEBUFFER,
		PFD_TYPE_RGBA,
		32,
		0, 0, 0, 0, 0, 0,
		0,
		0,
		0,
		0, 0, 0, 0,
		0, //depth
		0,
		0,
		PFD_MAIN_PLANE,
		0,
		0, 0, 0
	};
	
	if((hDC = GetDC(hWnd)) == NULL)
	{
		destroy();
		return false;
	}
  
	int PixelFormat;
	PixelFormat = ChoosePixelFormat(hDC, &pfd);
	
	if(PixelFormat == 0 || SetPixelFormat(hDC, PixelFormat, &pfd) == 0)
	{
		destroy();
		return false;
	}
	
	hRC = wglCreateContext(hDC);
  
	if(hRC == NULL || wglMakeCurrent(hDC, hRC) == 0)
	{
		destroy();
		return false;
	}
  
	ShowWindow(hWnd, SW_SHOW);
	SetForegroundWindow(hWnd);
	SetFocus(hWnd);

	oglInit();
	return initMouse();
}

void Window::destroy()
{
	if(settings.fullscreen == true)
		ChangeDisplaySettings(NULL, 0);
	
	if(hRC != NULL)
	{
		wglMakeCurrent(NULL, NULL);
		wglDeleteContext(hRC);
		hRC = NULL;
	}
  
	if(hDC != NULL)
	{
		ReleaseDC(hWnd, hDC);
		hDC = NULL;
	}
	
	if(hWnd != NULL)
	{
		DestroyWindow(hWnd);
		hWnd = NULL;
	}
	
	WNDCLASSA wc;
	if(GetClassInfoA(hInstance, "Window", &wc) != 0)
		UnregisterClassA("Window", hInstance);
	
	hInstance = NULL;
}

bool Window::draw(color * data)
{
	if(hInstance == 0)
		return false;

	//glClear(GL_COLOR_BUFFER_BIT);

	//glShadeModel(GL_FLAT);
	//glBindTexture(GL_TEXTURE_2D, texture);
	//glEnable(GL_TEXTURE_2D);
	//glDisable(GL_TEXTURE_2D);

	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, settings.width, settings.height,
		GL_BGR, GL_UNSIGNED_BYTE, (GLvoid *)data);

	glBegin(GL_QUADS);
		glTexCoord2f(0, 0);
		glVertex2f(-1, -1);
		glTexCoord2f(0, 1);
		glVertex2f(-1, 1);
		glTexCoord2f(1, 1);
		glVertex2f(1, 1);
		glTexCoord2f(1, 0);
		glVertex2f(1, -1);
	glEnd();

	SwapBuffers(hDC);

	MSG msg;
	while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) != 0)
	{
		if(msg.message == WM_QUIT)
			return false;
		//TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return true;
}

void Window::setOnMouseMove(onMouseMoveDelegate delegate)
{
	mouseMoveDelegate = delegate;
}

void Window::setOnKey(onKeyDelegate delegate)
{
	keyDelegate = delegate;
}

LRESULT CALLBACK wndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
	case WM_CLOSE:
		PostQuitMessage(0);
		break;

	case WM_INPUT:
		{
			RAWINPUT ri;
			UINT size = sizeof(RAWINPUT);
			if(GetRawInputData((HRAWINPUT)lParam, RID_INPUT, &ri, &size, sizeof(RAWINPUTHEADER)) == -1)
			{
				//blad pobierania danych
			}
			if(ri.header.dwType == RIM_TYPEMOUSE)
				mouseMoveDelegate(ri.data.mouse.lLastX, ri.data.mouse.lLastY);
		}
		break;

	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
		keyDelegate((int)wParam, true);
		break;

	case WM_KEYUP:
	case WM_SYSKEYUP:
		keyDelegate((int)wParam, false);
		break;
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}
