//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "systems/render/ve_render_unit.h"
#include "systems/render/ve_render_resource.h"
#include "systems/render/engine/lighting/ve_light_source.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// ve::render::PointLight works much like a real world light bulb, emitting
// light in all directions from the light bulb's tungsten filament.
class PointLight : public Unit, public Light
{
public:
	// Per-frame gpu data.
	struct FrameData
	{
		Buffer uniform_buffer;
	};

	// Contains all rendering resources owned by this unit. 
	struct GpuData : public Resource
	{
		const ut::DynamicType& Identify() const { return ut::Identify(this); }
		ut::Array<FrameData> frames;
	};

	// Identify() method must be implemented for the polymorphic types.
	const ut::DynamicType& Identify() const override;

	// Registers light source info into the reflection tree.
	//    @param snapshot - reference to the reflection tree.
	void Reflect(ut::meta::Snapshot& snapshot) override;

	// Bounds the light's visible influence.
	float attenuation_distance = 30.0f;

	// Radius of light source shape.
	float shape_radius = 0.0f;

	// Length of light source shape.
	float shape_length = 0.0f;

	// GPU resources owned by this unit. 
	RcRef<GpuData> data;

	// This vector is the original shape direction (before transformation)
	// in the case if this light source represents a tube (@shape_length > 0). 
	static const ut::Vector<3> skTubeDirection;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//