//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "ut.h"
#include "test_task.h"
#include "test_unit.h"
//----------------------------------------------------------------------------//
class ContainersTestUnit : public TestUnit
{
public:
	ContainersTestUnit();
};

//----------------------------------------------------------------------------//
class ArrayOpsTask : public TestTask
{
public:
	ArrayOpsTask();
	void Execute();
};

//----------------------------------------------------------------------------//
class MapTask : public TestTask
{
public:
	MapTask();
	void Execute();
};

//----------------------------------------------------------------------------//
class TreeTask : public TestTask
{
public:
	TreeTask();
	void Execute();
};

//----------------------------------------------------------------------------//
class AVLTreeTask : public TestTask
{
public:
	AVLTreeTask();
	void Execute();
};

//----------------------------------------------------------------------------//
class SharedPtrTask : public TestTask
{
public:
	SharedPtrTask();
	void Execute();
};

//----------------------------------------------------------------------------//
class ContainerTask : public TestTask
{
public:
	ContainerTask();
	void Execute();
};

//----------------------------------------------------------------------------//
class OptionalTask : public TestTask
{
public:
	OptionalTask();
	void Execute();
};

//----------------------------------------------------------------------------//
class ResultTask : public TestTask
{
public:
	ResultTask();
	void Execute();
};

//----------------------------------------------------------------------------//
class PairTask : public TestTask
{
public:
	PairTask();
	void Execute();
};

//----------------------------------------------------------------------------//
class SmartPtrTask : public TestTask
{
public:
	SmartPtrTask();
	void Execute();
};

//----------------------------------------------------------------------------//
class TestMemoryPool
{
public:
	TestMemoryPool() : busy(0)
	{}

	TestMemoryPool(const TestMemoryPool&) = delete;
	TestMemoryPool(TestMemoryPool&&) = delete;
	TestMemoryPool& operator =(const TestMemoryPool&) = delete;
	TestMemoryPool& operator =(TestMemoryPool&&) = delete;

	ut::byte* Allocate(size_t n)
	{
		if (busy + n > size)
		{
			return nullptr;
		}

		size_t offset = busy;
		busy += n;

		return &memory[offset];
	}

	void Deallocate(ut::byte* addr, size_t n)
	{

	}

private:
	static constexpr size_t size = 1024;
	size_t busy;
	ut::byte memory[size];
};


template<typename ElementType>
class TestAllocator
{
	typedef TestMemoryPool PoolType;
public:
	TestAllocator() : pool(ut::MakeShared<PoolType>())
	{}

	ElementType* Allocate(size_t n)
	{
		return reinterpret_cast<ElementType*>(pool ? pool->Allocate(n * sizeof(ElementType)) : nullptr);
	}

	void Deallocate(ElementType* addr, size_t n)
	{
		if (pool)
		{
			pool->Deallocate(reinterpret_cast<ut::byte*>(addr), n * sizeof(ElementType));
		}
	}

private:
	ut::SharedPtr<PoolType> pool;
};

//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//