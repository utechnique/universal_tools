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
#include "pointers/ut_shared_ptr.h"
#include "system/ut_lock.h"
#include "templates/ut_enable_if.h"
#include "templates/ut_is_default_constructible.h"
#include "templates/ut_is_copy_constructible.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ut)
//----------------------------------------------------------------------------//
// Forward declarations.
struct FactoryView;

// ut::Polymorphic is an abstract parent class for the polymorphic types.
// Every serializable polymorphic class must be inherited from this base
// class and needs to implement ut::Polymorphic::Identify() virtual function.
class Polymorphic
{
public:
	virtual const class DynamicType& Identify() const = 0;
	virtual ~Polymorphic() = default;
};

// ut::DynamicType is an abstract parent class to create unknown
// dynamic objects. Object of this class can create objects of
// the derived type without knowing the type explicitly.
class DynamicType : public NonCopyable
{
public:
	// Every dynamic type is identified by it's name
	// and singleton object of it's manager.
	typedef Pair<String, const DynamicType&> Id;

	typedef uptr Handle;

	// Constructor, @Id has to be known at this stage,
	// use ut::Factory<>::Register() to get @Id.
	DynamicType(const Id& type_id) : id(type_id)
	{ }

	// Returns the reference to the corresponding factory view.
	virtual Optional<const FactoryView&> GetFactory() const = 0;

	// Creates an object of the derived type.
	virtual Polymorphic* CreateInstance() const = 0;

	// Copies an object of the derived type.
	virtual Polymorphic* CloneObject(const Polymorphic& copy) const = 0;

	// Returns full identifier of the dynamic type.
	const Id& GetId() const
	{
		return id;
	}

	// Gets dynamic type name.
	const String& GetName() const
	{
		return id.first;
	}

	// Gets dynamic type handle.
	Handle GetHandle() const
	{
		return reinterpret_cast<DynamicType::Handle>(&id.second);
	}

private:
	// Identifier of the dynamic type.
	const Id& id;
};

// Intermediate interface to access factory functions avoiding explicit template
// instantiation.
struct FactoryView
{
	// Searches for the specified type by name.
	//    @param name - name of the type to be found.
	//    @return - dynamic type if it was found, or error otherwise
	virtual Result<const DynamicType&, Error> GetType(const String& name) const = 0;

	// Returns the reference to the registered type by it's index assigned
	// by this factory.
	//    @param index - index of the desired type.
	//    @return - reference to the dynamic type
	virtual const DynamicType& GetTypeByIndex(size_t index) const = 0;

	// Returns the number of types registered in this factory.
	virtual size_t CountTypes() const = 0;
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

	// Returns the reference to the corresponding factory view.
	Optional<const FactoryView&> GetFactory() const override;

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

	// Gets type name.
	static const String& GetName()
	{
		return id.first;
	}

	// Gets type handle.
	static Handle GetHandle()
	{
		return id.second.GetHandle();
	}

	// static (and thus single) identifier of the managed dynamic type.
	static const DynamicType::Id id;

private:
	// Creates instance if type is default-constructible
	template<typename TArg>
	inline typename EnableIf<IsDefaultConstructible<TArg>::value, TArg>::Type*
		CreateInstanceTemplate() const
	{
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
inline static const DynamicType& Identify(const T* t = nullptr)
{
	return PolymorphicType<T>::id.second;
}

// Convenient function to identify the name of a dynamic type.
// Note that you must declare ut::PolymorphicType<YourDynamicType>::id
// previously (just after declaring 'YourDynamicType' class).
template<typename T>
inline static String GetPolymorphicName()
{
	return PolymorphicType<T>::GetName();
}

template<typename T>
inline static DynamicType::Handle GetPolymorphicHandle()
{
	return reinterpret_cast<DynamicType::Handle>(&PolymorphicType<T>::id.second);
}

// Returns reference to the mutex that is represented as Meyers' singleton
// to protect polymorphic registration system (so that only one type could
// be registered at a time).
static Mutex& GetPolymorphicFactoryMutex()
{
	static Mutex mutex;
	return mutex;
}

// ut::Factory is a template class to implement factory pattern for the
// dynamic types. Every serializable polymorphic type has it's own factory.
// Derived types can be registered with this factory, so that they could
// be serialized with the pointers of the base type. Note, that ut::Factory
// has only static methods, thus no object has to be created before using it.
template<typename Base>
class Factory
{
	// All factories have mutual access to each other.
	template<typename> friend class Factory;

	// Polymorphic view of this factory.
	struct View : public FactoryView
	{
		// Searches for the specified type by name.
		//    @param name - name of the type to be found.
		//    @return - dynamic type if it was found, or error otherwise
		Result<const DynamicType&, Error> GetType(const String& name) const override
		{
			return Factory::GetType(name);
		}

		// Returns the reference to the registered type by it's index assigned
		// by this factory.
		//    @param index - index of the desired type.
		//    @return - reference to the dynamic type
		const DynamicType& GetTypeByIndex(size_t index) const override
		{
			return Factory::GetTypeByIndex(index);
		}

		// Returns the number of types registered in this factory.
		size_t CountTypes() const override
		{
			return Factory::CountTypes();
		}
	};

public:
	// Registers dynamic type @Derived, see ut::PolymorphicType for examples.
	//    @param name - desired name for the specified type.
	//    @return - identifier of the registered type.
	template <typename Derived>
	static DynamicType::Id Register(const String& name)
	{
		// protect function with a mutex, so that only
		// one type could be registered at a time
		ScopeLock lock(GetPolymorphicFactoryMutex());

		// check if type is already registered
		Optional<DynamicTypePtr&> result = GetMap().Find(name);

		// if type is registered - just return it's id
		if (result)
		{
			DynamicTypePtr& dyn_type = result.Get();
			return dyn_type->GetId();
		}
		else // otherwise - register a new one
		{
			// create shared pointer to the new polymorphic type
			DynamicType* type = new PolymorphicType<Derived>;
			DynamicTypePtr ptr(type);

			// register type in current factory
			Register(name, ptr);

			// register type in it's own factory
			Factory<Derived>::Register(name, ptr);

			// import children of the registered type
			Import<Derived>();

			// add callback to the factory of the registered type
			Factory<Derived>::GetCallbacks().Add(Factory<Base>::Register);

			// construct complete id object and return it
			return DynamicType::Id(name, *type);
		}
	}

	// Searches for the specified type by name.
	//    @param name - name of the type to be found.
	//    @return - dynamic type if it was found, or error otherwise
	static Result<const DynamicType&, Error> GetType(const String& name)
	{
		Optional<DynamicTypePtr&> result = GetMap().Find(name);
		if (!result)
		{
			return MakeError(error::not_found);
		}
		DynamicTypePtr& dyn_type = result.Get();
		return dyn_type.GetRef();
	}

	// Selects all objects of the specified derived type from the array of
	// pointers of the base type. Selected items are appended to the end
	// of the destination array.
	//    @param src - reference to the array of pointers of the base type.
	//    @param dst - referenct to the destination array of references of
	//                 the derived type.
	template<typename Derived,
	         typename SrcArray = Array< UniquePtr<Base> >,
	         typename DstArray = Array< Ref<Derived> > >
	static void Select(SrcArray& src, DstArray& dst)
	{
		const DynamicType::Handle dst_handle = PolymorphicType<Derived>::GetHandle();
		const size_t src_count = src.GetNum();
		for (size_t i = 0; i < src_count; i++)
		{
			const DynamicType& src_type = src[i]->Identify();
			if (src_type.GetHandle() == dst_handle)
			{
				dst.Add(static_cast<Derived&>(src[i].GetRef()));
			}
		}
	}

	// Returns the reference to the registered type by it's index assigned
	// by this factory.
	//    @param index - index of the desired type.
	//    @return - reference to the dynamic type
	static const DynamicType& GetTypeByIndex(size_t index)
	{
		return GetArray()[index];
	}

	// Returns the number of types registered in this factory.
	static size_t CountTypes()
	{
		return GetArray().GetNum();
	}

	// Returns the polymorphic view of this factory.
	static const View& GetView()
	{
		
		static const View view;
		return view;
	}

private:
	// There is only one instance per dynamic type, and it's shared between
	// different factories. Thread safety is disabled for this shared pointer
	// because only one type can be registered simultaneously.
	typedef SharedPtr<DynamicType, thread_safety::off> DynamicTypePtr;

	// Every type must be registered in all factories that are parents of the
	// current one. That is done by calling a registration callback of the
	// parent factory after own registration is done.
	typedef void(*RegisterCallback)(const String&, const DynamicTypePtr& ptr);

	// AVL tree is used as a container for the (type/name) map as it
	// provides good search performance.
	typedef AVLTree<String, DynamicTypePtr> Map;

	// Adds provided type to the map and asks parents to register this type too.
	static void Register(const String& name, const DynamicTypePtr& ptr)
	{
		Optional<DynamicTypePtr&> result = GetMap().Find(name);
		if (!result)
		{
			// add shared pointer to the own map at first
			bool result = GetMap().Insert(name, ptr) && GetArray().Add(ptr.GetRef());
			UT_ASSERT(result);

			// then let parent factories register this type too
			Array<RegisterCallback>& callbacks = GetCallbacks();
			for (size_t i = 0; i < callbacks.GetNum(); i++)
			{
				callbacks[i](name, ptr);
			}
		}
	}

	// Imports all types that are already registered in @Derived factory.
	template <typename Derived>
	static void Import()
	{
		// note that Register() function below registers a type into all parent factories too
		Map& map = Factory<Derived>::GetMap();
		Map::ConstIterator it;
		for (it = map.Begin(iterator::first); it != map.End(iterator::last); ++it)
		{
			const Map::Node& node = *it;
			Register(node.GetKey(), node.value);
		}
	}

	// Type names and type proxies are stored in a special map, so that
	// appropriate type could be found by name in relatively small time.
	static Map& GetMap()
	{
		static Map map;
		return map;
	}

	// References to the dynamic types are dublicated to the linear array
	// to perform convenient iteration.
	static Array< ConstRef<DynamicType> >& GetArray()
	{
		static Array< ConstRef<DynamicType> > types;
		return types;
	}

	// Registration callbacks of the parent factories.
	static Array<RegisterCallback>& GetCallbacks()
	{
		static Array<RegisterCallback> callbacks;
		return callbacks;
	}
};

template<typename Base>
Optional<const FactoryView&> PolymorphicType<Base>::GetFactory() const
{
	return Factory<Base>::GetView();
}

// Convenient macro for registering polymorphic types.
#define UT_REGISTER_TYPE(__factory, __type, __name) \
	template<> const ut::DynamicType::Id ut::PolymorphicType<__type>::id = \
	ut::Factory<__factory>::Register<__type>(__name);

//----------------------------------------------------------------------------//
END_NAMESPACE(ut)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//