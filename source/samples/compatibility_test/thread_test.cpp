//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#include "thread_test.h"
//----------------------------------------------------------------------------//
// Unit
ThreadTestUnit::ThreadTestUnit() : TestUnit("THREAD")
{
	tasks.Add(ut::MakeUnique<ThreadProcTask>());
	tasks.Add(ut::MakeUnique<ThreadLauncherTask>());
	tasks.Add(ut::MakeUnique<ThreadPoolTask>());
}

//----------------------------------------------------------------------------//

ThreadProcTask::ThreadProcTask() : TestTask("Thread Procedure") {}

void ThreadProcTask::Execute()
{
	ut::Thread thread([this] { this->report += "Hello from another thread. Success.\n"; });

	ut::UniquePtr< ut::BaseTask<void> > task = ut::MakeUnique< ut::Task<void()> >([this] { this->report += "Hello from another thread 2. Success.\n"; });
	ut::Thread thread2(ut::Move(task));
}

//----------------------------------------------------------------------------//
// Launcher
ThreadLauncherTask::ThreadLauncherTask() : TestTask("Thread Launcher") {}

void ThreadLauncherTask::Execute()
{
	ut::UniquePtr<ut::Job> job(ut::MakeUnique<TestJob>(*this, 0));
	ut::Thread test_thread(Move(job));
	ut::this_thread::Sleep(600);
	test_thread.Exit();

	ut::Synchronized<int> sa;
	sa.Set(12);
	ut::Synchronized<int> sb(ut::Move(sa));
	ut::Synchronized<int> sc;
	sc = ut::Move(sb);

	ut::UniquePtr<ut::Thread> thread2;

	ut::UniquePtr<ut::Job> job2(ut::MakeUnique<TestJob>(*this, 1));
	thread2 = ut::MakeUnique<ut::Thread>(Move(job2));
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
	while (!exit_request.Read())
	{
		ut::String number_str;
		i++;
		number_str.Print("%i ", i);
		report += number_str;
		ut::this_thread::Sleep(100);
	}

	report += "finished.";

	owner.AddReport(report);
}

//----------------------------------------------------------------------------//
// Thread pool
ThreadPoolTask::ThreadPoolTask() : TestTask(ut::String("Thread Pool ") + ut::Print(ut::GetNumberOfProcessors()))
                                 , counter(0)
{}

template <typename T> struct SumPoolCombiner
{
public:
	SumPoolCombiner() : sum(0)
	{}

	void operator()(T element)
	{
		sum += element;
	}

	T GetSum()
	{
		ut::ScopeLock lock(mutex);
		return sum;
	}

private:
	T sum;
	ut::Mutex mutex;
};

class PoolTest
{
public:
	PoolTest(ThreadPoolTask& in_owner, ut::uint32 in_id, ut::uint32 in_depth) : owner(in_owner), id(in_id), depth(in_depth)
	{}

	int Execute(ut::String arg)
	{
		ut::uint32 r = id % 3;
		ut::this_thread::Sleep(r * 10);

		ut::uptr threat_id = static_cast<ut::uptr>(ut::this_thread::GetId());

		ut::String str_id = ut::Print(id);
		ut::String str_th = ut::Print(threat_id);
		owner.AddReport(arg + ut::String("i:") + str_id + ut::String(" t:") + str_th);

		return 10;
	}

	template<ut::pool_sync::Method sync_method>
	int ExecuteRecursive(ut::String arg, ut::ThreadPool<int, sync_method>& pool)
	{
		const ut::uint32 series = depth == 0 ? 0 : 2;
		ut::Array<PoolTest> test_obj;
		for (ut::uint32 i = 0; i < series; i++)
		{
			test_obj.Add(PoolTest(owner, i, depth - 1));
		}

		ut::Scheduler<int, SumPoolCombiner<int>, sync_method> sum_scheduler = pool.template CreateScheduler< SumPoolCombiner<int> >();
		for (ut::uint32 i = 0; i < series; i++)
		{
			auto function = ut::MemberFunction<PoolTest, int(ut::String, ut::ThreadPool<int, sync_method>&)>(&test_obj[i], &PoolTest::ExecuteRecursive<sync_method>);
			ut::UniquePtr< ut::BaseTask<int> > task(new ut::Task<int(ut::String, ut::ThreadPool<int, sync_method>&)>(function, arg + ut::Print(depth), pool));
			sum_scheduler.Enqueue(ut::Move(task));
		}
		SumPoolCombiner<int>& sum_combiner = sum_scheduler.WaitForCompletion();

		return sum_combiner.GetSum() + Execute(arg);
	}

	void ExecuteVoid()
	{
		Execute("o");
	}

private:
	const ut::uint32 depth;
	const ut::uint32 id;
	ThreadPoolTask& owner;
};

void ThreadPoolTask::Execute()
{
	const ut::uint32 series = 16;

	ut::Array<PoolTest> test_obj;
	for (ut::uint32 i = 0; i < series; i++)
	{
		test_obj.Add(PoolTest(*this, i, 2));
	}

	// plain
	ut::UniquePtr< ut::ThreadPool<void, ut::pool_sync::cond_var> > pool(ut::MakeUnique< ut::ThreadPool<void> >());
	report += ut::String("Starting plain pool test:") + ut::cret;
	ut::Scheduler<void, ut::DefaultCombiner<void>, ut::pool_sync::cond_var> scheduler = pool->CreateScheduler();
	for (ut::uint32 i = 0; i < series; i++)
	{
		auto function = ut::MemberFunction<PoolTest, void()>(&test_obj[i], &PoolTest::ExecuteVoid);
		ut::UniquePtr< ut::BaseTask<void> > task(ut::MakeUnique< ut::Task<void()> >(function));
		scheduler.Enqueue(ut::Move(task));
	}
	scheduler.WaitForCompletion();
	pool.Delete();
	report += ut::String("finished.") + ut::cret + ut::cret + "    ";

	// sum
	ut::ThreadPool<int, ut::pool_sync::cond_var> int_pool;
	report += ut::String("Starting plain pool test (sum combiner):") + ut::cret + "    ";
	ut::Scheduler<int, SumPoolCombiner<int>, ut::pool_sync::cond_var > sum_scheduler = int_pool.CreateScheduler< SumPoolCombiner<int> >();
	for (ut::uint32 i = 0; i < series; i++)
	{
		auto function = ut::MemberFunction<PoolTest, int(ut::String)>(&test_obj[i], &PoolTest::Execute);
		ut::UniquePtr< ut::BaseTask<int> > task(new ut::Task<int(ut::String)>(function, ut::String("o")));
		sum_scheduler.Enqueue(ut::Move(task));
	}
	SumPoolCombiner<int>& sum_combiner = sum_scheduler.WaitForCompletion();
	int sum = sum_combiner.GetSum();
	if (sum != 160)
	{
		report += ut::String("FAIL: Invalid sum - ") + ut::Print(sum);
		failed_test_counter.Increment();
		return;
	}
	report += ut::String("Combiner result: ") + ut::Print(sum) + ut::cret + "    ";
	report += ut::String("finished.") + ut::cret + ut::cret + "    ";

	// recursion with condition variable
	report += ut::String("Starting recursive pool test (cvar):") + ut::cret + "    ";
	for (ut::uint32 i = 0; i < series; i++)
	{
		auto function = ut::MemberFunction<PoolTest, int(ut::String, ut::ThreadPool<int, ut::pool_sync::cond_var>&)>(&test_obj[i], &PoolTest::ExecuteRecursive<ut::pool_sync::cond_var>);
		ut::UniquePtr< ut::BaseTask<int> > task(new ut::Task<int(ut::String, ut::ThreadPool<int, ut::pool_sync::cond_var>&)>(function, ut::String("o"), int_pool));
		sum_scheduler.Enqueue(ut::Move(task));
	}
	SumPoolCombiner<int>& rec_combiner = sum_scheduler.WaitForCompletion();
	sum = rec_combiner.GetSum();
	if (sum != 1280)
	{
		report += ut::String("FAIL: Invalid sum - ") + ut::Print(sum);
		failed_test_counter.Increment();
		return;
	}
	report += ut::String("Combiner result: ") + ut::Print(rec_combiner.GetSum()) + ut::cret + "    ";
	report += ut::String("finished.") + ut::cret + ut::cret + "    ";

	// recursion with atomic operations
	ut::ThreadPool<int, ut::pool_sync::atomic> int_pool_atomic;
	report += ut::String("Starting recursive pool test (atomics):") + ut::cret + "    ";
	ut::Scheduler<int, SumPoolCombiner<int>, ut::pool_sync::atomic > sum_scheduler_atomics = int_pool_atomic.CreateScheduler< SumPoolCombiner<int> >();
	for (ut::uint32 i = 0; i < series; i++)
	{
		auto function = ut::MemberFunction<PoolTest, int(ut::String, ut::ThreadPool<int, ut::pool_sync::atomic>&)>(&test_obj[i], &PoolTest::ExecuteRecursive<ut::pool_sync::atomic>);
		ut::UniquePtr< ut::BaseTask<int> > task(new ut::Task<int(ut::String, ut::ThreadPool<int, ut::pool_sync::atomic>&)>(function, ut::String("o"), int_pool_atomic));
		sum_scheduler_atomics.Enqueue(ut::Move(task));
	}
	SumPoolCombiner<int>& rec_combiner_atomics = sum_scheduler_atomics.WaitForCompletion();
	sum = rec_combiner_atomics.GetSum();
	if (sum != 1120)
	{
		report += ut::String("FAIL: Invalid sum - ") + ut::Print(sum);
		failed_test_counter.Increment();
		return;
	}
	report += ut::String("Combiner result: ") + ut::Print(rec_combiner_atomics.GetSum()) + ut::cret + "    ";
	report += ut::String("finished.") + ut::cret + "    ";
}

void ThreadPoolTask::AddReport(const ut::String& str)
{
	if (counter == 0)
	{
		report += "    ";
	}

	ut::ScopeLock sl(report_mutex);
	report += str;

	counter++;

	if (counter % 4 == 0)
	{
		report += ut::cret;
		report += "    ";
	}
	else
	{
		report += " ";
	}
}

//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//