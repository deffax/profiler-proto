#pragma once
#include "NonCopyable.h"

class CallstackNode : private NonCopyable
{
public:
	CallstackNode(const char name[], CallstackNode* parent = nullptr);
	virtual ~CallstackNode();
	
	virtual void begin(){};
	virtual void end(){};
	virtual CallstackNode* createNode(const char name[], CallstackNode* parent) = 0;

	static CallstackNode* traverse(CallstackNode* n);

	CallstackNode* getChildByName(const char name[]);
	
	size_t callDepth() const;
	

	const char* name;
	size_t recursionCount;
	CallstackNode* parent;
	CallstackNode* sibling;
	CallstackNode* firstChild;
};

class CallstackProfiler : private NonCopyable
{
public:
	CallstackProfiler();
	~CallstackProfiler();

	virtual void begin(const char name[]);
	virtual void end();

	void setRootNode(CallstackNode* root);
	CallstackNode* getRootNode() { return mRootNode;}
	CallstackNode* getCurrentNode() {return mCurrentNode;}

	bool enable;
protected:
	CallstackNode* mRootNode;
	CallstackNode* mCurrentNode;
};