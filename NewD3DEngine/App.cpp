#include "App.h"

App::App()
	:wnd(800, 300, "Donkey Fart Box")
{
	
}

int App::Go()
{
	while (true)
	{
		if (const auto ecode = Window::ProcessMessages())
		{
			return *ecode;
		}

		DoFrame();
		
	}

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
