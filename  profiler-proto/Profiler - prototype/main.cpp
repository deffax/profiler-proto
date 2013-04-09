#include "MemoryProfiler.h"

typedef MemoryProfiler::Scope Scope;

	
void functionC(void* newByFunB)
{
		Scope s("functionC");
		delete[] newByFunB;
		//recurse(10);
		
}

void functionB()
{
		Scope s("functionB");
		functionC(new int[10]);
		//recurse1(10);
	}


void functionA()
{
		Scope s("functionA");

		{	// Let's make some noise!
			void* b = malloc(10);
			free(b);

			b = malloc(0);
			free(b);
			free(nullptr);

			b = realloc(nullptr, 10);
			b = realloc(b, 20);		// Most likely the memory pointer does not altered
			b = realloc(b, 2000);	// Most likely the memory pointer is altered
			b = realloc(b, 0);

			b = calloc(10, 4);
			free(b);
			std::string s("hello world!");

			
		}

		functionB();
}


int main()
{
	MemoryProfiler& memoryProfiler = MemoryProfiler::singleton();
	memoryProfiler.setEnable(true);
	functionA();
	return 0;
}