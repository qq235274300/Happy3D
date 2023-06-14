#pragma once
#include <bitset>
#include <queue>



class Keyboard
{
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

private:
	//
	static constexpr unsigned int nKeys = 256u;
	std::bitset<nKeys> keyStates;
	std::queue<Event> keyBuffer;
	std::queue<char> charBuffer;
	
};

