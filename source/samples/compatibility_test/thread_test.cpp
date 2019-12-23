//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#include "thread_test.h"
//----------------------------------------------------------------------------//
// Unit
ThreadTestUnit::ThreadTestUnit() : TestUnit("THREAD")
{
	tasks.Add(new ThreadLauncherTask);
}

//----------------------------------------------------------------------------//
// Launcher
ThreadLauncherTask::ThreadLauncherTask() : TestTask("Thread Launcher") {}

void ThreadLauncherTask::Execute()
{
	ut::UniquePtr<ut::Job> job(new TestJob(*this, 0));
	ut::Thread test_thread(Move(job));
	ut::Sleep(600);
	test_thread.Exit();

	ut::Synchronized<int> sa;
	sa.Set(12);
	ut::Synchronized<int> sb(sa);
	sb = sa;

	ut::UniquePtr<ut::Thread> thread2;

	ut::UniquePtr<ut::Job> job2(new TestJob(*this, 1));
	thread2 = new ut::Thread(Move(job2));
}

void ThreadLauncherTask::AddReport(const ut::String& str)
{
	ut::ScopeLock sl(report_mutex);
	report += str;
}

//----------------------------------------------------------------------------//
// Job
TestJob::TestJob(ThreadLauncherTask& in_owner, int v) : owner(in_owner), a(v)
{ }

void TestJob::Execute()
{
	ut::String report;
	report.Print("\n    thread test, value is \"%i\", starting a loop..\n    ", a);

	int i = 0;
	while (exit_request.Get() != true)
	{
		ut::String number_str;
		i++;
		number_str.Print("%i ", i);
		report += number_str;
		ut::Sleep(100);
	}

	report += "finished.";

	owner.AddReport(report);
}

//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//