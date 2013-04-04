#include "MemoryProfiler.h"

MemoryProfilerNode::MemoryProfilerNode(const char name[], CallstackNode* parent)
	: CallstackNode(name, parent), callCount(0)
{
}

MemoryProfilerNode::~MemoryProfilerNode()
{
}

void MemoryProfilerNode::begin()
{
	++callCount;
}

CallstackNode* MemoryProfilerNode::createNode(const char name[], CallstackNode* parent)
{
	return new MemoryProfilerNode(name, parent);
}


//Memory Profiler

MemoryProfiler::MemoryProfiler()
{
}

MemoryProfiler::~MemoryProfiler()
{
	CallstackProfiler::setRootNode(nullptr);
}

void MemoryProfiler::begin(const char name[])
{
	
}

void MemoryProfiler::end()
{

}

MemoryProfiler& MemoryProfiler::singleton()
{
	static MemoryProfiler instance;
	return instance;
}