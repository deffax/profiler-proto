#include "MemoryProfiler.h"
#include <iostream>
#include "Mutex.h"

#include <thread>
#include <vector>
#include <algorithm> //for_each
#include "Timer.h"
#include <list>
#include <conio.h>	// For _kbhit()




//MUTEX TEST
/*
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
*/
//MEMORY PROFILER TEST

class TestObj
{
	typedef MemoryProfiler::Scope Scope;
public:
	void FA()
	{
		Scope s("FA");
		{
			void* b = malloc(2);
			
			//free(b);
			
		}
		FB();
		FC();
		//FD();
	}

	void FB()
	{
		Scope s("FB");
		FE();
		R1(3);
	}

	void FC()
	{
		Scope s("FC");
	}

	void FD()
	{
		Scope s("FD");
		{
			void* b = malloc(1);
			//free(b);
			void* c = malloc(1);
			//void* d = malloc(1);
			free(b);
			free(c);
			//free(d);
		}
	}

	void FE()
	{
		Scope s("FE");

	}

	void R1(int count)
	{
		Scope s("R1");
		if(count > 0)
			R1(count -1);
	}

};

//int main()
//{
//	MemoryProfiler& memoryProfiler = MemoryProfiler::singleton();
//	memoryProfiler.setEnable(true);
//	Timer timer;
//	//Mutex mutex;
//	//std::vector<std::thread> threads;
//	TestObj obj;
//	
//	/*
//	for(unsigned int i = 0; i < 1; ++i)
//	{
//		threads.push_back(std::thread([&obj, &memoryProfiler, &timer]() { 
//			
//				while(true)				
//				{
//				obj.functionA();
//		
//				memoryProfiler.nextFrame();
//				if(timer.get().asSecond() > 1)
//				{
//					
//					{	
//						
//						MemoryProfiler::Scope s("system(\"cls\")");
//						::system("cls");
//					}
//					
//					std::string s = memoryProfiler.defaultReport(20, 0);
//					//std::cout << "Press any key to quit...\n\n";
//					std::cout << s << std::endl;
//					memoryProfiler.reset();
//					timer.reset();
//			
//				}
//		
//				}
//				
//			 }));
//	}
//	*/
//	while(true) {
//		obj.FA();
//		// Inform the profiler we move to the next iteration
//		memoryProfiler.nextFrame();
//
//		// Refresh and display the profiling result every 1 second
//		if(timer.get().asSecond() > 1) {
//			{	MemoryProfiler::Scope s("system(\"cls\")");
//				::system("cls");
//			}
//
//			std::string s = memoryProfiler.defaultReport(20, 0);
//			std::cout << "Press any key to quit...\n\n";
//			std::cout << s << std::endl;
//			memoryProfiler.reset();
//			timer.reset();
//
//			if(_kbhit())
//				return 0;
//		}
//	}
//	
//	//std::for_each(threads.begin(), threads.end(), std::mem_fn(&std::thread::join));
//	system("pause");
//	return 0;
//}*/

//ATOMIC TEST!
#include <atomic>
#include "Atomic.h"

struct AtomicCounter {
    //std::atomic<int> value;
	
	AtomicInteger value;
	//int value;
    void increment(){
        ++value;
    }

    void decrement(){
        --value;
    }

    int get(){
        //return value.load();
		return value;
    }
};

int main(){
    AtomicCounter counter;

    std::vector<std::thread> threads;
    for(int i = 0; i < 10; ++i){
        threads.push_back(std::thread([&counter](){
            for(int i = 0; i < 500; ++i){
                counter.increment();
            }
        }));
    }

    for(auto& thread : threads){
        thread.join();
    }

    std::cout << counter.get() << std::endl;
	system("pause");
    return 0;
}