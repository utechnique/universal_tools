//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "meta/ut_meta_base_parameter.h"
#include "meta/ut_meta_controller.h"
#include "templates/ut_enable_if.h"
#include "templates/ut_is_base_of.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
START_NAMESPACE(meta)
//----------------------------------------------------------------------------//
// ut::meta::Parameter without specialization is a template parameter for
// fundamental types and simple structures (containing only simple types) or
// classes inherited from ut::meta::Reflective.
template<typename T>
class Parameter : public BaseParameter
{
public:
	// Constructor
	//    @param p - pointer to the managed object
	Parameter(T* p) : BaseParameter(p)
	{ }

	// Returns the name of the managed type
	String GetTypeName() const
	{
		return GetTypeNameVariant<T>();
	}

	// Registers children into reflection tree.
	//    @param snapshot - reference to the reflection tree
	void Reflect(Snapshot& snapshot)
	{
		ReflectVariant<T>(snapshot);
	}

	// Serializes managed object.
	//    @param controller - meta controller that helps to write data
	//    @return - ut::Error if encountered an error
	Optional<Error> Save(Controller& controller)
	{
		return SaveVariant<T>(controller);
	}

	// Deserializes managed object.
	//    @param controller - meta controller that helps to read data
	//    @return - ut::Error if encountered an error
	Optional<Error> Load(Controller& controller)
	{
		return LoadVariant<T>(controller);
	}

private:
	// SFINAE_IS_REFLECTIVE and SFINAE_IS_NOT_REFLECTIVE are temporarily defined
	// here to make short SFINAE parameter. Default template parameters in member
	// functions are allowed only since C++11 for visual studio, so we need to apply
	// SFINAE pattern via default function argument to deduce the correct way to
	// save/load appropriate parameter.
#define SFINAE_IS_REFLECTIVE \
	typename EnableIf<IsBaseOf<Reflective, ElementType>::value>::Type* sfinae = nullptr
#define SFINAE_IS_NOT_REFLECTIVE \
	typename EnableIf<!IsBaseOf<Reflective, ElementType>::value>::Type* sfinae = nullptr

	// If managed object is a reflective node (derived from ut::meta::Reflective)
	// then typename is "reflective" for all derived classes
	template<typename ElementType>
	inline static String GetTypeNameVariant(SFINAE_IS_REFLECTIVE)
	{
		return Reflective::skTypeName;
	}

	// If managed object is not a reflective node (not derived from ut::meta::Reflective)
	// then type name is the name of the intrinsic type
	template<typename ElementType>
	inline static String GetTypeNameVariant(SFINAE_IS_NOT_REFLECTIVE)
	{
		return TypeName<T>();
	}

	// If managed object is a reflective node (derived from ut::meta::Reflective)
	// then reflect all of it's contents
	template<typename ElementType>
	inline void ReflectVariant(Snapshot& snapshot, SFINAE_IS_REFLECTIVE)
	{
		T* object_ptr = static_cast<T*>(ptr);
		object_ptr->Reflect(snapshot);
	}

	// If managed object is not a reflective node (not derived from ut::meta::Reflective)
	// then do nothing
	template<typename ElementType>
	inline void ReflectVariant(Snapshot& snapshot, SFINAE_IS_NOT_REFLECTIVE)
	{
		// v o i d
	}

	// If managed object is a reflective object (derived from ut::meta::Reflective),
	// there is nothing to save (all reflected members will be saved separately)
	template<typename ElementType>
	inline Optional<Error> SaveVariant(Controller& controller,
	                                   SFINAE_IS_REFLECTIVE) const
	{
		return Optional<Error>();
	}

	// If managed object is not a reflective object (not derived from ut::meta::Reflective),
	// then just write value to the value node using meta controller
	template<typename ElementType>
	inline Optional<Error> SaveVariant(Controller& controller,
	                                   SFINAE_IS_NOT_REFLECTIVE) const
	{
		return controller.WriteValue<T>(*static_cast<const T*>(ptr));
	}

	// If managed object is a reflective object (derived from ut::meta::Reflective),
	// there is nothing to load (all reflected members will be loaded separately)
	template<typename ElementType>
	inline Optional<Error> LoadVariant(Controller& controller,
	                                   SFINAE_IS_REFLECTIVE) const
	{
		return Optional<Error>();
	}

	// If managed object is not an archive (not derived from ut::Archive)
	// then just read raw byte data of the corresponding type size
	template<typename ElementType>
	inline Optional<Error> LoadVariant(Controller& controller,
	                                   SFINAE_IS_NOT_REFLECTIVE)
	{
		// read value
		Result<T, Error> read_result = controller.ReadValue<T>();
		if (!read_result)
		{
			return read_result.MoveAlt();
		}

		// move result to the managed object
		*static_cast<T*>(ptr) = read_result.MoveResult();

		// success
		return Optional<Error>();
	}

	// undef macros here
#undef SFINAE_IS_REFLECTIVE
#undef SFINAE_IS_NOT_REFLECTIVE
};

//----------------------------------------------------------------------------//
END_NAMESPACE(meta)
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//