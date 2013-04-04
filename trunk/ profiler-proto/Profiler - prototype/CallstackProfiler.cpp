#include "CallstackProfiler.h"

CallstackNode::CallstackNode(const char name[], CallstackNode* parent)
	: name(name), recursionCount(0),
	parent(parent), firstChild(nullptr), sibling(nullptr)
{	
}

CallstackNode::~CallstackNode()
{
	delete firstChild;
	delete sibling;
}

CallstackNode* CallstackNode::traverse(CallstackNode* n)
{
	if(!n) return nullptr;
	if(n->firstChild)
		n = n->firstChild;
	else 
	{
		while (!n->sibling)
		{
			n = n->parent;
			if(!n)
				return nullptr;
		}
		n = n->sibling;
	}
	return n;
}

size_t CallstackNode::callDepth() const
{
	CallstackNode* p = parent;
	size_t depth = 0;
	while (p)
	{
		p = p->parent;
		++depth;
	}
	return depth;
}

CallstackNode* CallstackNode::getChildByName(const char name_[])
{
	//search for children...
	{
		CallstackNode* n = firstChild;
		while (n)
		{
			if(n->name == name_)
				return n;
			n = n->sibling;
		}
	}
	//...if none, search for all ancestor with non-zero recursion...
	{
		CallstackNode* n = parent;
		while (n)
		{
			if(n->name == name_ && n->recursionCount > 0)
				return n;
			n = n->parent;
		}
	}
	//..if none, create a new node.
	{
		CallstackNode* node = createNode(name_, this);
		if(firstChild)
		{
			CallstackNode* lastChild;
			CallstackNode* tmp = firstChild;
			do
			{
				lastChild = tmp;
				tmp = tmp->sibling;
			} while (tmp);
			lastChild->sibling = node;
		}
		else
		{
			firstChild = node;
		}
		return node;
	}
}