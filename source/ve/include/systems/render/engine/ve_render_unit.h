//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "ve_transform.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// Render unit represents something affecting rendering: what, where, or
// how to render. Every unit type must be registered using UT_REGISTER_TYPE
// macro and have specialized ve::render::Policy template. Also render unit type
// must be added to the ve::render::EngineUnits list to be used by the render
// engine.
class Unit : public ut::Polymorphic, public ut::meta::Reflective
{
	template<typename UnitContainer> friend struct UnitInitializer;
public:
	// Explicitly declare defaulted constructors and move operator.
	Unit() = default;
	Unit(Unit&&) = default;
	Unit& operator =(Unit&&) = default;

	// Copying is prohibited.
	Unit(const Unit&) = delete;
	Unit& operator =(const Unit&) = delete;

	// Identify() method must be implemented for the polymorphic types.
	virtual const ut::DynamicType& Identify() const = 0;

	// Registers @initialized member.
	virtual void Reflect(ut::meta::Snapshot& snapshot);

	// Abstract classes must have virtual destructor.
	virtual ~Unit() = default;

	// Makes this unit invalid, it will be recreated by associated policy on the
	// next frame.
	void Invalidate();

	// Returns true if this unit was fully initialized by the render engine and
	// can parcitipate in rendering.
	bool IsInitialized() const
	{
		return initialized;
	}

	// Local transform in the entity-space.
	Transform local_trasform;

	// Don't use, for internal use only.
	Transform world_transform;
	ut::Matrix<4, 4> world_matrix;

private:
	// Every unit is created with "initialized" member set to 'false', and only
	// a ve::render::UnitManager object can set it to 'true' by calling
	// Initialize() method of the corresponding policy.
	bool initialized = false;
};

//----------------------------------------------------------------------------//
// Every unit type must have it's own specialized policy template. A policy
// describes how a unit is initialized and rendered.
// All policies must have:
//    1) A constructor accepting three parameters - 1 ve::render::Toolset&
//                                                  2 ve::render::UnitSelector&
//                                                  3 ve::render::Policies&
//
//    2) "void Initialize(Unit& unit)" member function where "Unit" is a type
//        of the specialized unit. Note that this function must be thread-safe
//        and it can be called from the outside of the rendering thread.
template<class UnitType> struct Policy;

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//