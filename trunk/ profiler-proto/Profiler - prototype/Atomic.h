#pragma once
#include "Mutex.h"

template<typename T, typename TArg=T>
class AtomicValue
{
public:
	AtomicValue() : mVal() {}

	explicit AtomicValue(TArg val) : mVal(val) {}

	AtomicValue(const AtomicValue& rhs) {
		set(rhs.get());
	}

	AtomicValue& operator=(const AtomicValue& rhs)
	{
		set(rhs.get());
		return *this;
	}

	AtomicValue& operator=(TArg val)
	{
		set(val);
		return *this;
	}

	operator T() const {
		return get();
	}

	T get() const
	{
		// Rather than simply "return mVal;", we use a more explicit protection
		T ret;
		{	ScopeLock lock(mMutex);
			ret = mVal;
		}
		return ret;
	}

	void set(TArg val)
	{
		ScopeLock lock(mMutex);
		mVal = val;
	}

protected:
	T mVal;
	mutable Mutex mMutex;
};	// AtomicValue


class AtomicInteger
{
public:
	AtomicInteger(int i=0) { value=i; }

	//! Atomically pre-increment
	inline int operator++();

	//! Atomically post-increment
	int operator++(int) {
		return ++(*this) - 1;
	}

	//! Atomically pre-decrement
	inline int operator--();

	//! Atomically post-decrement
	int operator--(int) {
		return --(*this) + 1;
	}

	operator int() const {
		return value;
	}
private:
	volatile int value;
};	// AtomicInteger



// Intrinsic functions
extern "C" {
	long __cdecl _InterlockedIncrement(long volatile* pn);
	long __cdecl _InterlockedDecrement(long volatile* pn);
};

#pragma intrinsic(_InterlockedIncrement)
#pragma intrinsic(_InterlockedDecrement)

inline int AtomicInteger::operator++() {
	return _InterlockedIncrement((long*)&value);
}

inline int AtomicInteger::operator--() {
	return _InterlockedDecrement((long*)&value);
}