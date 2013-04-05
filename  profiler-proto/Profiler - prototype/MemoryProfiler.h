#pragma once
#include "CallstackProfiler.h"
#include <mutex>

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
	std::recursive_timed_mutex mMutex;
};

class MemoryProfiler : public CallstackProfiler
{
	MemoryProfiler();
	~MemoryProfiler();
public:
	class Scope;
	static MemoryProfiler& singleton();

	void begin(const char name[]);
	void end();
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

