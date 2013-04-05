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
	functionA();
	return 0;
}