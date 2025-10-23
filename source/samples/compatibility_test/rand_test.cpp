//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#include "rand_test.h"
#include <random> // for std::mt19937
//----------------------------------------------------------------------------//
// Unit
RandTestUnit::RandTestUnit() : TestUnit("RANDOM")
{
	tasks.Add(ut::MakeUnique<Mt19937Task>());
}

//----------------------------------------------------------------------------//

Mt19937Task::Mt19937Task() : TestTask("mt19937 generator") {}

void Mt19937Task::Execute()
{
	static constexpr ut::uint32 rng_max = ut::rng::Generator<ut::rng::Algorithm::mt19937>::Max();
	ut::uint32 seed = static_cast<ut::uint32>(ut::time::GetTime<ut::time::Unit::second>() % rng_max);
	report += ut::Print(ut::time::GetTime<ut::time::Unit::second, ut::time::Epoch::windows>()) + "\n";

	ut::rng::Generator<ut::rng::Algorithm::mt19937> generator(seed);
	ut::rng::Generator<ut::rng::Algorithm::mt19937> generator_copy;

	ut::rng::Generator<ut::rng::Algorithm::mt19937>* gen = &generator;
	constexpr size_t num = 0xffffff;
	ut::Array<ut::uint32> arr(num);
	for (size_t i = 0; i < num; i++)
	{
		if (i == num / 2)
		{
			generator_copy = generator;
			gen = &generator_copy;
		}

		arr[i] = gen->operator()() % 1000;
	}

	// compare with std
	std::mt19937 std_generator(seed);
	for (size_t i = 0; i < num; i++)
	{
		ut::uint32 std_value = std_generator() % 1000;
		if (arr[i] != std_value)
		{
			report += ut::String("FAIL: Invalid value for ID=") + ut::Print(i) +
				": " + ut::Print(arr[i]) + ", must be " + ut::Print(std_value);
			failed_test_counter.Increment();
			return;
		}
	}

	// avg
	double avg = 0.0f;
	const double weight = 1.0 / static_cast<double>(num);
	for (size_t i = 0; i < num; i++)
	{
		avg += weight * static_cast<double>(arr[i]);
	}
	report += ut::String("Average: ") + ut::Print(avg) + "\n";

	report += "Success";
}

//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//