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
// SFINAE_IS_POLYMORPHIC and SFINAE_IS_NOT_POLYMORPHIC are temporarily defined
// here to make short SFINAE argument. MS Visual Studio 2008 and 2010 doesn't
// support template specialization inside template classes, so the only way to
// deduce correct type name of the managed value - is to use SFINAE pattern.
#define SFINAE_IS_POLYMORPHIC \
	typename EnableIf<IsBaseOf<Polymorphic, ElementType>::value>::Type* sfinae = nullptr
#define SFINAE_IS_NOT_POLYMORPHIC \
	typename EnableIf<!IsBaseOf<Polymorphic, ElementType>::value>::Type* sfinae = nullptr

//----------------------------------------------------------------------------//
// ut::meta::SharedPtrHolder is a template class that completes type-erasure
// idiom for the shared pointers by inheriting ut::meta::SharedPtrHolderBase
// class. Thus you can use pointers to SharedPtrHolderBase class to be able
// to save/load shared parameters without knowing the exact type.
template<typename T, thread_safety::Mode mode, typename Deleter>
class SharedPtrHolder : public SharedPtrHolderBase
{
	typedef SharedPtr<T, mode, Deleter> SharedPtrType;
public:
	// Constructor, passes an address of the managed smart pointer to the
	// constructior of the parent class.
	SharedPtrHolder(const SharedPtrType& in_ptr,
	                String in_type_name) : SharedPtrHolderBase(&ptr)
	                                     , ptr(in_ptr)
	                                     , type_name(Move(in_type_name))
	{ }

	// Copy constructor, pointer must be reset.
	SharedPtrHolder(const SharedPtrHolder& copy) : SharedPtrHolderBase(&ptr)
	                                             , ptr(copy.ptr)
	                                             , type_name(copy.type_name)
	{ }

	// Move constructor, pointer must be reset.
	SharedPtrHolder(SharedPtrHolder&& rval) noexcept : SharedPtrHolderBase(&ptr)
	                                                 , ptr(Move(rval.ptr))
	                                                 , type_name(Move(rval.type_name))
	{ }

	// Assignment operator, pointer must be reset.
	SharedPtrHolder& operator = (const SharedPtrHolder& copy)
	{
		ptr = copy.ptr;
		type_name = copy.type_name;
		address = &ptr;
		return *this;
	}

	// Move operator, pointer must be reset.
	SharedPtrHolder& operator = (SharedPtrHolder&& rval) noexcept
	{
		ptr = Move(rval.ptr);
		type_name = Move(rval.type_name);
		address = &ptr;
		return *this;
	}

	// Serializes managed object using provided controller.
	//    @param controller - reference to the controller to save the
	//                        managed object.
	//    @param name - name of the parameter.
	//    @return - ut::Error if failed.
	Optional<Error> Save(Controller& controller, const String& name)
	{
		Snapshot snapshot = Snapshot::Capture(ptr.GetRef(), name, controller.GetInfo());
		return controller.WriteNode(snapshot, false);
	}

	// Deserializes managed object using provided controller.
	//    @param controller - reference to the controller to load the
	//                        managed object.
	//    @param name - name of the parameter.
	//    @param type_name - name of the parameter's type.
	//    @return - ut::Error if failed.
	Optional<Error> Load(Controller& controller, const String& name)
	{
		// create a new instance
		Result<SharedPtrType, Error> instance = CreateNewInstanceVariant<T>(type_name);
		if (!instance)
		{
			return instance.MoveAlt();
		}

		// reset smart pointer
		ptr = instance.Move();

		// reflect data into snapshot
		Snapshot snapshot = Snapshot::Capture(ptr.GetRef(), name, controller.GetInfo());

		// deserialize captured node
		Result<Controller::Uniform, Error> read_result = controller.ReadNode(snapshot,
		                                                                     false, false);
		if (!read_result)
		{
			return read_result.MoveAlt();
		}

		// success
		return Optional<Error>();
	}

private:
	// If managed object is a custom (not derived from ut::Polymorphic)
	// element - just check static type and create a new inctance
	template<typename ElementType>
	inline Result<SharedPtrType, Error> CreateNewInstanceVariant(const String& new_type_name,
	                                                             SFINAE_IS_NOT_POLYMORPHIC)
	{
		// check static type
		if (type_name != new_type_name)
		{
			return MakeError(error::types_not_match);
		}

		// create instance
		SharedPtrType instance(new T);
		return Move(instance);
	}

	// If managed object is a polymorphic object - then
	// we must load polymorphic name string, and create an
	// object of the corresponding type.
	template<typename ElementType>
	inline Result<SharedPtrType, Error> CreateNewInstanceVariant(const String& new_type_name,
	                                                             SFINAE_IS_POLYMORPHIC)
	{
		// get dynamic type by name
		Result<const DynamicType&, Error> type_result = Factory<T>::GetType(new_type_name);
		if (!type_result)
		{
			return MakeError(type_result.MoveAlt());
		}

		// create a new object
		const DynamicType& dyn_type = type_result.Get();
		SharedPtrType instance(static_cast<T*>(dyn_type.CreateInstance()));
		return Move(instance);
	}

	SharedPtrType ptr;
	const String type_name;
};

//----------------------------------------------------------------------------//
// ut::Parameter<SharedPtr> is a template specialization for shared pointers.
template<typename T, thread_safety::Mode mode, typename Deleter>
class Parameter< SharedPtr<T, mode, Deleter> > : public BaseParameter
{
	typedef SharedPtr<T, mode, Deleter> SharedPtrType;
	typedef SharedPtrHolder<T, mode, Deleter> HolderType;
public:
	// Constructor
	//    @param p - pointer to the managed string
	Parameter(SharedPtrType* p) : BaseParameter(p)
	{ }

	// Returns the name of the managed type
	String GetTypeName() const
	{
		return BaseParameter::DeduceTypeName< SharedPtr<T> >();
	}

	// Serializes managed object.
	//    @param controller - meta controller that helps to write data
	//    @return - ut::Error if encountered an error
	Optional<Error> Save(Controller& controller)
	{
		// write value type name
		SharedPtrType& ptr_ref = *static_cast<SharedPtrType*>(ptr);
		String value_type_name = ptr_ref ? GetTypeNameVariant<T>() : String(Type<void>::Name());
		const Optional<Error> write_error = controller.WriteAttribute(value_type_name, node_names::skValueType);
		if (write_error)
		{
			return write_error;
		}

		// exit if pointer is empty
		if (!ptr_ref)
		{
			return Optional<Error>();
		}

		// register parameter
		SharedPtr<SharedPtrHolderBase> holder(new HolderType(ptr_ref, Move(value_type_name)));
		const Optional<Error> register_error = controller.WriteSharedObject(holder, static_cast<const void*>(ptr_ref.Get()));
		if (register_error)
		{
			return register_error;
		}

		// write id
		return controller.WriteLink(this, ptr_ref.Get());
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
		SharedPtrType& ptr_ref = *static_cast<SharedPtrType*>(ptr);

		// check if serialized pointer is not null
		if (read_type_result.Get() == Type<void>::Name())
		{
			ptr_ref.Reset(); // reset current value
			return Optional<Error>(); // exit, ok
		}

		// read id and link up with the correct object
		SharedPtr<SharedPtrHolderBase> holder(new HolderType(ptr_ref, read_type_result.Move()));
		return controller.ReadSharedLink(this, holder);
	}

	// Links pointer with provided object (that is specified by address).
	//    @param address - address of the shared pointer to link with.
	//    @return - ut::Error if encountered an error
	Optional<Error> Link(void* address)
	{
		SharedPtrType& my_ptr_ref = *static_cast<SharedPtrType*>(ptr);
		SharedPtrType& linked_ptr_ref = *static_cast<SharedPtrType*>(address);
		my_ptr_ref = linked_ptr_ref;

		// success
		return Optional<Error>();
	}

private:
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
};

//----------------------------------------------------------------------------//
END_NAMESPACE(meta)
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
// undef macros here
#undef SFINAE_IS_POLYMORPHIC
#undef SFINAE_IS_NOT_POLYMORPHIC
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//