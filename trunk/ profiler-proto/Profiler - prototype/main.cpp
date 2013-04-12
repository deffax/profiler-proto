#include "MemoryProfiler.h"
#include <iostream>
#include "Mutex.h"
//#include <mutex>
#include <thread>
#include <vector>
#include <algorithm> //for_each

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



//MUTEX TEST
class counter
{
public:
    counter() : count(0) { }

    int add(int val) {
		//std::lock_guard<std::mutex> scoped_lock(mutex);
		//std::lock_guard<std::recursive_mutex> scoped_lock(mutex);
		ScopeRecursiveLock scoped_lock(mutex);
        count += val;
        return count;
    }   
    int increment() {
        //std::lock_guard<std::mutex> scoped_lock(mutex);
		//std::lock_guard<std::recursive_mutex> scoped_lock(mutex);
		ScopeRecursiveLock scoped_lock(mutex);
        return add(1);
    }

private:
    //std::mutex mutex;
	//std::recursive_mutex mutex;
	RecursiveMutex* mutex;
    int count;
};

counter c;

void change_count(void*)
{
    std::cout << "count == " << c.increment() << std::endl;
}


int main()
{
	change_count(nullptr);
	std::vector<std::thread> threads;
	for(unsigned int i = 0; i < 4; ++i)
	{
		threads.push_back(std::thread(change_count, nullptr));
	}

	std::for_each(threads.begin(), threads.end(), std::mem_fn(&std::thread::join));
	
	system("pause");
	return 0;
}

/* MEMORY PROFILER TEST
int main()
{
	MemoryProfiler& memoryProfiler = MemoryProfiler::singleton();
	memoryProfiler.setEnable(true);
	functionA();
	return 0;
}*/