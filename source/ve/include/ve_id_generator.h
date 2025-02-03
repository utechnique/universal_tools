//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "ve_component.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
//----------------------------------------------------------------------------//
// ve::IdGenerator is a template class that helps to generate unique identifiers
// for any kind of entities.
template<typename IdType, ut::thread_safety::Mode mode = ut::thread_safety::Mode::off>
class IdGenerator
{
protected:
	// Type of the identifier.
	typedef IdType Id;

public:
	// Constructor.
	//    @param start_id - minimum id value to start from.
	IdGenerator(Id start_id = 0) : last(start_id)
	{ }

	// Generates unique identifier.
	//    @return - unique identifier
	Id Generate()
	{
		if (vacant.Count() > 0)
		{
			Id id = vacant.GetLast();
			vacant.PopBack();
			return id;
		}

		return last++;
	}

	// Signals that provided identifier (that was generated before)
	// is free again.
	//    @param free_id - identifier that can be reused in next
	//                     IdGenerator::Generate() call.
	void Release(Id free_id)
	{
		vacant.Add(free_id);
	}

private:
	Id last;
	ut::Array<Id> vacant;
};

//----------------------------------------------------------------------------//
// Thread-safe specialization of the ve::IdGenerator template.
template<typename IdType>
class IdGenerator<IdType, ut::thread_safety::Mode::on> : public IdGenerator<IdType, ut::thread_safety::Mode::off>
{
	typedef IdGenerator<IdType, ut::thread_safety::Mode::off> Base;
	typedef typename Base::Id Id;
public:
	// Constructor.
	//    @param start_id - minimum id value to start from.
	IdGenerator(Id start_id = 0) : Base(start_id)
	{ }

	// Generates unique identifier.
	//    @return - unique identifier
	Id Generate()
	{
		ut::ScopeLock lock(mutex);
		return Base::Generate();
	}

	// Signals that provided identifier (that was generated before)
	// is free again.
	//    @param free_id - identifier that can be reused in next
	//                     IdGenerator::Generate() call.
	void Release(Id free_id)
	{
		ut::ScopeLock lock(mutex);
		Base::Release(free_id);
	}

private:
	ut::Mutex mutex;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//