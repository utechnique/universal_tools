//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "ve_component.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
//----------------------------------------------------------------------------//
// ve::Entity is a class representing indivisible entity containing different
// components that model it's behaviour.
class Entity
{
public:
	// Explicitly declare defaulted constructors and move operator.
	Entity() = default;
	Entity(Entity&&) = default;
	Entity& operator =(Entity&&) = default;

	// Copying is prohibited
	Entity(const Entity&) = delete;
	Entity& operator =(const Entity&) = delete;

	// Type of the identifier that can be associated with entity.
	typedef ut::uint32 Id;

	// Returns the number of components.
	size_t GetComponentCount() const
	{
		return components.GetNum();
	}

	// Returns a reference to the component with the specified id.
	const Component& GetComponentByIndex(size_t id) const
	{
		return components[id].GetRef();
	}

	// Returns a reference to the component with the specified id.
	Component& GetComponentByIndex(size_t id)
	{
		return components[id].GetRef();
	}

	// Returns a reference to the component with the specified id.
	ut::Optional<const Component&> GetComponentByType(ut::DynamicType::Handle type) const
	{
		const ut::Optional<const ut::Ref<Component>&> find_result = cache.Find(type);
		return find_result ? find_result->Get() : ut::Optional<const Component&>();
	}

	// Returns a reference to the component with the specified id.
	ut::Optional<Component&> GetComponentByType(ut::DynamicType::Handle type)
	{
		ut::Optional<ut::Ref<Component>&> find_result = cache.Find(type);
		return find_result ? find_result->Get() : ut::Optional<Component&>();
	}

	// Removes a component of the specified type from the entity.
	//    @return - optional ut::Error if failed to remove a component.
	template<typename ComponentType>
	ut::Optional<ut::Error> RemoveComponent()
	{
		return RemoveComponent(ut::GetPolymorphicHandle<ComponentType>());
	}

	// Returns a reference to the component of the specified type.
	//    @return - reference to the component or nothing if
	//              specified component is absent.
	template<typename ComponentType>
	ut::Optional<ComponentType&> GetComponent()
	{
		const ut::DynamicType::Handle type_handle = ut::GetPolymorphicHandle<ComponentType>();
		ut::Optional<ut::Ref<Component>&> find_result = cache.Find(type_handle);
		if (!find_result)
		{
			return ut::Optional<ComponentType&>();
		}

		Component& ref = find_result.Get();
		return ut::Optional<ComponentType&>(static_cast<ComponentType&>(ref));
	}

	// Adds provided component to the entity.
	//    @param component - unique pointer to the new component.
	//    @param overwrite - flag that controls whether component must be
	//                       overwritten in case if a component of such
	//                       type already exists.
	//    @return - optional ut::Error if failed to add @component.
	ut::Optional<ut::Error> AddComponent(ut::UniquePtr<Component> component,
	                                     bool overwrite = false);

	// Removes a component of the specified type from the entity.
	//    @param component_type - handle to the type of the component
	//                            to be deleted.
	//    @return - optional ut::Error if failed to remove a component.
	ut::Optional<ut::Error> RemoveComponent(const ut::DynamicType::Handle component_type);

private:
	ut::Array< ut::UniquePtr<Component> > components;
	ut::AVLTree< ut::DynamicType::Handle, ut::Ref<Component> > cache;
};
//----------------------------------------------------------------------------//
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//