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
			
			//free(b); //mem lick
			
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
//#include <atomic>
//#include "Atomic.h"
//
//struct AtomicCounter {
//    //std::atomic<int> value;
//	
//	AtomicInteger value;
//	//int value;
//    void increment(){
//        ++value;
//    }
//
//    void decrement(){
//        --value;
//    }
//
//    int get(){
//        //return value.load();
//		return value;
//    }
//};
//
//int main(){
//    AtomicCounter counter;
//
//    std::vector<std::thread> threads;
//    for(int i = 0; i < 10; ++i){
//        threads.push_back(std::thread([&counter](){
//            for(int i = 0; i < 500; ++i){
//                counter.increment();
//            }
//        }));
//    }
//
//    for(auto& thread : threads){
//        thread.join();
//    }
//
//    std::cout << counter.get() << std::endl;
//	system("pause");
//    return 0;
//}

#include "Thread.h"

template<typename T>
class TestList : protected std::list<T>
{
	typedef std::list<T> Super;

public:
	void push_back(const T& val)
	{
		ScopeLock lock(mutex);
		Super::push_back(val);
	}

	void pop_front()
	{
		ScopeLock lock(mutex);
		if(!Super::empty())
			Super::pop_front();
	}

	mutable Mutex mutex;
};	// TestList

class MyObject
{
	typedef MemoryProfiler::Scope Scope;

public:
	void functionA() {
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

			testList.push_back(123);	// pop_front in functionD()
		}

		functionB();
		functionD();
	}

	void functionB() {
		Scope s("functionB");
		functionC(new int[10]);
		recurse1(10);
	}

	void functionC(void* newByFunB) {
		Scope s("functionC");
		delete[] newByFunB;
		recurse(10);
		functionD();
	}

	void functionD() {
		Scope s("functionD");
		testList.pop_front();	// push_back in functionA()
	}

	// Recurse itself
	void recurse(int count) {
		Scope s("recurse");
		free(malloc(1));
		if(count > 0)
			recurse(count - 1);
	}

	/*!	Recurse between two functions:
		recurse1 -> recurse2 -> recurse1 -> recurse2 ...
	 */
	void recurse1(int count) {
		Scope s("recurse1");
		recurse2(count);
	}

	void recurse2(int count) {
		Scope s("recurse2");
		if(count > 0)
			recurse1(count - 1);
		free(malloc(1));
	}

	// A static variable shared by all threads
	static TestList<int> testList;
};	// MyObject

TestList<int> MyObject::testList;
class LoopRunnable : public Thread::IRunnable
{
protected:
	void run(Thread& thread) throw()
	{
		while(thread.keepRun())
		{
			MemoryProfiler::Scope s("LoopRunnable::run");
			mMyObject.functionA();
		}
	}
	MyObject mMyObject;
};
//THREAD TEST

MyObject obj;
MemoryProfiler& memoryProfiler = MemoryProfiler::singleton();
	
int main()
{
	Timer timer;

	
	memoryProfiler.setEnable(true);
	LoopRunnable runnable1, runnable2, runnable3;
	Thread thread1(runnable1, false);
	thread1.setPriority(Thread::NormalPriority);
//	Thread thread2(runnable2, false);
//	thread2.setPriority(Thread::LowPriority);
//	Thread thread3(runnable3, false);
//	thread3.setPriority(Thread::LowPriority);

	while(true) {
		obj.functionA();

		// Inform the profiler we move to the next iteration
		memoryProfiler.nextFrame();

		// Refresh and display the profiling result every 1 second
		if(timer.get().asSecond() > 1) {
			{	MemoryProfiler::Scope s("system(\"cls\")");
				::system("cls");
			}

			std::string s = memoryProfiler.defaultReport(20, 0);
			std::cout << "Press any key to quit...\n\n";
			std::cout << s << std::endl;
			memoryProfiler.reset();
			timer.reset();

			if(_kbhit())
				return 0;
		}
	}
	system("pause");
	return 0;
}