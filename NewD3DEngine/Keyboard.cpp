#include "Keyboard.h"
#include <optional>

bool Keyboard::IsKeyPressed(unsigned char keycode) const noexcept
{
	return keyStates[keycode];
}

Keyboard::Event Keyboard::ReadKey() noexcept
{
	if (keyBuffer.size() > 0u)
	{
		Keyboard::Event e = keyBuffer.front();
		keyBuffer.pop();
		return e;
	}
	else
	{
		return Keyboard::Event();
		//return std::optional<Keyboard::Event>();
	}
}

bool Keyboard::KeyIsEmpty() const noexcept
{
	return keyBuffer.empty();
}

void Keyboard::FlushKey() noexcept
{
	keyBuffer = std::queue<Keyboard::Event>();
}

char Keyboard::ReadChar() noexcept
{
	if (CharIsEmpty())
	{
		return 0;
	}
	else
	{
		unsigned char target = charBuffer.front();
		charBuffer.pop();
		return target;
	}
}

bool Keyboard::CharIsEmpty() const noexcept
{
	return charBuffer.empty();
}

void Keyboard::FlushChar() noexcept
{
	charBuffer = std::queue<char>();
}

void Keyboard::Flush() noexcept
{
	keyBuffer = std::queue<Keyboard::Event>();
	charBuffer = std::queue<char>();
}

void Keyboard::EnableAutorepeat() noexcept
{
	autorepeatEnable = true;
}

void Keyboard::DisableAutorepeat() noexcept
{
	autorepeatEnable = false;
}

bool Keyboard::AutorepeatIsEnabled() const noexcept
{
	return autorepeatEnable;
}

void Keyboard::OnKeyPressed(unsigned char keycode) noexcept
{
	keyStates[keycode] = true;
	keyBuffer.push(Keyboard::Event(Keyboard::Event::Type::Press, keycode));
	TrimBuffer(keyBuffer);
}

void Keyboard::OnKeyReleased(unsigned char keycode) noexcept
{
	keyStates[keycode] = false;
	keyBuffer.push(Keyboard::Event(Keyboard::Event::Type::Release, keycode));
	TrimBuffer(keyBuffer);
}

void Keyboard::OnChar(char character) noexcept
{
	charBuffer.push(character);
	TrimBuffer(charBuffer);
}

void Keyboard::ClearState() noexcept
{
	keyStates.reset();
}

template<typename T>
void Keyboard::TrimBuffer(std::queue<T>& buffer) noexcept
{
	while (buffer.size() > nBufferSize)
	{
		buffer.pop();
	}
}

