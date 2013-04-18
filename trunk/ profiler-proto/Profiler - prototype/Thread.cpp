#include "Thread.h"
#include "PlatformInclude.h"
#include <memory.h>
#include <iostream>




DWORD WINAPI _Run(LPVOID p)
{

	Thread* t = reinterpret_cast<Thread*>(p);
	Thread::IRunnable* runnable = t->runnable();
	
	runnable->run(*t);
	return 0;
}

Thread::Thread()
{
	init();
}

Thread::Thread(IRunnable& runnable, bool autoDeleteRunnable)
{
	init();
	start(runnable, autoDeleteRunnable);
}

void Thread::init()
{
	mRunnable = nullptr;

	mHandle = nullptr;
	mId = 0;

	mPriority = NormalPriority;
	mAutoDeleteRunnable = false;
	mKeepRun = false;
}

Thread::~Thread()
{
	// No need to lock

	if(isWaitable())
		wait();
	else
		cleanup();

	if(mAutoDeleteRunnable)
		delete mRunnable;
}

void Thread::start(IRunnable& runnable, bool autoDeleteRunnable)
{
	ScopeRecursiveLock lock(mMutex);

	if(mHandle)
		throw std::logic_error("A thread is already in execution, call wait() before starting another one");

	// Delete the previous runnable if needed
	if(mRunnable && mAutoDeleteRunnable)
		delete mRunnable;

	mAutoDeleteRunnable = autoDeleteRunnable;
	mRunnable = &runnable;
	mKeepRun = true;


	
	mHandle = reinterpret_cast<void*>(::CreateThread(nullptr, 0, &_Run, this, 0, (LPDWORD)&mId));
	if(!mHandle)

		std::cout << "Warning: Error creating thread\n";
}

void Thread::postQuit()
{
	// No need to lock (mKeepRun is an AtomicValue)
	mKeepRun = false;
}

bool Thread::keepRun() const
{
	// No need to lock (mKeepRun is an AtomicValue)
	return mKeepRun;
}

bool Thread::isWaitable() const
{
	ScopeRecursiveLock lock(mMutex);
	return mHandle && getCurrentThreadId() != id();
}

void Thread::throwIfWaited() const
{
	ScopeRecursiveLock lock(mMutex);
	if(!mHandle)
		throw std::logic_error("The thread is already stopped");
}

void Thread::cleanup()
{
	ScopeRecursiveLock lock(mMutex);

	if(!mHandle)
		return;


	mId = 0;

}

Thread::Priority Thread::getPriority() const
{
	ScopeRecursiveLock lock(mMutex);

	throwIfWaited();


	int ret = ::GetThreadPriority(reinterpret_cast<HANDLE>(mHandle));
	if(ret == THREAD_PRIORITY_ERROR_RETURN)
		std::cout << "Warning: Error getting thread priority\n";

	if(ret > 0)
		ret = HighPriority;
	else if(ret < 0)
		ret = LowPriority;

	return Priority(ret);

}

void Thread::setPriority(Priority priority)
{
	ScopeRecursiveLock lock(mMutex);

	throwIfWaited();


	if(::SetThreadPriority(reinterpret_cast<HANDLE>(mHandle), int(priority)) == 0)
		std::cout << "Warning: Error setting thread priority\n";

}

void Thread::wait()
{
	ScopeRecursiveLock lock(mMutex);

	if(!isWaitable())
		throw std::logic_error("The thread is not waitable");

	postQuit();

	


	HANDLE handleBackup = reinterpret_cast<HANDLE>(mHandle);
//	mHandle = nullptr;

	
	mId = 0;

	// Unlock the mutex before the actual wait operation
	lock.mutex().unlock();
	DWORD res = ::WaitForSingleObject(handleBackup, INFINITE);
	lock.mutex().lock();

	if(res == WAIT_FAILED)
		std::cout << "Warning: Error waiting thread\n";
	


}

int Thread::id() const
{
	ScopeRecursiveLock lock(mMutex);

	return mId;

}

int getCurrentThreadId()
{

	return ::GetCurrentThreadId();

}

void mSleep(size_t millseconds)
{

	::Sleep(DWORD(millseconds));

}

void uSleep(useconds_t microseconds)
{

	::Sleep(microseconds / 1000);

}

