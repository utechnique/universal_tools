//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#include "polymorphic_test.h"
//----------------------------------------------------------------------------//

class PolymorphicTestBase : public ut::Polymorphic
{
public:
	const ut::DynamicType& Identify() const override { return ut::Identify(this); }
};

class PolymorphicTestA : public PolymorphicTestBase
{
public:
	const ut::DynamicType& Identify() const override { return ut::Identify(this); }
};

class PolymorphicTestB : public PolymorphicTestBase
{
public:
	virtual const ut::DynamicType& Identify() const override { return ut::Identify(this); }
};

class PolymorphicTestC : public PolymorphicTestB
{
public:
	virtual const ut::DynamicType& Identify() const override { return ut::Identify(this); }
};

class PolymorphicTestD : public PolymorphicTestC
{
public:
	virtual const ut::DynamicType& Identify() const override { return ut::Identify(this); }
};

class PolymorphicTestE : public PolymorphicTestD
{
public:
	virtual const ut::DynamicType& Identify() const override { return ut::Identify(this); }
};

//----------------------------------------------------------------------------//

// simple polymorphic types
UT_REGISTER_TYPE(PolymorphicTestBase, PolymorphicTestBase, "polymorphic_base")
UT_REGISTER_TYPE(PolymorphicTestBase, PolymorphicTestA, "polymorphic_a")
UT_REGISTER_TYPE(PolymorphicTestBase, PolymorphicTestB, "polymorphic_b")
UT_REGISTER_TYPE(PolymorphicTestB, PolymorphicTestC, "polymorphic_c")
UT_REGISTER_TYPE(PolymorphicTestC, PolymorphicTestD, "polymorphic_d")
UT_REGISTER_TYPE(PolymorphicTestD, PolymorphicTestE, "polymorphic_e")

//----------------------------------------------------------------------------//
PolymorphicTestUnit::PolymorphicTestUnit() : TestUnit("POLYMORPHIC")
{
	tasks.Add(ut::MakeUnique<PolymorphicBaseTestTask>());
	tasks.Add(ut::MakeUnique<PolymorphicFactoryTestTask>());
	tasks.Add(ut::MakeUnique<PolymorphicClassificationTask>());
}

//----------------------------------------------------------------------------//
PolymorphicBaseTestTask::PolymorphicBaseTestTask() : TestTask("Base test")
{ }

void PolymorphicBaseTestTask::Execute()
{
	ut::DynamicType::Handle handles[5] = {
		ut::GetPolymorphicHandle<PolymorphicTestA>(),
		ut::GetPolymorphicHandle<PolymorphicTestB>(),
		ut::GetPolymorphicHandle<PolymorphicTestC>(),
		ut::GetPolymorphicHandle<PolymorphicTestD>(),
		ut::GetPolymorphicHandle<PolymorphicTestE>()
	};

	report += "Handles: ";
	for (size_t i = 0; i < 5; i++)
	{
		for (size_t j = 0; j < 5; j++)
		{
			if (i == j)
			{
				continue;
			}

			if (handles[i] == handles[j])
			{
				report += "Error! handles are not unique: ";
				report += ut::Print(i) + " and " + ut::Print(j);
				failed_test_counter.Increment();
				return;
			}
		}

		ut::uint64 hn = static_cast<ut::uint64>(handles[i]);
		ut::String h_str;
		h_str.Print("0x%llx", hn);
		report += h_str + " ";
	}

	report += "Success ";
}

//----------------------------------------------------------------------------//
PolymorphicFactoryTestTask::PolymorphicFactoryTestTask() : TestTask("Factory test")
{ }

void PolymorphicFactoryTestTask::Execute()
{
	report += "\nIterate all types: ";

	size_t type_count = ut::Factory<PolymorphicTestBase>::CountTypes();
	if (type_count != 6)
	{
		report += "Error! Invalid number of types.";
		failed_test_counter.Increment();
		return;
	}

	for (size_t i = 0; i < type_count; i++)
	{
		const ut::DynamicType& dynamic_type = ut::Factory<PolymorphicTestBase>::GetTypeByIndex(i);
		report += ut::String("\n    ") + dynamic_type.GetName();
	}

	report += "\nIterate intermediate types: ";

	type_count = ut::Factory<PolymorphicTestC>::CountTypes();
	if (type_count != 3)
	{
		report += "Error! Invalid number of types.";
		failed_test_counter.Increment();
		return;
	}

	for (size_t i = 0; i < type_count; i++)
	{
		const ut::DynamicType& dynamic_type = ut::Factory<PolymorphicTestC>::GetTypeByIndex(i);
		report += ut::String("\n    ") + dynamic_type.GetName();
	}

	report += "\nSuccess ";
}

//----------------------------------------------------------------------------//
PolymorphicClassificationTask::PolymorphicClassificationTask() : TestTask("Classification")
{}

void PolymorphicClassificationTask::Execute()
{
	ut::Array< ut::UniquePtr<PolymorphicTestBase> > base;

	base.Add(ut::MakeUnique<PolymorphicTestA>());
	base.Add(ut::MakeUnique<PolymorphicTestA>());
	base.Add(ut::MakeUnique<PolymorphicTestA>());
	base.Add(ut::MakeUnique<PolymorphicTestB>());
	base.Add(ut::MakeUnique<PolymorphicTestC>());
	base.Add(ut::MakeUnique<PolymorphicTestC>());
	base.Add(ut::MakeUnique<PolymorphicTestD>());
	base.Add(ut::MakeUnique<PolymorphicTestE>());


	ut::Array< ut::Ref<PolymorphicTestA> > derived_a;
	ut::Factory<PolymorphicTestBase>::Select<PolymorphicTestA>(base, derived_a);

	if (derived_a.GetNum() != 3)
	{
		report += ut::String("FAILED: Invalid number of derived objects.") + ut::CRet();
		failed_test_counter.Increment();
		return;
	}

	ut::meta::Selector<
		PolymorphicTestBase,
		PolymorphicTestA,
		PolymorphicTestB,
		PolymorphicTestC
	> selector;

	selector.Select(base);

	ut::Array< ut::Ref<PolymorphicTestA> >& a = selector.Get<PolymorphicTestA>();
	ut::Array< ut::Ref<PolymorphicTestB> >& b = selector.Get<PolymorphicTestB>();
	ut::Array< ut::Ref<PolymorphicTestC> >& c = selector.Get<PolymorphicTestC>();

	if (a.GetNum() != 3 || b.GetNum() != 1 || c.GetNum() != 2)
	{
		report += ut::String("FAILED: Invalid number of derived objects.") + ut::CRet();
		failed_test_counter.Increment();
		return;
	}

	selector.Reset();

	if (a.GetNum() != 0 || b.GetNum() != 0 || c.GetNum() != 0)
	{
		report += ut::String("FAILED: non-empty array after a Reset() call.") + ut::CRet();
		failed_test_counter.Increment();
		return;
	}

	if (a.GetCapacity() == 0 || b.GetCapacity() == 0 || c.GetCapacity() == 0)
	{
		report += ut::String("FAILED: capacity became 0 after a Reset() call.") + ut::CRet();
		failed_test_counter.Increment();
		return;
	}

	selector.Select(base);

	if (a.GetNum() != 3 || b.GetNum() != 1 || c.GetNum() != 2)
	{
		report += ut::String("FAILED: Invalid number of derived objects.") + ut::CRet();
		failed_test_counter.Increment();
		return;
	}

	report += "Ok.";
}

//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//