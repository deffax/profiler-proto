#include "MemoryProfiler.h"

MemoryProfilerNode::MemoryProfilerNode(const char name[], CallstackNode* parent)
	: CallstackNode(name, parent), callCount(0),
	exclusiveBytes(0), exclusiveCount(0), countSinceLastReset(0),
	mIsMutexOwner(false)//, mMutex(nullptr)
{
}

MemoryProfilerNode::~MemoryProfilerNode()
{
	//if(mIsMutexOwner)
		//delete mMutex;
}

void MemoryProfilerNode::begin()
{
	++callCount;
}

CallstackNode* MemoryProfilerNode::createNode(const char name[], CallstackNode* parent)
{
	return new MemoryProfilerNode(name, parent);
}

void MemoryProfilerNode::reset()
{
	MemoryProfilerNode* n1, *n2;
	{
		std::lock_guard<std::recursive_timed_mutex> lock(mMutex);
		callCount = 0;
		countSinceLastReset = 0;
		n1 = static_cast<MemoryProfilerNode*>(firstChild);
		n2 = static_cast<MemoryProfilerNode*>(sibling);
	}
	if(n1) n1->reset();
	if(n2) n2->reset();
}

size_t MemoryProfilerNode::inclusiveCount() const
{
	size_t total = exclusiveCount;
	const MemoryProfilerNode* n = static_cast<MemoryProfilerNode*>(firstChild);
	if(!n)
		return total;
	do
	{
		decltype(n->mMutex) mutex;
		std::lock_guard<std::recursive_timed_mutex> lock(mutex);
		total += n->inclusiveCount();
		n = static_cast<MemoryProfilerNode*>(n->sibling);
	} while (n);
	return total;
}

size_t MemoryProfilerNode::inclusiveBytes() const
{
	size_t total = exclusiveBytes;
	const MemoryProfilerNode* n = static_cast<MemoryProfilerNode*>(firstChild);
	if(!n)
		return total;
	do
	{
		decltype(n->mMutex) mutex;
		std::lock_guard<std::recursive_timed_mutex> lock(mutex);
		total += n->inclusiveCount();
		n = static_cast<MemoryProfilerNode*>(n->sibling);
	} while (n);
	return total;
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