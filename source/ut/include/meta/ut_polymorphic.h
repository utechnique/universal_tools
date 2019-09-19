//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "common/ut_common.h"
#include "containers/ut_array.h"
#include "containers/ut_avltree.h"
#include "containers/ut_pair.h"
#include "containers/ut_singleton.h"
#include "pointers/ut_unique_ptr.h"
#include "templates/ut_enable_if.h"
#include "templates/ut_is_default_constructible.h"
#include "templates/ut_is_copy_constructible.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
//----------------------------------------------------------------------------//
// ut::Polymorphic is an abstract parent class for the polymorphic types.
// Every serializable polymorphic class must be inherited from this base
// class and needs to implement ut::Polymorphic::Identify() virtual function.
class Polymorphic
{
public:
	virtual const class DynamicType& Identify() const = 0;
};

// ut::DynamicType is an abstract parent class to create unknown
// dynamic objects. Object of this class can create objects of
// the derived type without knowing the type explicitly.
class DynamicType : public NonCopyable
{
public:
	// Every dynamic type is identified by it's name
	// and singleton object of it's manager.
	typedef Pair< String, ConstRef<DynamicType> > Id;

	// Constructor, @Id has to be known at this stage,
	// use ut::Factory<>::Register() to get @Id.
	DynamicType(const Id& type_id) : id(type_id)
	{ }

	// Creates an object of the derived type.
	virtual Polymorphic* CreateInstance() const = 0;

	// Copies an object of the derived type.
	virtual Polymorphic* CloneObject(const Polymorphic& copy) const = 0;

	// Gets dynamic type name.
	const String& GetName() const
	{
		return id.first;
	}

private:
	// Identifier of the dynamic type.
	const Id& id;
};

// ut::PolymorphicType is a template class to create dynamic objects of
// the managed type. ut::PolymorphicType is inherited from ut::DynamicType,
// and handles static member ut::PolymorphicType::id, while it's parent -
// ut::DynamicType has only non-static reference to this member.
// Every polymorphic type (inherited from ut::Polymorphic) has to initialize
// this identifier by calling Register() method of the corresponding factory.
// Example:
// const ut::DynamicType::Id ut::PolymorphicType<YourBaseType>::id =
//                           ut::Factory<YourBaseType>::Register<YourBaseType>("base");
// const ut::DynamicType::Id ut::PolymorphicType<YourDerivedType>::id =
//                           ut::Factory<YourBaseType>::Register<YourDerivedType>("derived");
template <typename T>
class PolymorphicType : public DynamicType
{
public:
	// Constructor, just passes identifier to the base class.
	PolymorphicType() : DynamicType(id)
	{ }

	// Creates a new object of the managed typed.
	Polymorphic* CreateInstance() const
	{
		return CreateInstanceTemplate<T>();
	}

	// Copies an object of the derived type.
	Polymorphic* CloneObject(const Polymorphic& copy) const
	{
		return CopyObjectTemplate<T>(copy);
	}

	// static (and thus single) identifier of the managed dynamic type.
	static const DynamicType::Id id;

private:
	// Creates instance if type is default-constructible
	template<typename TArg>
	inline typename EnableIf<IsDefaultConstructible<TArg>::value, TArg>::Type*
		CreateInstanceTemplate() const
	{
		// If you have error here in old compiler versions (compiler is trying to
		// instantiate non-existent default constructor) - make sure you put
		// UT_NO_DEFAULT_CONSTRUCTOR macro inside 'public:' section of your
		// class declaration.
		return new T;
	}

	// Returns null if type is not default-constructible
	template<typename TArg>
	inline typename EnableIf<!IsDefaultConstructible<TArg>::value, TArg>::Type*
		CreateInstanceTemplate() const
	{
		return nullptr;
	}

	// Clones object if type is copy-constructible
	template<typename TArg>
	inline typename EnableIf<IsCopyConstructible<TArg>::value, TArg>::Type*
		CopyObjectTemplate(const Polymorphic& copy) const
	{
		return new T(static_cast<const T&>(copy));
	}

	// Returns null if type is not copy-constructible
	template<typename TArg>
	inline typename EnableIf<!IsCopyConstructible<TArg>::value, TArg>::Type*
		CopyObjectTemplate(const Polymorphic& copy) const
	{
		return nullptr;
	}
};

// Convenient function to identify dynamic types.
// Use it with ut::Polymorphic::Identify() virtual function.
// Example:
//     const ut::DynamicType& YourDynamicType::Identify() const
//     {
//         return ut::Identify(this);
//     }
// Note that you must declare ut::PolymorphicType<YourDynamicType>::id
// previously (just after declaring 'YourDynamicType' class).
template<typename T>
inline static const DynamicType& Identify(const T* t)
{
	return PolymorphicType<T>::id.second;
}

// Convenient function to identify the name of a dynamic type.
// Note that you must declare ut::PolymorphicType<YourDynamicType>::id
// previously (just after declaring 'YourDynamicType' class).
template<typename T>
inline static String GetPolymorphicName()
{
	return PolymorphicType<T>::id.first;
}

// ut::Factory is a template class to implement factory pattern for the
// dynamic types. Every serializable polymorphic type has it's own factory.
// Derived types can be registered with this factory, so that they could
// be serialized with the pointers of the base type. Note, that ut::Factory
// has only static methods, thus no object has to be created before using it.
template<typename Base>
class Factory
{
public:
	// Registers dynami type @Derived, see ut::PolymorphicType for examples.
	//    @param name - desired name for the specified type.
	//    @return - identifier of the registered type.
	template <typename Derivered>
	static DynamicType::Id Register(const String& name)
	{
		DynamicType* type = new PolymorphicType<Derivered>;
		UniquePtr<DynamicType> ptr(type);
		bool result = GetMap().Insert(name, Move(ptr));
		UT_ASSERT(result);
		return DynamicType::Id(name, *type);
	}

	// Searches for the specified type by name.
	//    @param name - name of the type to be found.
	//    @return - dynamic type if it was found, or error otherwise
	static Result<ConstRef<DynamicType>, Error> GetType(const String& name)
	{
		Result< Ref<UniquePtr<DynamicType> >, Error> result = GetMap().Find(name);
		if (!result)
		{
			return MakeError(result.GetAlt());
		}
		UniquePtr<DynamicType>& dyn_type = result.GetResult();
		return ConstRef<DynamicType>(dyn_type.GetRef());
	}

private:
	// Declare a separate type for the map, otherwise
	// Singleton<Map> will be the same for all factory types.
	template <typename T0, typename T1, typename T2>
	class MyTree : public AVLTree<T0, T1>
	{ };

	// Type names and managing objects are stored in the special map, so that
	// appropriate type could be found by name in relatively small time.
	typedef MyTree<String, UniquePtr<DynamicType>, Base> Map;
	static Map& GetMap()
	{
		return Singleton<Map>::Instance();
	}
};

// Convenient macro for registering polymorphic types.
#define UT_REGISTER_TYPE(__factory, __type, __name) \
	template<> const ut::DynamicType::Id ut::PolymorphicType<__type>::id = \
	ut::Factory<__factory>::Register<__type>(__name);

//----------------------------------------------------------------------------//
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//