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
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//