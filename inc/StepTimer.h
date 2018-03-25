#pragma once
#ifndef STEPTIMER_H
#define STEPTIMER_H

class Timer
{
public:

	Timer();

	float GetDeltaTime()const;
	float GetTotalTime()const;

	void Tick();
	void Start();
	void Stop();
	void Reset();

private:

	bool isPaused;
	double m_deltaTime;
	double m_secPerCount;

	__int64 m_prevTime;
	__int64 m_currTime;
	__int64 m_startTime;
	__int64 m_pauseTime;
	__int64 m_stopTime;

};

#endif // !STEPTIMER_H
