#include "Window.h"
#include <sstream>

int CALLBACK WinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR     lpCmdLine,
	int       nCmdShow)
{
	try
	{
		Window wnd(800, 300, "Donkey Fart Box");

		MSG msg;
		BOOL gResult;
		int WheelNum = 0;
		while ((gResult = GetMessage(&msg, nullptr, 0, 0)) > 0)
		{
			// TranslateMessage will post auxilliary WM_CHAR messages from key msgs
			TranslateMessage(&msg);
			DispatchMessage(&msg);

			//Test Knd
			if (wnd.kbd.IsKeyPressed(VK_MENU))
			{
				MessageBox(nullptr, "Something Happon", "Sapce Key is Pressed", MB_OK | MB_ICONEXCLAMATION);
			}
			//Test Mouse
			while (!wnd.mouse.IsEmpty())
			{
				Mouse::Event e = wnd.mouse.Read();
				/*if (e.GetType() == Mouse::Event::Type::Move)
				{
					std::ostringstream oss;
					oss << "MousePos:(" << e.GetPosX() << "," << e.GetPosY()<<")";
					wnd.SetTitle(oss.str());
				}
				if (e.GetType() == Mouse::Event::Type::Leave)
				{
					std::ostringstream oss;
					oss << "MouseLeave:(";
					wnd.SetTitle(oss.str());
				}*/
				if (e.GetType() == Mouse::Event::Type::WheelUp)
				{
					WheelNum++;
				}
				if (e.GetType() == Mouse::Event::Type::WheelDown)
				{
					WheelNum--;
				}

				wnd.SetTitle(std::to_string(WheelNum));
			}
		}

		if (gResult == -1)
		{
			return -1;
		}

		// wParam here is the value passed to PostQuitMessage
		return msg.wParam;
	}

	catch (const ChiliException& e)
	{
	
		MessageBox(nullptr, e.what(), e.GetType(), MB_OK | MB_ICONEXCLAMATION);
	}

	// check if GetMessage call itself borked

	catch (const std::exception& e)
	{
		return -1;
		MessageBox(nullptr, e.what(), "Standard Exception", MB_OK | MB_ICONEXCLAMATION);
	}

	// wParam here is the value passed to PostQuitMessage
	
	catch (...)
	{
		MessageBox(nullptr, "No details available", "Unknown Exception", MB_OK | MB_ICONEXCLAMATION);
	}
	return -1;
}