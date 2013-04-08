#include "MemoryProfiler.h"
#include "functionPatcher.h"

std::chrono::milliseconds spin(200);

//function pointers typedefs
typedef LPVOID (WINAPI *MyHeapAlloc)(HANDLE, DWORD, SIZE_T);
typedef LPVOID (WINAPI *MyHeapReAlloc)(HANDLE, DWORD, LPVOID, SIZE_T);
typedef LPVOID (WINAPI *MyHeapFree)(HANDLE, DWORD, LPVOID);

LPVOID WINAPI myHeapAlloc(HANDLE, DWORD, SIZE_T);
LPVOID WINAPI myHeapReAlloc(HANDLE, DWORD, LPVOID, SIZE_T);
LPVOID WINAPI myHeapFree(HANDLE, DWORD, LPVOID);

FunctionPatcher functionPatcher;
MyHeapAlloc orgHeapAlloc;
MyHeapReAlloc orgHeapReAlloc;
MyHeapFree orgHeapFree;




/*!	A footer struct that insert to every patched memory allocation,
	aim to indicate which call stack node this allocation belongs to.
 */
struct MyMemFooter
{
	/*! The node pointer is placed in-between the 2 fourcc values,
		ensure maximum protected as possible.
	 */
	uint32_t fourCC1;
	MemoryProfilerNode* node;
	uint32_t fourCC2;

	/*!	Magic number for HeapFree verification.
		The node pointer is not used if any of the fourcc is invalid.
		It's kind of dirty, but there is no other way to indicate a pointer
		is allocated by original HeapAlloc or our patched version.
	 */
	static const uint32_t cFourCC1 = 123456789;
	static const uint32_t cFourCC2 = 987654321;
};	// MyMemFooter


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
	MemoryProfilerNode* parentNode = static_cast<MemoryProfilerNode*>(parent);
	MemoryProfilerNode* n = new MemoryProfilerNode(name, parent);
	
	if(!parentNode)
	{
		mIsMutexOwner = true;
	}
	else 
	{
		mIsMutexOwner = false;
	}
	return n;
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
	setEnable(enable());
	onThreadAttach("MAIN THREAD");
	std::timed_mutex gFooterMutex;
}

MemoryProfiler::~MemoryProfiler()
{
	setEnable(false);
	CallstackProfiler::setRootNode(nullptr);
	TlsSetValue(gTlsIndex, nullptr);
	gTlsIndex = 0;
	delete mTlsList;
}

void MemoryProfiler::setRootNode(CallstackNode* root)
{
	CallstackProfiler::setRootNode(root);
}

void MemoryProfiler::begin(const char name[])
{
	if(!enable())
		return;

	TlsStruct* tls = getTlsStruct();
	if(!tls)
		tls = reinterpret_cast<TlsStruct*>(onThreadAttach());
	MemoryProfilerNode* node = tls->currentNode();

	decltype(node->mMutex) mutex;
	std::lock_guard<std::recursive_timed_mutex> lock(mutex);
	if(name != node->name)
	{
		tls->recurseCount++;
		node = static_cast<MemoryProfilerNode*>(node->getChildByName(name));
		tls->recurseCount--;
		
		if(node->recursionCount == 0)
			tls->setCurrentNode(node);
	}

	node->begin();
	node->recursionCount++;

	
}

void MemoryProfiler::end()
{
	if(!enable())
		return;

	TlsStruct* tls = getTlsStruct();

	// The code in MemoryProfiler::begin() may be skipped because of !enable()
	// therefore we need to detect and create tls for MemoryProfiler::end() also.
	if(!tls)
		tls = reinterpret_cast<TlsStruct*>(onThreadAttach());

	MemoryProfilerNode* node = tls->currentNode();

	// Race with MemoryProfiler::reset(), MemoryProfiler::defaultReport() and commonDealloc()
	decltype(node->mMutex) mutex;
	std::lock_guard<std::recursive_timed_mutex> lock(mutex);

	node->recursionCount--;
	node->end();

	// Only back to the parent when the current node is not inside a recursive function
	if(node->recursionCount == 0)
		tls->setCurrentNode(node->parent);
}

void* MemoryProfiler::onThreadAttach(const char* threadName)
{
	TlsStruct* tls = new TlsStruct(threadName);
	{
		decltype(mTlsList->mMutex) mutex;
		std::lock_guard<std::timed_mutex> lock(mutex);
		mTlsList->push_back(tls);
	}
	TlsSetValue(gTlsIndex, tls);
	return tls;
}

bool MemoryProfiler::enable() const
{
	return CallstackProfiler::enable;
}

void MemoryProfiler::setEnable(bool flag)
{
	
	CallstackProfiler::enable = flag;
	functionPatcher.UnpatchAll();
	if(flag)
	{
		const int prologueSize[] = {5, 5, 5};
		HMODULE h = GetModuleHandle(_T("ntdll.dll"));
		void* pAlloc, *pReAlloc, *pFree;
		if(h)
		{
			pAlloc = GetProcAddress(h, "RtlAllocateHeap");
			pReAlloc = GetProcAddress(h, "RtlReAllocateHeap");
			pFree = GetProcAddress(h, "RtlFreeHeap");
		}
		else
		{
			pAlloc = &HeapAlloc;
			pReAlloc = &HeapReAlloc;
			pFree = & HeapFree;
		}

		// Back up the original function and then do patching
		/*
		orgHeapAlloc = (MyHeapAlloc) functionPatcher.copyPrologue(pAlloc, prologueSize[0]);
		orgHeapReAlloc = (MyHeapReAlloc) functionPatcher.copyPrologue(pReAlloc, prologueSize[1]);
		orgHeapFree = (MyHeapFree) functionPatcher.copyPrologue(pFree, prologueSize[2]);

		functionPatcher.patch(pAlloc, &myHeapAlloc);
		functionPatcher.patch(pReAlloc, &myHeapReAlloc);
		functionPatcher.patch(pFree, &myHeapFree);*/
	}

}

void MemoryProfiler::reset()
{
	if(!mRootNode || !enable())
		return;
	
	frameCount = 0;
	static_cast<MemoryProfilerNode*>(mRootNode)->reset();
}

void MemoryProfiler::nextFrame()
{
	if(!enable())
		return;

	++frameCount;
}

std::string MemoryProfiler::defaultReport(size_t nameLength, size_t skipMargin) const
{
	using namespace std;
	ostringstream ss;

	const size_t countWidth = 9;
	const size_t bytesWidth = 12;

	ss.flags(ios_base::left);
	ss	<< setw(nameLength)		<< "Name" << setiosflags(ios::right)
		<< setw(countWidth)		<< "TCount"
		<< setw(countWidth)		<< "SCount"
		<< setw(bytesWidth)		<< "TkBytes"
		<< setw(bytesWidth)		<< "SkBytes"
		<< setw(countWidth)		<< "SCount/F"
		<< setw(countWidth-2)	<< "Call/F"
		<< endl;

	MemoryProfilerNode* n = static_cast<MemoryProfilerNode*>(mRootNode);

	while(n)
	{	// NOTE: The following std stream operation may trigger HeapAlloc,
		// there we need to use recursive mutex here.

		// Race with MemoryProfiler::begin(), MemoryProfiler::end(), commonAlloc() and commonDealloc()
		decltype(n->mMutex) mutex;
		std::lock_guard<std::recursive_timed_mutex> lock(mutex);
		//ScopeRecursiveLock lock(n->mMutex);

		// Skip node that have no allocation at all
		if(n->callDepth() == 0 || n->exclusiveCount != 0 || n->countSinceLastReset != 0)
		{
			size_t callDepth = n->callDepth();
			const char* name = n->name;
			size_t iCount = n->inclusiveCount();
			size_t eCount = n->exclusiveCount;
			float iBytes = float(n->inclusiveBytes()) / 1024;
			float eBytes = float(n->exclusiveBytes) / 1024;
			float countSinceLastReset = float(n->countSinceLastReset) / frameCount;
			float callCount = float(n->callCount) / frameCount;

			{	// The string stream will make allocations, therefore we need to unlock the mutex
				// to prevent dead lock.
				//ScopeRecursiveUnlock unlock(n->mMutex);
				ss.flags(ios_base::left);
				ss	<< setw(callDepth) << ""
					<< setw(nameLength - callDepth) << name
					<< setiosflags(ios::right)// << setprecision(3)
					<< setw(countWidth)		<< iCount
					<< setw(countWidth)		<< eCount
					<< setw(bytesWidth)		<< iBytes
					<< setw(bytesWidth)		<< eBytes
					<< setw(countWidth)		<< countSinceLastReset
					<< setprecision(2)
					<< setw(countWidth-2)	<< callCount
					<< endl;
			}
		}

		n = static_cast<MemoryProfilerNode*>(CallstackNode::traverse(n));
	}

	return ss.str();
}

MemoryProfiler& MemoryProfiler::singleton()
{
	static MemoryProfiler instance;
	return instance;
}




