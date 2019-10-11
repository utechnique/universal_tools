//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "ut.h"
#include "test_task.h"
#include "test_unit.h"
//----------------------------------------------------------------------------//
class SerializationTestUnit : public TestUnit
{
public:
	SerializationTestUnit();
};

//----------------------------------------------------------------------------//
class SerializationVariantsTask : public TestTask
{
public:
	SerializationVariantsTask();
	void Execute();

	void AddReportEntry(const ut::String& entry);

	bool TestVariant(const ut::meta::Info& info, const ut::String& name);

private:
	ut::Map<ut::String, ut::meta::Info> info_variants;
	typedef ut::Pair<ut::String, ut::meta::Info> PairType;
};

//----------------------------------------------------------------------------//
class SerializationSubClass : public ut::meta::Reflective
{
public:
	SerializationSubClass();

	void Reflect(ut::meta::Snapshot& snapshot);

	ut::uint16 u16val;
	ut::String str;
	ut::Array<ut::int32> iarr;
};

//----------------------------------------------------------------------------//
class TestBase : public ut::meta::Reflective, public ut::Polymorphic
{
public:
	TestBase();
	virtual const ut::DynamicType& Identify() const;
	void Reflect(ut::meta::Snapshot& snapshot);

	float fval;
};

//----------------------------------------------------------------------------//
class ReflectiveBase : public ut::meta::Reflective, public ut::Polymorphic
{
public:
	ReflectiveBase();
	virtual const ut::DynamicType& Identify() const;
	virtual void Reflect(ut::meta::Snapshot& snapshot);

	ut::String base_str;
};

//----------------------------------------------------------------------------//
class ReflectiveBaseAlt : public ut::meta::Reflective, public ut::Polymorphic
{
public:
	ReflectiveBaseAlt();
	virtual const ut::DynamicType& Identify() const;
	virtual void Reflect(ut::meta::Snapshot& snapshot);

	ut::String base_str;
};

//----------------------------------------------------------------------------//
class PolymorphicA : public TestBase
{
public:
	PolymorphicA(ut::int32 in_ival = 0, ut::uint32 in_uval = 0);
	const ut::DynamicType& Identify() const;
	void Reflect(ut::meta::Snapshot& snapshot);

	ut::int32  ival;
	ut::uint32 uval;
};

//----------------------------------------------------------------------------//
class PolymorphicB : public TestBase
{
public:
	PolymorphicB(const char* in_str = "", ut::uint16 in_uval = 0);
	const ut::DynamicType& Identify() const;
	void Reflect(ut::meta::Snapshot& snapshot);

	ut::String  str;
	ut::uint16 uval;
};

//----------------------------------------------------------------------------//
class ReflectiveA : public ReflectiveBase
{
public:
	ReflectiveA(ut::int32 in_ival = 0, ut::uint32 in_uval = 0);
	const ut::DynamicType& Identify() const;
	void Reflect(ut::meta::Snapshot& snapshot);

	ut::int32  ival;
	ut::uint32 uval;
};

//----------------------------------------------------------------------------//
class ReflectiveB : public ReflectiveBase, public ut::NonCopyable
{
public:
	ReflectiveB(const char* in_str = "", ut::uint16 in_uval = 0, ut::int32 in_ival = 0);
	const ut::DynamicType& Identify() const;
	void Reflect(ut::meta::Snapshot& snapshot);

	ut::String alt_str;
	ut::uint16 ui16;
	ut::String bstr;
	ut::uint16 uval;
	ut::UniquePtr<ut::int32> i_ptr;
	ut::Array< ut::UniquePtr<ut::byte> > b_ptr_arr;
};

//----------------------------------------------------------------------------//
class ReflectiveAltB : public ReflectiveBaseAlt, public ut::NonCopyable
{
public:
	ReflectiveAltB(const char* in_str = "", ut::uint16 in_uval = 0, ut::int32 in_ival = 0);
	const ut::DynamicType& Identify() const;
	void Reflect(ut::meta::Snapshot& snapshot);

	ut::int32 ival;
	ut::uint32 ui32;
	ut::String bstr;
	ut::uint16 uval;
	ut::UniquePtr<ut::int32> i_ptr;
	ut::Array< ut::UniquePtr<ut::byte> > b_ptr_arr;
};

//----------------------------------------------------------------------------//
class SharedTestLevel2 : public ut::meta::Reflective
{
public:
	SharedTestLevel2();
	void Reflect(ut::meta::Snapshot& snapshot);
	ut::SharedPtr<ut::int32> i32_shared;
};

//----------------------------------------------------------------------------//
class SharedTestLevel1 : public ut::meta::Reflective
{
public:
	SharedTestLevel1();
	void Reflect(ut::meta::Snapshot& snapshot);
	ut::SharedPtr<SharedTestLevel2> shared_obj;
};

//----------------------------------------------------------------------------//
class SharedTestLevel0 : public ut::meta::Reflective
{
public:
	SharedTestLevel0();
	void Reflect(ut::meta::Snapshot& snapshot);
	ut::SharedPtr<SharedTestLevel1> shared_obj;
};

//----------------------------------------------------------------------------//
class SerializationTest : public ut::meta::Reflective
{
public:
	SerializationTest(bool in_alternate, bool in_can_have_links);

	void Reflect(ut::meta::Snapshot& snapshot);

	bool alternate;
	bool can_have_links;

	SerializationSubClass a;

	ut::Array<SerializationSubClass> arr;

	ut::int32 ival;
	ut::int32 ival2;
	ut::int32* ival_ptr;
	ut::int32* void_ptr;
	const ut::int32* ival_const_ptr;
	ut::String* str_ptr;
	ut::String** str_ptr_ptr;
	ut::UniquePtr<ut::int16> int16_unique;
	ut::UniquePtr<ut::int16> int16_unique_void;
	ut::int16* i16_ptr;
	ReflectiveBase* refl_ptr;
	ReflectiveBase** refl_ptr_ptr;
	ReflectiveA* refl_A_ptr;
	ut::SharedPtr<ut::int32> ival_shared_ptr_0;
	ut::SharedPtr<ut::int32> ival_shared_ptr_1;
	ut::SharedPtr<ut::int32> ival_shared_ptr_2;
	ut::SharedPtr<ReflectiveBase> refl_shared_ptr;
	ut::SharedPtr<ReflectiveBase> refl_shared_void_ptr;
	ut::SharedPtr<SharedTestLevel0> refl_shared_level_ptr;
	ut::WeakPtr<ut::int32> ival_weak_ptr_0;
	ut::WeakPtr<ut::int32> deep_weak_ptr_0;
	ut::WeakPtr<ut::int32> void_weak_ptr_0;
	ut::uint64 uval;
	bool bool_val;
	ut::String str;
	ut::Array<ut::String> strarr;
	ut::UniquePtr<TestBase> dyn_type_ptr;
	ut::Array< ut::Array<ut::String> > strarrarr;
	ut::Array< ut::UniquePtr<ut::uint16> > u16ptrarr;
	ut::UniquePtr<ReflectiveBase> reflective_param;
	ut::UniquePtr<ReflectiveBaseAlt> reflective_param_alt;
	ut::UniquePtr<ReflectiveBase> void_param;
	ut::UniquePtr<ReflectiveBase> reflect_unique_ptr;
	ut::UniquePtr<ReflectiveBase> reflect_unique_ptr2;
	float fval;
};

//----------------------------------------------------------------------------//
// Changes serialized object to validate it further
// (if loading will fail - parameters would have default values against changed ones)
void ChangeSerializedObject(SerializationTest& object);

// Checks if serialized object was loaded with the correct values,
// note that ChangeSerializedObject() must be called before saving an object
bool CheckSerializedObject(const SerializationTest& object, bool alternate, bool linkage);

//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//