#include "Mutex.h"
#include "PlatformInclude.h"


//MUTEX
Mutex::Mutex(int spinCount)
{
	if(spinCount< 0 || !::InitializeCriticalSectionAndSpinCount((LPCRITICAL_SECTION)&mMutex, spinCount))
		::InitializeCriticalSection((LPCRITICAL_SECTION)&mMutex);
}

Mutex::~Mutex()
{
	::DeleteCriticalSection((LPCRITICAL_SECTION)&mMutex);
}

void Mutex::lock()
{
	::EnterCriticalSection((LPCRITICAL_SECTION)&mMutex);
}

void Mutex::unlock()
{
	::LeaveCriticalSection((LPCRITICAL_SECTION)&mMutex);
}

bool Mutex::trylock()
{
	if(::TryEnterCriticalSection((LPCRITICAL_SECTION)&mMutex) >0)
	{
		return true;
	}
	else
	{
		return false;
	}
}

//RECURSIVE MUTEX

RecursiveMutex::RecursiveMutex(int spinCount)
{
	if(spinCount< 0 || !::InitializeCriticalSectionAndSpinCount((LPCRITICAL_SECTION)&mMutex, spinCount))
		::InitializeCriticalSection((LPCRITICAL_SECTION)&mMutex);
}

RecursiveMutex::~RecursiveMutex()
{
	::DeleteCriticalSection((LPCRITICAL_SECTION)&mMutex);
}

void RecursiveMutex::lock()
{
	::EnterCriticalSection((LPCRITICAL_SECTION)&mMutex);
}

void RecursiveMutex::unlock()
{
	::LeaveCriticalSection((LPCRITICAL_SECTION)&mMutex);
}

bool RecursiveMutex::trylock()
{
	return ::TryEnterCriticalSection((LPCRITICAL_SECTION)&mMutex) >0;
}


