#pragma once
#include "NonCopyable.h"

class Mutex : private NonCopyable
{
public:
	Mutex(int spinCount = 200);
	~Mutex();

	void lock();
	void unlock();
	bool trylock();

	//CRITICAL_SECTION mMutex;
	char mMutex[24];

};

class RecursiveMutex : private NonCopyable
{
public:
	RecursiveMutex(int spinCount = 200);
	~RecursiveMutex();

	void lock();
	void unlock();
	bool trylock();

	//CRITICAL_SECTION mMutex;
	char mMutex[24];
};

class Cancelable
{
public:
	Cancelable() : mCanceled(false) {}
	void cancel() {mCanceled = true;}
	void resume() {mCanceled = false;}
	bool isCanceled() {return mCanceled;}
protected:
	bool mCanceled;
};


//SCOPELOCK
class ScopeLock : public Cancelable, private NonCopyable
{
public:
	explicit ScopeLock(Mutex& m) : Cancelable(), m(&m) {m.lock();}
	explicit ScopeLock(Mutex* m) : Cancelable(), m(m) {if(m) m->lock(); else cancel();}
	~ScopeLock() {if(!isCanceled()) m->unlock();}
	void swapMutex(Mutex& other) {m->unlock(); m = &other; m->lock();}
	Mutex& mutex() {return *m;}
protected:
	Mutex* m;
};

class ScopeUnLock : public Cancelable, private NonCopyable
{
public:
	explicit ScopeUnLock(Mutex& m) : Cancelable(), m(&m) {m.unlock();}
	explicit ScopeUnLock(Mutex* m) : Cancelable(), m(m) {if(m) m->unlock(); else cancel();}
	~ScopeUnLock() {if(!isCanceled()) m->lock();}
	Mutex& mutex() {return *m;}
protected:
	Mutex* m;
};

class ScopeUnLockOnly : public Cancelable, private NonCopyable
{
public:
	explicit ScopeUnLockOnly(Mutex& m) : Cancelable(), m(&m) {}
	explicit ScopeUnLockOnly(Mutex* m) : Cancelable(), m(m) {if(!m) cancel();}
	~ScopeUnLockOnly() {if(!isCanceled()) m->lock();}
	Mutex& mutex() {return *m;}
protected:
	Mutex* m;
};

//SCOPERECURSIVELOCK
class ScopeRecursiveLock : public Cancelable, private NonCopyable
{
public:
	explicit ScopeRecursiveLock(RecursiveMutex& m) : Cancelable(), m(&m) {m.lock();}
	explicit ScopeRecursiveLock(RecursiveMutex* m) : Cancelable(), m(m) {if(m) m->lock(); else cancel();}
	~ScopeRecursiveLock() {if(!isCanceled()) m->unlock();}
	void swapMutex(RecursiveMutex& other) {m->unlock(); m = &other; m->lock();}
	RecursiveMutex& mutex() {return *m;}
protected:
	RecursiveMutex* m;
};

class ScopeRecursiveUnLock : public Cancelable, private NonCopyable
{
public:
	explicit ScopeRecursiveUnLock(RecursiveMutex& m) : Cancelable(), m(&m) {m.unlock();}
	explicit ScopeRecursiveUnLock(RecursiveMutex* m) : Cancelable(), m(m) {if(m) m->unlock(); else cancel();}
	~ScopeRecursiveUnLock() {if(!isCanceled()) m->lock();}
	RecursiveMutex& mutex() {return *m;}
protected:
	RecursiveMutex* m;
};

class ScopeRecursiveUnLockOnly : public Cancelable, private NonCopyable
{
public:
	explicit ScopeRecursiveUnLockOnly(RecursiveMutex& m) : Cancelable(), m(&m) {}
	explicit ScopeRecursiveUnLockOnly(RecursiveMutex* m) : Cancelable(), m(m) {if(!m) cancel();}
	~ScopeRecursiveUnLockOnly() {if(!isCanceled()) m->lock();}
	RecursiveMutex& mutex() {return *m;}
protected:
	RecursiveMutex* m;
};