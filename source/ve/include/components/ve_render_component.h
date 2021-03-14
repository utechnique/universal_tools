//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "ve_component.h"
#include "systems/render/ve_render_unit.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
//----------------------------------------------------------------------------//
// Render component contains an array of render units. Each unit represents
// something affecting rendering: what, where, or how to render.
class RenderComponent : public Component
{
public:
	// Explicitly declare defaulted constructors and move operator.
	RenderComponent() = default;
	RenderComponent(RenderComponent&&) = default;
	RenderComponent& operator =(RenderComponent&&) = default;

	// Copying is prohibited.
	RenderComponent(const RenderComponent&) = delete;
	RenderComponent& operator =(const RenderComponent&) = delete;

	// Meta routine.
	const ut::DynamicType& Identify() const;
	void Reflect(ut::meta::Snapshot& snapshot);

	// Render units.
	ut::Array< ut::UniquePtr<render::Unit> > units;

	// This cache is used to track if unit array has changed
	// since the previous frame.
	ut::Array<render::Unit*> cache;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//