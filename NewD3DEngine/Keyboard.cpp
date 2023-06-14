#include "Keyboard.h"

bool Keyboard::IsKeyPressed(unsigned char keycode) const noexcept
{
	return keyStates[keycode];
}

Keyboard::Event Keyboard::ReadKey() noexcept
{
	if (keyBuffer.size() > 0u)
	{
		Event e = keyBuffer.front();
		keyBuffer.pop();
		return e;
	}
	else
	{
		return Keyboard::Event();
	}
}
