#pragma once

#include "Window.h"

#include <bitset>
#include <queue>



class Keyboard
{

	friend class Window;

public:
	class Event
	{
	public:
		enum class Type
		{
			Press,
			Release,
			Invalid
		};

	private:
		Type type;
		unsigned char keycode;
		
	public:
		Event()noexcept
			:type(Type::Invalid), keycode(0u)
		{
			
		}

		Event(Type inType) noexcept
			:type(inType), keycode(0u)
		{
			
		}

		Event(Type inType,unsigned char inKeycode)noexcept
			:type(inType), keycode(inKeycode)
		{

		}

		bool isPressed()const noexcept
		{
			return type == Type::Press;
		}

		bool isRelease()const noexcept
		{
			return type == Type::Release;
		}

		bool isValide()const noexcept
		{
			return type != Type::Invalid;
		}

		unsigned char GetKeyCode() const noexcept
		{
			return keycode;
		}
	};

public:
	Keyboard() = default;
	Keyboard(const Keyboard&) = delete;
	Keyboard& operator=(const Keyboard&) = delete;

	//EVENT
	bool IsKeyPressed(unsigned char keycode)const noexcept;
	Event ReadKey()noexcept;
	bool KeyIsEmpty()const noexcept;
	void FlushKey()noexcept;

	//Char Text Input
	char ReadChar() noexcept;
	bool CharIsEmpty()const noexcept;
	void FlushChar()noexcept;
	
	void Flush()noexcept;
	
	//autoRepeat control
	void EnableAutorepeat()noexcept;
	void DisableAutorepeat()noexcept;
	bool AutorepeatIsEnabled()const noexcept;

private:
	//This Functions only use in WinAPI Side
	void OnKeyPressed(unsigned char keycode)noexcept;
	void OnKeyReleased(unsigned char keycode)noexcept;
	void OnChar(char character)noexcept;
	void ClearState()noexcept;
	template<typename T>
	static void TrimBuffer(std::queue<T>& buffer) noexcept;

private:
	//
	static constexpr unsigned int nKeys = 256u;
	static constexpr unsigned int nBufferSize = 16u;
	//keyStates ����Windows KeyMessages ��һ��Byte�ڱ�ʾ 0-255 �����Կ�����λ����bitset ��ʾ��true��ʾ���keycode ����
	std::bitset<nKeys> keyStates;
	std::queue<Event> keyBuffer;
	std::queue<char> charBuffer;

	//
	bool autorepeatEnable = false;
	
};

