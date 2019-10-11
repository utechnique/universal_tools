//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "common/ut_common.h"
#include "pointers/ut_unique_ptr.h"
#include "meta/ut_meta_parameter.h"
#include "meta/ut_polymorphic.h"
#include "meta/ut_meta_node.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
START_NAMESPACE(meta)
//----------------------------------------------------------------------------//
// ut::Parameter<Ptr> is a template specialization for pointers.
template<typename T, typename Deleter>
class Parameter< UniquePtr<T, Deleter> > : public BaseParameter
{
	typedef UniquePtr<T, Deleter> UniquePtrType;
public:
	// Constructor
	//    @param p - pointer to the managed string
	Parameter(UniquePtrType* p) : BaseParameter(p)
	{ }

	// Returns the name of the managed type
	String GetTypeName() const
	{
		return BaseParameter::DeduceTypeName< UniquePtr<T> >();
	}

	// Registers children into reflection tree.
	//    @param snapshot - reference to the reflection tree
	void Reflect(Snapshot& snapshot)
	{
		UniquePtrType* p = static_cast<UniquePtrType*>(ptr);
		if (p->Get())
		{
			snapshot.Add(p->GetRef(), "unique");
		}
	}

	// Serializes managed object.
	//    @param controller - meta controller that helps to write data
	//    @return - ut::Error if encountered an error
	Optional<Error> Save(Controller& controller)
	{
		// write value type name
		const UniquePtrType& ptr_ref = *static_cast<UniquePtrType*>(ptr);
		String value_type_name = ptr_ref ? GetTypeNameVariant<T>() : String(Type<void>::Name());
		return controller.WriteAttribute(value_type_name, node_names::skValueType);
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

		// get a reference to the unique pointer object
		UniquePtrType& ptr_ref = *static_cast<UniquePtrType*>(ptr);

		// check if serialized pointer is not null
		if (read_type_result.GetResult() == Type<void>::Name())
		{
			ptr_ref.Delete(); // reset current value
			return Optional<Error>(); // exit, ok
		}

		// create new instance
		Result<UniquePtrType, Error> create_result = CreateNewInstanceVariant<T>(read_type_result.GetResult());
		if (create_result)
		{
			ptr_ref = Move(create_result.MoveResult());
		}
		else
		{
			return create_result.MoveAlt();
		}

		// success
		return Optional<Error>();
	}

private:
	// SFINAE_IS_POLYMORPHIC and SFINAE_IS_NOT_POLYMORPHIC are temporarily defined here to make
	// short SFINAE argument. MS Visual Studio 2008 and 2010 doesn't support template specialization
	// inside template classes, so the only way to deduce correct type name of the managed value - is
	// to use SFINAE pattern (feature).
#define SFINAE_IS_POLYMORPHIC \
	typename EnableIf<IsBaseOf<Polymorphic, ElementType>::value>::Type* sfinae = nullptr
#define SFINAE_IS_NOT_POLYMORPHIC \
	typename EnableIf<!IsBaseOf<Polymorphic, ElementType>::value>::Type* sfinae = nullptr

	// If managed object has polymorphic type (derived from ut::Polymorphic)
	// then we must extract it's derived type name
	template<typename ElementType>
	inline String GetTypeNameVariant(SFINAE_IS_POLYMORPHIC) const
	{
		const UniquePtr<ElementType>& ptr_ref = *static_cast<UniquePtr<ElementType>*>(ptr);
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

	// If managed object is a custom (not derived from ut::Polymorphic)
	// element - just check static type and create a new inctance
	template<typename ElementType>
	inline Result<UniquePtrType, Error> CreateNewInstanceVariant(const String& type_name,
	                                                             SFINAE_IS_NOT_POLYMORPHIC)
	{
		// check static type
		String current_type_name = GetTypeNameVariant<T>();
		if (current_type_name != type_name)
		{
			return MakeError(error::types_not_match);
		}

		// create instance
		UniquePtrType instance(new T);
		return Move(instance);
	}

	// If managed object is a polymorphic object - then
	// we must load polymorphic name string, and create an
	// object of the corresponding type.
	template<typename ElementType>
	inline Result<UniquePtrType, Error> CreateNewInstanceVariant(const String& type_name,
	                                                             SFINAE_IS_POLYMORPHIC)
	{
		// get dynamic type by name
		Result<ConstRef<DynamicType>, Error> type_result = Factory<T>::GetType(type_name);
		if (!type_result)
		{
			return MakeError(type_result.MoveAlt());
		}

		// create a new object
		const DynamicType& dyn_type = type_result.GetResult();
		UniquePtrType instance(static_cast<T*>(dyn_type.CreateInstance()));
		return Move(instance);
	}

	// undef macros here
#undef SFINAE_IS_POLYMORPHIC
#undef SFINAE_IS_NOT_POLYMORPHIC

	// name of the ut::UniquePtr type
	static const char* skTypeName;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(meta)
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//