#pragma once
#include "CallstackProfiler.h"
//#include <mutex>
#include "Mutex.h"
#include <vector>
#include <chrono>
#include <Windows.h>
#include <iomanip>
#include <sstream>
#include <tchar.h>

class MemoryProfilerNode : public CallstackNode
{
public:
	MemoryProfilerNode(const char name[], CallstackNode* parent = nullptr);
	~MemoryProfilerNode();

	void begin();
	CallstackNode* createNode(const char name[], CallstackNode* parent);

	size_t inclusiveCount() const;
	size_t inclusiveBytes() const;

	void reset();

	size_t exclusiveCount;
	size_t exclusiveBytes;
	size_t callCount;
	size_t countSinceLastReset;
	bool mIsMutexOwner;
	//std::recursive_timed_mutex mMutex;
	RecursiveMutex* mMutex;
};

class MemoryProfiler : public CallstackProfiler
{
	MemoryProfiler();
	~MemoryProfiler();
public:
	class Scope;
	static MemoryProfiler& singleton();

	void setRootNode(CallstackNode* root);

	bool enable() const;
	void setEnable(bool flag);

	void reset();
	void nextFrame();

	std::string defaultReport(size_t nameLength=100, size_t skipMargin=1) const;

	void* onThreadAttach(const char* threadName = "WORKER THREAD");

	void begin(const char name[]);
	void end();

	
	size_t frameCount;
protected:
	struct TlsList;
	TlsList* mTlsList;
};


class MemoryProfiler::Scope 
{
public:
	Scope(const char name[])
	{
		MemoryProfiler::singleton().begin(name);
	}

	~Scope()
	{
		MemoryProfiler::singleton().end();
	}
};

