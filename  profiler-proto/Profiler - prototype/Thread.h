#pragma once
#include "NonCopyable.h"
#include "Atomic.h"
#include <stdexcept>

typedef unsigned int useconds_t;

class Thread : NonCopyable
{
public:
	class IRunnable //abstract
	{
	public:
		virtual ~IRunnable(){}
		virtual void run(Thread& thread) throw() = 0;
	};

	Thread();
	Thread(IRunnable& runnable, bool autoDeleteRunnable = true);
	~Thread();

	void start(IRunnable& runnable, bool autoDeleteRunnable = true);
	IRunnable* runnable() const
	{
		return mRunnable;
	}

	void postQuit();
	bool keepRun() const;
	void wait();

	enum Priority
	{
		LowPriority = -1,
		NormalPriority = 0,
		HighPriority = 1
	};

	Priority getPriority() const;
	void setPriority(Priority priority);

	int id() const;
	bool isWaitable() const;
	void throwIfWaited() const;

protected:
	void init();
	void cleanup();

	IRunnable* mRunnable;
	int mId;
	void* mHandle;
	//intptr_t mHandle;
	Priority mPriority;
	bool mAutoDeleteRunnable;
	AtomicValue<bool> mKeepRun;
	mutable RecursiveMutex mMutex;
}; //Thread

int getCurrentThreadId();
void mSleep(size_t millseconds);
void uSleep(useconds_t microseconds);