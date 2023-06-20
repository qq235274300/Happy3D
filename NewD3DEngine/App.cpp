#include "App.h"

App::App()
	:wnd(800, 300, "Donkey Fart Box")
{
	
}

int App::Go()
{
	MSG msg;
	BOOL gResult;
	int WheelNum = 0;
	while ((gResult = GetMessage(&msg, nullptr, 0, 0)) > 0)
	{
		// TranslateMessage will post auxilliary WM_CHAR messages from key msgs
		TranslateMessage(&msg);
		DispatchMessage(&msg);

		DoFrame();
	}

	if (gResult == -1)
	{
		throw CHWND_LAST_EXCEPT();
	}

	// wParam here is the value passed to PostQuitMessage
	return msg.wParam;

}

void App::DoFrame()
{
	//GameLogic
	const float t = timer.Peek();
	std::ostringstream oss;
	//std::setprecision(1) ���㾫��Ϊ1 3.1 std::fixed �̶�������� ��ʹΪ0 
	oss <<"Time elapsed"<< std::setprecision(1)<< std::fixed << t <<"s";
	wnd.SetTitle(oss.str());
}
