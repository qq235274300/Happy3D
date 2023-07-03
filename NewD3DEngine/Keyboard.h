#pragma once

#include <bitset>
#include <queue>
#include <optional>


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
		};

	private:
		Type type;
		unsigned char keycode;
		
	public:
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

	
		unsigned char GetKeyCode() const noexcept
		{
			return keycode;
		}
	};

public:
	Keyboard() = default;
	
	Keyboard(const Keyboard&) = delete;
	Keyboard& operator=(const Keyboard&) = delete;

	//目前有的API 能力，查询是否按键，消息在Window里面绑定
	//EVENT
	bool IsKeyPressed(unsigned char keycode)const noexcept;
	std::optional<Event> ReadKey()noexcept;
	bool KeyIsEmpty()const noexcept;
	void FlushKey()noexcept;

	//Char Text Input
	std::optional<char> ReadChar() noexcept;
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
	//keyStates 由于Windows KeyMessages 在一个Byte内表示 0-255 ，所以可以用位运算bitset 表示，true表示这个keycode 按下
	std::bitset<nKeys> keyStates;
	std::queue<Event> keyBuffer;
	std::queue<char> charBuffer;

	//
	bool autorepeatEnable = false;
	
};

