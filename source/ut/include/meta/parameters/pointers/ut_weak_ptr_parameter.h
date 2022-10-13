//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "common/ut_common.h"
#include "meta/ut_meta_parameter.h"
#include "meta/ut_polymorphic.h"
#include "pointers/ut_shared_ptr.h"
#include "meta/linkage/ut_meta_linker.h"
#include "templates/ut_is_base_of.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
START_NAMESPACE(meta)
//----------------------------------------------------------------------------//
// ut::Parameter<WeakPtr> is a template specialization for weak pointers.
template<typename T, thread_safety::Mode mode>
class Parameter< WeakPtr<T, mode> > : public BaseParameter
{
	typedef WeakPtr<T, mode> WeakPtrType;
	typedef SharedPtr<T, mode> SharedPtrType;
public:
	// Constructor
	//    @param p - pointer to the managed string
	Parameter(WeakPtrType* p) : BaseParameter(p)
	{ }

	// Returns the name of the managed type
	String GetTypeName() const
	{
		return BaseParameter::DeduceTypeName< WeakPtr<T, mode> >();
	}

	// Serializes managed object.
	//    @param controller - meta controller that helps to write data
	//    @return - ut::Error if encountered an error
	Optional<Error> Save(Controller& controller)
	{
		// write value type name
		WeakPtrType& ptr_ref = *static_cast<WeakPtrType*>(ptr);
		String value_type_name = ptr_ref.IsValid() ? GetTypeNameVariant<T>() : String(Type<void>::Name());
		const Optional<Error> write_error = controller.WriteAttribute(value_type_name, node_names::skValueType);
		if (write_error)
		{
			return write_error;
		}

		// exit if pointer is empty
		if (!ptr_ref.IsValid())
		{
			return Optional<Error>();
		}

		// write id
		SharedPtrType shared_ptr = ptr_ref.Pin();
		return controller.WriteLink(this, shared_ptr.Get());
	}

	// Deserializes managed object.
	//    @param controller - meta controller that helps to read data
	//    @return - ut::Error if encountered an error
	Optional<Error> Load(Controller& controller)
	{
		// read type name
		Result<String, Error> read_type_result = controller.ReadAttribute<String>(node_names::skValueType);
		if (!read_type_result)
		{
			return read_type_result.MoveAlt();
		}

		// get a reference to the shared pointer
		WeakPtrType& ptr_ref = *static_cast<WeakPtrType*>(ptr);

		// check if serialized pointer is not null
		if (read_type_result.Get() == Type<void>::Name())
		{
			ptr_ref.Reset(); // reset current value
			return Optional<Error>(); // exit, ok
		}

		// read id and link up with the correct object
		return controller.ReadWeakLink(this);
	}

	// Links pointer with provided object (that is specified by address).
	//    @param address - address of the shared pointer to link with.
	//    @return - ut::Error if encountered an error
	Optional<Error> Link(void* address)
	{
		WeakPtrType& my_ptr_ref = *static_cast<WeakPtrType*>(ptr);
		SharedPtrType& linked_ptr_ref = *static_cast<SharedPtrType*>(address);
		my_ptr_ref = linked_ptr_ref;

		// success
		return Optional<Error>();
	}

	// Returns a set of traits specific for this parameter.
	Traits GetTraits() override
	{
		Traits traits;
		traits.container = Traits::ContainerTraits();
		return traits;
	}

private:
	// SFINAE_IS_POLYMORPHIC and SFINAE_IS_NOT_POLYMORPHIC are temporarily defined
	// here to make short SFINAE argument. MS Visual Studio 2008 and 2010 doesn't
	// support template specialization inside template classes, so the only way to
	// deduce correct type name of the managed value - is to use SFINAE pattern.
#define SFINAE_IS_POLYMORPHIC \
	typename EnableIf<IsBaseOf<Polymorphic, ElementType>::value>::Type* sfinae = nullptr
#define SFINAE_IS_NOT_POLYMORPHIC \
	typename EnableIf<!IsBaseOf<Polymorphic, ElementType>::value>::Type* sfinae = nullptr

	// If managed object has polymorphic type (derived from ut::Polymorphic)
	// then we must extract it's derived type name
	template<typename ElementType>
	inline String GetTypeNameVariant(SFINAE_IS_POLYMORPHIC) const
	{
		const SharedPtrType& ptr_ref = *static_cast<const SharedPtrType*>(ptr);
		if (ptr_ref)
		{
			const DynamicType& dyn_type = ptr_ref->Identify();
			return dyn_type.GetName();
		}
		else
		{
			return Type<void>::Name();
		}
	}

	// If managed object has trivial type (not derived from ut::Polymorphic)
	// then just write it's name
	template<typename ElementType>
	inline String GetTypeNameVariant(SFINAE_IS_NOT_POLYMORPHIC) const
	{
		return BaseParameter::DeduceTypeName<T>();
	}

	// undef macros here
#undef SFINAE_IS_POLYMORPHIC
#undef SFINAE_IS_NOT_POLYMORPHIC

};

//----------------------------------------------------------------------------//
END_NAMESPACE(meta)
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//