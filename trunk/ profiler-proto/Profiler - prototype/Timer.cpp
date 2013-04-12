#include "Timer.h"

typedef unsigned __int32 uint32_t;

uint64_t getTimeSinceMachineStartup()
{
	uint64_t ret;
	::QueryPerformanceCounter((LARGE_INTEGER*)(&ret));
	return ret;
}

uint64_t getQueryPerformanceFrequency()
{
	LARGE_INTEGER ret;
	::QueryPerformanceFrequency(&ret);
	return uint64_t(ret.QuadPart);
}

static uint64_t cTickPerSecond = getQueryPerformanceFrequency();
static double cInvTickPerSecond = 1.0 / cTickPerSecond;

void TimeInterval::set(double sec)
{
	mTicks = uint64_t(sec*cTickPerSecond);
}

double TimeInterval::asSecond() const
{
	return mTicks * cInvTickPerSecond;
}

TimeInterval TimeInterval::getMax() {
	return TimeInterval(uint64_t(ULLONG_MAX));
}

Timer::Timer()
{
	reset();
}

TimeInterval Timer::get() const
{
	return TimeInterval(getTimeSinceMachineStartup() - mStartTime.asTicks());
}

TimeInterval Timer::reset()
{
	TimeInterval backup = get();
	mStartTime = TimeInterval(getTimeSinceMachineStartup());
	return backup;
}

DeltaTimer::DeltaTimer() {
	mLastTime = mTimer.get();
}

DeltaTimer::DeltaTimer(const TimeInterval& firstDelta) {
	mLastTime = mTimer.get() - firstDelta;
}

TimeInterval DeltaTimer::getDelta() const
{
	TimeInterval currentTime = mTimer.get();
	// We knows the minus operation won't produce negative result, so
	// use the ticks directly as an optimization
	uint64_t dt = currentTime.asTicks() - mLastTime.asTicks();
	mLastTime = currentTime;
	return TimeInterval(dt);
}