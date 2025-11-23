#include "Timer.h"

Timer::Timer()
{
}

void Timer::Start()
{
	if (!m_bIsRunning)
	{
		m_StartPoint = steady_clock::now();
		m_bIsRunning = true;
		m_bIsPaused = false;
	}
}

void Timer::Restart()
{
	if (!m_bIsRunning)
		return;

	m_StartPoint = steady_clock::now();
}

void Timer::Stop()
{
	if (m_bIsRunning)
		m_bIsRunning = false;
}

void Timer::Pause()
{
	if (m_bIsRunning && !m_bIsPaused)
	{
		m_PausePoint = steady_clock::now();
		m_bIsPaused = true;
	}
}

void Timer::Resume()
{
	if (m_bIsRunning && m_bIsPaused)
	{
		m_StartPoint += duration_cast<milliseconds>(steady_clock::now() - m_PausePoint);
	}
}

const int64_t Timer::ElapsedMS() const
{
	if (m_bIsRunning)
	{
		if (m_bIsPaused)
			return duration_cast<milliseconds>(m_PausePoint - m_StartPoint).count();
		else
			return duration_cast<milliseconds>(steady_clock::now() - m_StartPoint).count();
	}

	return 0;
}

const int64_t Timer::ElapsedSec() const
{
	return ElapsedMS() / 1000;
}
