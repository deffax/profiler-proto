#include "MemoryProfiler.h"

typedef MemoryProfiler::Scope Scope;

	

void functionB()
{
	Scope s("functionB");
}


void functionA()
{
	Scope s("functionA");
	functionB();
}


int main()
{
	MemoryProfiler& memoryProfiler = MemoryProfiler::singleton();
	memoryProfiler.setEnable(true);
	functionA();
	return 0;
}