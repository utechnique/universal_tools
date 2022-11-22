//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "common/ut_common.h"
#include "templates/ut_function.h"
#include "templates/ut_is_same.h"
#include "system/ut_endianness.h"
#include "text/ut_text_node.h"
#include "meta/ut_reflective.h"
#include "meta/ut_polymorphic.h"
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
	// Set of traits specific for this parameter type.
	struct Traits
	{
		struct ContainerTraits
		{
			// Dont forget to check if callback is available by calling
			// ut::Function<>::IsValid() function.
			struct Callbacks
			{
				Function<void()> reset;
				Function<void(ut::Optional<const DynamicType&>)> create;
				Function<const FactoryView&()> get_factory;
				Function<void()> push_back;
				Function<void(void* element_addr)> remove_element;
			};

			// Indicates if the managed object is a polymorphic object.
			bool managed_type_is_polymorphic = false;

			// Indicates if this container can handle multiple elements.
			bool contains_multiple_elements = false;

			// Set of callbacks available for the container.
			Callbacks callbacks;
		};

		// A set of container traits. This member is empty if parameter
		// is not a container.
		Optional<ContainerTraits> container;
	};

	// Constructor.
	//    @param p - pointer to the serializable data
	BaseParameter(void* p);

	// Virtual destructor.
	virtual ~BaseParameter() = default;

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

	// Links internal value with provided object (that is specified by address).
	// Every parameter calling Controller::WriteLink() and
	// Controller::ReadLink() must override this function so that
	// linker could operate with it.
	//    @param address - address of the object to link with.
	//    @return - ut::Error if encountered an error
	virtual Optional<Error> Link(void* address);

	// Returns a set of traits specific for this parameter.
	virtual Traits GetTraits();

	// Returns an address of the managed object.
	void* GetAddress();

	// Returns a constant pointer to the managed object.
	const void* GetAddress() const;

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