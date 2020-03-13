//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "ut.h"
#include "test_task.h"
#include "test_unit.h"
//----------------------------------------------------------------------------//
class ThreadTestUnit : public TestUnit
{
public:
	ThreadTestUnit();
};

//----------------------------------------------------------------------------//
class ThreadProcTask : public TestTask
{
public:
	ThreadProcTask();
	void Execute();

private:
	ut::Mutex report_mutex;
};

//----------------------------------------------------------------------------//
class ThreadLauncherTask : public TestTask
{
public:
	ThreadLauncherTask();
	void Execute();
	void AddReport(const ut::String& str);
private:
	ut::Mutex report_mutex;
};

//----------------------------------------------------------------------------//
class TestJob : public ut::Job
{
public:
	TestJob(ThreadLauncherTask& owner, int v = 0);
	void Execute();

private:
	int a;
	ut::Synchronized<int> b;
	ThreadLauncherTask& owner;
};

//----------------------------------------------------------------------------//
class ThreadPoolTask : public TestTask
{
public:
	ThreadPoolTask();
	void Execute();
	void AddReport(const ut::String& str);
	ut::Mutex report_mutex;

private:
	ut::uint32 counter;
};

//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//