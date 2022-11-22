//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "ut.h"
//----------------------------------------------------------------------------//

class TestTask : public ut::Job
{
public:
	TestTask(const ut::String& task_name);
	virtual void Execute() = 0;
	ut::String GetName();
	ut::String GetReport();
	static constexpr size_t perf_arr_count = 4096;

protected:
	ut::String name;
	ut::String report;
	ut::Mutex mutex;
};

typedef ut::UniquePtr<TestTask> TestTaskPtr;

//----------------------------------------------------------------------------//

class FailedTestCounter
{
public:
	FailedTestCounter();
	void Increment();
	ut::uint32 Count();

private:
	ut::uint32 num;
	ut::Mutex mutex;
};

extern FailedTestCounter failed_test_counter;

//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//