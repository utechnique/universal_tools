//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "common/ut_common.h"
#include "system/ut_endianness.h"
#include "text/ut_text_node.h"
#include "meta/ut_reflective.h"
#include "templates/ut_enable_if.h"
#include "templates/ut_is_base_of.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
START_NAMESPACE(meta)
//----------------------------------------------------------------------------//
// Forward declarations.
class Controller;

//----------------------------------------------------------------------------//
// ut::meta::BaseParameter is abstract class to serialize custom data. Every
// serializable parameter must be derived from this interface and implement
// two functions: ut::BaseParameter::Save() and ut::BaseParameter::Load().
class BaseParameter : public Reflective
{
public:
	// Constructor
	//    @param p - pointer to the serializable data
	BaseParameter(void* p);

	// Registers children into reflection tree.
	//    @param snapshot - reference to the reflection tree
	virtual void Reflect(Snapshot& snapshot);

	// Returns name of the managed type. All derived classes must
	// override this member function. Good practice is to override it with
	// a call to BaseParameter::DeduceTypeName() template function.
	virtual String GetTypeName() const = 0;

	// Serializes managed object.
	//    @param controller - meta controller that helps to write data
	//    @return - ut::Error if encountered an error
	virtual Optional<Error> Save(Controller& controller);

	// Deserializes managed object.
	//    @param controller - meta controller that helps to read data
	//    @return - ut::Error if encountered an error
	virtual Optional<Error> Load(Controller& controller);

	// Links internal value with provided parameter.
	// Every parameter calling Controller::WriteLink() and
	// Controller::ReadLink() must override this function so that
	// linker could operate with it.
	//    @param parameter - parameter to link with.
	//    @return - ut::Error if encountered an error
	virtual Optional<Error> Link(BaseParameter& parameter);

	// Returns 'true' if current parameter is a container
	// for multiple uniform objects.
	virtual bool IsArray() const;

	// Returns an address of the managed object.
	void* GetAddress();

protected:
	// Returns a name of the provided (as a template argument) type.
	// Complements ut::Type template with "reflective" type name.
	template<typename ElementType>
	inline static String DeduceTypeName()
	{
		return GetTypeNameVariant<ElementType>();
	}

	// pointer to the managed object
	void* ptr;

private:
	// SFINAE_IS_REFLECTIVE and SFINAE_IS_NOT_REFLECTIVE are temporarily defined here to make
	// short SFINAE argument. Default template parameters in member functions are allowed only
	// since C++11 for visual studio, so we need to apply SFINAE pattern via default function
	// argument to deduce the correct way to determine appropriate type name of the managed
	// object. Both macros are used to fork detecting a name of the type. All reflective
	// parameters are expected to have "reflective" type name, and non-reflective parameters
	// must have a name of the managed type.
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
	// then type name is a name of the intrinsic type
	template<typename ElementType>
	inline static String GetTypeNameVariant(SFINAE_IS_NOT_REFLECTIVE)
	{
		return Type<ElementType>::Name();
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