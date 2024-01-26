//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "systems/render/ve_render_api.h"
#include "ve_transform.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// ve::render::Light is a base class for all light source types.
class Light : public ut::meta::Reflective
{
public:
	// All possible source types.
	enum SourceType
	{
		source_directional,
		source_point,
		source_spot,
		source_ambient,
		source_type_count
	};

	// Contents of the uniform buffer (common for all source types).
	struct Uniforms
	{
		ut::Vector<4> position;
		ut::Vector<4> direction; // .w - source shape radius
		ut::Color<4> color; // .w - source shape length
		ut::Vector<4> attenuation; // .x - attenuation distance
		                           // .y - inner cone
		                           // .z - outer cone
		ut::Vector<4> orientation;
	};

	// Contains references to all light source units.
	struct Sources
	{
		ut::Array< ut::Ref<class DirectionalLight> >& directional;
		ut::Array< ut::Ref<class PointLight> >& point;
		ut::Array< ut::Ref<class SpotLight> >& spot;
		ut::Array< ut::Ref<class AmbientLight> >& ambient;
	};

	// Registers light source info into the reflection tree.
	//    @param snapshot - reference to the reflection tree.
	void Reflect(ut::meta::Snapshot& snapshot) override;

	// Direction of the light source is calculated as @rotation * skDirection.
	static ut::Vector<3> GetDirection(const ut::Quaternion<float>& rotation);

	// Returns @rotation * skTubeDirection.
	static ut::Vector<3> GetTubeDirection(const ut::Quaternion<float>& rotation);

	// Color of the light source.
	ut::Color<3> color = ut::Color<3>(1);

	// Intensity of light source (usually multiplied with color in shaders).
	float intensity = 1.0f;

	// Original direction of the light source. Final direction is calculated as
	// skDirection * transform.
	static const ut::Vector<3> skDirection;

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