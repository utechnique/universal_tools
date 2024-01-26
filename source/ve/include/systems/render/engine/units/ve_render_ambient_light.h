//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "systems/render/engine/ve_render_unit.h"
#include "systems/render/engine/ve_render_resource.h"
#include "systems/render/engine/lighting/ve_light_source.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// ve::render::AmbientLight illuminates an entire scene with the same color.
class AmbientLight : public Unit, public Light
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
	void Reflect(ut::meta::Snapshot& snapshot);

	// Bounds the light's visible influence.
	float attenuation_distance = 1.0e+10f;

	// GPU resources owned by this unit. 
	RcRef<GpuData> data;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//