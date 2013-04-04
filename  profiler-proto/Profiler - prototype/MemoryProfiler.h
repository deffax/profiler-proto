#pragma once
#include "CallstackProfiler.h"

class MemoryProfilerNode : public CallstackNode
{
public:
	MemoryProfilerNode(const char name[], CallstackNode* parent = nullptr);
	~MemoryProfilerNode();

	void begin();
	CallstackNode* createNode(const char name[], CallstackNode* parent);

	size_t callCount;
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