#include "pch.h"
#include "StepTimer.h"

Timer::Timer() :
	isPaused(false), m_deltaTime(-1.0), m_secPerCount(0.0), m_prevTime(0),
	m_currTime(0), m_startTime(0), m_pauseTime(0), m_stopTime(0)
{
	__int64 countsPerSec;
	QueryPerformanceFrequency((LARGE_INTEGER*)&countsPerSec);
	m_secPerCount = 1.0 / static_cast<double>(countsPerSec);
}

float Timer::GetDeltaTime()const
{
	return static_cast<float>(m_deltaTime);
}

float Timer::GetTotalTime()const
{
	if (isPaused)
		return static_cast<float>(((m_stopTime - m_pauseTime) - m_startTime)*m_secPerCount);
	else
		return static_cast<float>(((m_currTime - m_pauseTime) - m_startTime)*m_secPerCount);
}

void Timer::Tick()
{
	if (isPaused)
	{
		m_deltaTime = 0.0;
		return;
	}

	QueryPerformanceCounter((LARGE_INTEGER*)&m_currTime);
	m_deltaTime = (m_currTime - m_prevTime) * m_secPerCount;
	m_prevTime = m_currTime;

	if (m_deltaTime < 0.0)
		m_deltaTime = 0.0;
}

void Timer::Start()
{
	if (isPaused)
	{
		QueryPerformanceCounter((LARGE_INTEGER*)&m_currTime);

		m_pauseTime += (m_stopTime - m_startTime);
		m_prevTime = m_startTime;

		m_stopTime = 0;
		isPaused = false;
	}
}

void Timer::Stop()
{
	if (!isPaused)
	{
		QueryPerformanceCounter((LARGE_INTEGER*)&m_currTime);

		m_stopTime = m_currTime;
		isPaused = true;
	}
}

void Timer::Reset()
{
	QueryPerformanceCounter((LARGE_INTEGER*)&m_currTime);

	m_prevTime = m_currTime;
	m_startTime = m_currTime;
	m_stopTime = 0;
	isPaused = false;
}