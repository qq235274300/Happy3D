#include "App.h"

App::App()
	:wnd(800, 600, "Donkey Fart Box")
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
	//std::setprecision(1) 浮点精度为1 3.1 std::fixed 固定输出精度 即使为0 
	oss <<"Time elapsed"<< std::setprecision(1)<< std::fixed << t <<"s";
	wnd.SetTitle(oss.str());

	const float red = std::sin(timer.Peek()) /2.f + 0.5f;
	wnd.Gfx().ClearBuffer(red, red, 1.0f);

	wnd.Gfx().DrawSomeShit(timer.Peek());

	wnd.Gfx().EndFrame();
	
}
