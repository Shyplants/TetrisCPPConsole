#include "Keyboard.h"
#include "../utils/Logger.h"

Keyboard::Keyboard()
	: m_Keys{}
{
}

void Keyboard::Update()
{
	for (int i = 0; i < KEY_LAST; ++i)
	{
		m_Keys[i].m_bIsJustPressed = false;
		m_Keys[i].m_bIsJustReleased = false;
	}
}

void Keyboard::OnKeyDown(int key)
{
	if (key >= KEY_LAST)
	{
		TETRIS_ERROR("[" + std::to_string(key) + "] - Is not defined!");
		return;
	}

	m_Keys[key].Update(true);
}

void Keyboard::OnKeyUp(int key)
{
	if (key >= KEY_LAST)
	{
		TETRIS_ERROR("[" + std::to_string(key) + "] - Is not defined!");
		return;
	}

	m_Keys[key].Update(false);
}

bool Keyboard::IsKeyHeld(int key) const
{
	if (key >= KEY_LAST)
	{
		TETRIS_ERROR("[" + std::to_string(key) + "] - Is not defined!");
		return false;
	}

	return m_Keys[key].m_bIsDown;
}

bool Keyboard::IsKeyJustPressed(int key) const
{
	if (key >= KEY_LAST)
	{
		TETRIS_ERROR("[" + std::to_string(key) + "] - Is not defined!");
		return false;
	}

	return m_Keys[key].m_bIsJustPressed;
}

bool Keyboard::IsKeyJustReleased(int key) const
{
	if (key >= KEY_LAST)
	{
		TETRIS_ERROR("[" + std::to_string(key) + "] - Is not defined!");
		return false;
	}

	return m_Keys[key].m_bIsJustReleased;
}
