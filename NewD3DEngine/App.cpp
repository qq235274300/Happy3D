#include "App.h"
#include "Box.h"
#include <memory>

App::App()
	:wnd(800, 600, "Donkey Fart Box")
{
	std::mt19937 rng(std::random_device{}());
	std::uniform_real_distribution<float> adist(0.0f, 3.1415f * 2.0f);
	std::uniform_real_distribution<float> ddist(0.0f, 3.1415f * 2.0f);
	std::uniform_real_distribution<float> odist(0.0f, 3.1415f * 0.3f);
	std::uniform_real_distribution<float> rdist(6.0f, 20.0f);
	for (auto i = 0; i < 80; i++)
	{
		boxes.push_back(std::make_unique<Box>(
			wnd.Gfx(), rng, adist,
			ddist, odist, rdist
			));
	}
	wnd.Gfx().SetProjection(DirectX::XMMatrixPerspectiveLH(1.0f, 3.0f / 4.0f, 0.5f, 40.0f));
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

App::~App()
{
}

void App::DoFrame()
{
	//GameLogic
	//const float t = timer.Peek();
	//std::ostringstream oss;
	////std::setprecision(1) 浮点精度为1 3.1 std::fixed 固定输出精度 即使为0 
	//oss <<"Time elapsed"<< std::setprecision(1)<< std::fixed << t <<"s";
	//wnd.SetTitle(oss.str());

	//const float red = std::sin(timer.Peek()) /2.f + 0.5f;
	//wnd.Gfx().ClearBuffer(red, red, 1.0f);

	auto dt = timer.Mark();
	wnd.Gfx().ClearBuffer(0.07f, 0.0f, 0.12f);
	for (auto& b : boxes)
	{
		b->Update(dt);
		b->Draw(wnd.Gfx());
	}

	wnd.Gfx().EndFrame();
	
}
