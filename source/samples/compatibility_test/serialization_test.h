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
class SerializationTask : public TestTask
{
public:
	SerializationTask();
	void Execute();
};

//----------------------------------------------------------------------------//
class TextSerializationTask : public TestTask
{
public:
	TextSerializationTask();
	void Execute();
};

//----------------------------------------------------------------------------//
class SerializationSubClass : public ut::Serializable
{
public:
	SerializationSubClass();

	void Serialize(ut::MetaStream& stream);

	ut::uint16 u16val;
	ut::String str;
	ut::Array<ut::int32> iarr;
};

//----------------------------------------------------------------------------//
class TestBase : public ut::Serializable, public ut::Polymorphic
{
public:
	TestBase();
	virtual const ut::DynamicType& Identify() const;
	void Serialize(ut::MetaStream& stream);

	float fval;
};

//----------------------------------------------------------------------------//
class ReflectiveBase : public ut::Reflective, public ut::Polymorphic
{
public:
	ReflectiveBase();
	virtual const ut::DynamicType& Identify() const;
	virtual void Register(ut::MetaRegistry& registry);

	ut::String base_str;
};

//----------------------------------------------------------------------------//
class ReflectiveBaseAlt : public ut::Reflective, public ut::Polymorphic
{
public:
	ReflectiveBaseAlt();
	virtual const ut::DynamicType& Identify() const;
	virtual void Register(ut::MetaRegistry& registry);

	ut::String base_str;
};

//----------------------------------------------------------------------------//
class PolymorphicA : public TestBase
{
public:
	PolymorphicA(ut::int32 in_ival = 0, ut::uint32 in_uval = 0);
	const ut::DynamicType& Identify() const;
	void Serialize(ut::MetaStream& stream);

	ut::int32  ival;
	ut::uint32 uval;
};

//----------------------------------------------------------------------------//
class PolymorphicB : public TestBase
{
public:
	PolymorphicB(const char* in_str = "", ut::uint16 in_uval = 0);
	const ut::DynamicType& Identify() const;
	void Serialize(ut::MetaStream& stream);

	ut::String  str;
	ut::uint16 uval;
};

//----------------------------------------------------------------------------//
class ReflectiveA : public ReflectiveBase
{
public:
	ReflectiveA(ut::int32 in_ival = 0, ut::uint32 in_uval = 0);
	const ut::DynamicType& Identify() const;
	void Register(ut::MetaRegistry& registry);

	ut::int32  ival;
	ut::uint32 uval;
};

//----------------------------------------------------------------------------//
class ReflectiveB : public ReflectiveBase, public ut::NonCopyable
{
public:
	ReflectiveB(const char* in_str = "", ut::uint16 in_uval = 0, ut::int32 in_ival = 0);
	const ut::DynamicType& Identify() const;
	void Register(ut::MetaRegistry& registry);

	ut::String alt_str;
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
	void Register(ut::MetaRegistry& registry);

	ut::int32 ival;
	ut::String  bstr;
	ut::uint16 uval;
	ut::UniquePtr<ut::int32> i_ptr;
	ut::Array< ut::UniquePtr<ut::byte> > b_ptr_arr;
};

//----------------------------------------------------------------------------//
class SerializationTest : public ut::Serializable
{
public:
	SerializationTest(bool in_alternate);

	void Serialize(ut::MetaStream& stream);

	bool alternate;

	SerializationSubClass a;

	ut::Array<SerializationSubClass> arr;

	ut::int32  ival;
	ut::uint32 uval;
	ut::String str;
	ut::Array<ut::String> strarr;
	ut::UniquePtr<TestBase> dyn_type_ptr;
	ut::Array< ut::Array<ut::String> > strarrarr;
	ut::Array< ut::UniquePtr<ut::uint16> > u16ptrarr;
	ut::UniquePtr<ReflectiveBase> reflective_param;
	ut::UniquePtr<ReflectiveBaseAlt> reflective_param_alt;
	float      fval;
};

//----------------------------------------------------------------------------//
// Changes serialized object to validate it further
// (if loading will fail - parameters would have default values against changed ones)
void ChangeSerializedObject(SerializationTest& object);

// Checks if serialized object was loaded with the correct values,
// note that ChangeSerializedObject() must be called before saving an object
bool CheckSerializedObject(const SerializationTest& object);

//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//