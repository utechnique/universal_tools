//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#include "signal_test.h"
//----------------------------------------------------------------------------//
SignalTestUnit::SignalTestUnit() : TestUnit("SIGNALS")
{
	tasks.Add(new Slot0Task);
	tasks.Add(new Slot1Task);
	tasks.Add(new Slot2Task);
	tasks.Add(new Slot3Task);
}

//----------------------------------------------------------------------------//
Slot0Task::Slot0Task() : TestTask("0 arguments")
{ }

static int g_signal_test = 0;

void Signal0Proc()
{
	g_signal_test += 101;
}

void Slot0Task::Execute()
{
	g_signal_test = 0;
	ut::Signal<void()> signal;
	signal.Connect(Signal0Proc);
	signal.Connect(Signal0Proc);
	signal();

	if (g_signal_test == 202)
	{
		report += ut::String("success");
	}
	else
	{
		report += ut::String("FAIL");
		failed_test_counter.Increment();
	}
}

//----------------------------------------------------------------------------//
Slot1Task::Slot1Task() : TestTask("1 argument")
{ }

void Signal1Proc(int a)
{
	g_signal_test += a;
}

void Slot1Task::Execute()
{
	ut::FunctionTraits<const char*(int, const char*)>::ReturnType str = "str";
	ut::FunctionTraits<const char*(int, const char*)>::Argument<0>::Type i_test = 0;
	ut::FunctionTraits<const char*(int, const char*)>::Argument<1>::Type str_test = "test";

	g_signal_test = 0;
	ut::Signal<void(int)> signal;
	signal.Connect(Signal1Proc);
	signal(3);
	signal(2);
	signal(1);

	if (g_signal_test == 6)
	{
		report += ut::String("success");
	}
	else
	{
		report += ut::String("FAIL");
		failed_test_counter.Increment();
	}
}

//----------------------------------------------------------------------------//
Slot2Task::Slot2Task() : TestTask("2 arguments")
{ }

static ut::String g_signal_test_str = "";

void Signal2ProcA(int a, ut::String s)
{
	g_signal_test_str.Print("%i", a);
	g_signal_test_str += s;
}

void Slot2Task::Execute()
{
	g_signal_test_str.SetEmpty();
	ut::Signal<void(int, ut::String)> signalA;
	signalA.Connect(Signal2ProcA);
	ut::String test_string("test");
	signalA(40, test_string);

	if (g_signal_test_str != "40test")
	{
		report += ut::String("FAIL");
		failed_test_counter.Increment();
		return;
	}

	if (test_string != "test")
	{
		report += ut::String("FAIL");
		failed_test_counter.Increment();
		return;
	}

	g_signal_test_str.SetEmpty();
	test_string = "moved";
	signalA(51, Move(test_string));

	if (g_signal_test_str != "51moved")
	{
		report += ut::String("FAIL");
		failed_test_counter.Increment();
		return;
	}

	ut::Container<int, int> c(0, 1);
	c.Get<0>() = 2;
	c.Get<1>() = 3;

	report += ut::String("success");
}

//----------------------------------------------------------------------------//
Slot3Task::Slot3Task() : TestTask("3 arguments")
{ }

int Signal3ProcA(int a, int& b, const int& c)
{
	b++;
	g_signal_test += a;
	return a + b + c;
}

int Signal3ProcB(int a, int& b, const int& c)
{
	b++;
	g_signal_test += c;
	return a * b * c;
}

struct SumSignalCombiner
{
	SumSignalCombiner() : sum(0)
	{ }

	int sum;

	int operator()(const int& element)
	{
		sum += element;
		return sum;
	}
};

void Slot3Task::Execute()
{
	g_signal_test = 0;

	int i_test = 4;

	ut::Signal<int(int, int&, const int&), SumSignalCombiner> signal;
	signal.Connect(Signal3ProcA);
	signal.Connect(Signal3ProcB);
	ut::Result<int, ut::Error> result = signal(3, i_test, 2);

	if (!result)
	{
		report += ut::String("FAIL: ");
		report += result.GetAlt().GetDesc();
		failed_test_counter.Increment();
		return;
	}

	if (result.GetResult() != 10 + 36)
	{
		report += ut::String("FAIL");
		failed_test_counter.Increment();
		return;
	}

	if (i_test != 6)
	{
		report += ut::String("FAIL");
		failed_test_counter.Increment();
		return;
	}

	if (g_signal_test != 5)
	{
		report += ut::String("FAIL");
		failed_test_counter.Increment();
		return;
	}

	report += ut::String("success");
}

//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//