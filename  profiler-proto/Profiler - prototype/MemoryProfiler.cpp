#include "MemoryProfiler.h"
#include <Windows.h>

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



//TLS struct
struct TlsStruct
{
	TlsStruct(const char* name) :
		recurseCount(0),
		mCurrentNode(nullptr), threadName(::_strdup(name))
	{
	}

	~TlsStruct()
	{
		::free((void*) threadName);
	}

	MemoryProfilerNode* currentNode()
	{
		if(!mCurrentNode)
		{
			CallstackNode* rootNode = MemoryProfiler::singleton().getRootNode();
			recurseCount++;
			mCurrentNode = static_cast<MemoryProfilerNode*>(
				rootNode->getChildByName(threadName));
			recurseCount--;
		}
		return mCurrentNode;
	}

	MemoryProfilerNode* setCurrentNode(CallstackNode* node)
	{
		return mCurrentNode = static_cast<MemoryProfilerNode*>(node);
	}

	size_t recurseCount;
	const char* threadName;
protected:
	MemoryProfilerNode* mCurrentNode;
};

DWORD gTlsIndex = 0;

TlsStruct* getTlsStruct()
{
	return reinterpret_cast<TlsStruct*>(TlsGetValue(gTlsIndex));
}

struct MemoryProfiler::TlsList : public std::vector<TlsStruct*>
{
	
	~TlsList()
	{
		for(iterator i = begin(); i!= end(); ++i)
			delete(*i);
	}
	std::timed_mutex mMutex;
};


//Memory Profiler

MemoryProfiler::MemoryProfiler()
{
	mTlsList = new TlsList();
	gTlsIndex = TlsAlloc();

	setRootNode(new MemoryProfilerNode("root"));
}

MemoryProfiler::~MemoryProfiler()
{
	CallstackProfiler::setRootNode(nullptr);
}

void MemoryProfiler::setRootNode(CallstackNode* root)
{
	CallstackProfiler::setRootNode(root);
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



