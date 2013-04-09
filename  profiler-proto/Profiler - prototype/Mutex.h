#pragma once
#include "NonCopyable.h"

class Mutex : private NonCopyable
{
public:
	Mutex(int spinCount = 200);
	~Mutex();

	void lock();
	void unlock();
	bool trylock();

	//CRITICAL_SECTION mMutex;
	char mMutex[24];

};