#pragma once

#include "Window.h"
#include "ChiliTimer.h"

class App
{
public:
	App();
public:
	//Start Game Loop��When App Start ��ʱ����ã�  //Message Loop 
	int Go();
	~App();


private:
	//Actually Game Loop
	void DoFrame();
	
private:
	Window wnd;
	ChiliTimer timer;

	std::vector<std::unique_ptr<class Box>> boxes;
};

