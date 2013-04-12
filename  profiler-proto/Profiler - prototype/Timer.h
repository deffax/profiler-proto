#pragma once
#include <Windows.h>

typedef unsigned __int64 uint64_t;

class TimeInterval
{
public:
	TimeInterval() : mTicks(0){}
	explicit TimeInterval(uint64_t ticks)
	{
		set(ticks);
	}
	TimeInterval(double second)
	{
		set(second);
	}

	void set(const uint64_t ticks)
	{
		mTicks = ticks;
	}
	void set(double sec);
	uint64_t asTicks() const
	{
		return mTicks;
	}
	double asSecond() const;
	static TimeInterval getMax();

	bool operator==(const TimeInterval& rhs) const
	{
		return (mTicks == rhs.mTicks);
	}

	bool operator<(const TimeInterval& rhs) const
	{
		return (mTicks < rhs.mTicks);
	}

	TimeInterval& operator+=(const TimeInterval& rhs)
	{
		mTicks += rhs.mTicks;
		return *this;
	}
	TimeInterval operator+(const TimeInterval& rhs) const
	{
		
		return (TimeInterval(*this) += rhs);
	}
	TimeInterval& operator-=(const TimeInterval& rhs)
	{
		mTicks = rhs.mTicks > mTicks? 0 : mTicks - rhs.mTicks;
		return *this;
	}
	TimeInterval operator-(const TimeInterval& rhs) const
	{
		return (TimeInterval(*this) -= rhs);
	}
	friend TimeInterval operator*(double lhs, const TimeInterval& rhs)
	{
		TimeInterval ret;
		ret.mTicks = uint64_t(rhs.mTicks * lhs);
		return ret;
	}
	friend TimeInterval operator*(const TimeInterval& lhs, double rhs)
	{
		TimeInterval ret;
		ret.mTicks = uint64_t(lhs.mTicks * rhs);
		return ret;
	}


private:
	friend uint64_t getTimeSinceMachineStartup();
	uint64_t mTicks;
};

//TIMER
class Timer
{
public:
	Timer();
	TimeInterval get() const;
	TimeInterval reset();
protected:
	mutable TimeInterval mLastGetTime;
	TimeInterval mStartTime;
};

//DELTA TIMER
class DeltaTimer
{
public:
	DeltaTimer();
	explicit DeltaTimer(const TimeInterval& firstDelta);

	TimeInterval getDelta() const;

protected:
	Timer mTimer;
	mutable TimeInterval mLastTime;
};
