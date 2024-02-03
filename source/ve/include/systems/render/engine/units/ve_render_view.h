//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "systems/render/engine/ve_render_unit.h"
#include "systems/render/engine/ve_render_resource.h"
#include "systems/render/engine/post_process/ve_post_process_view_data.h"
#include "systems/render/engine/lighting/ve_lighting_view_data.h"
#include "systems/render/engine/ve_render_hitmask.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// ve::render::View is a render unit representing a render surface. It contains
// view and projection matrices, a set of render targets and other things
// to draw environment from custom position.
class View : public Unit
{
public:
	// Available modes.
	enum Mode
	{
		mode_complete,
		mode_diffuse,
		mode_normal,
	};

	// Per-view uniform buffer representation.
	struct Uniforms
	{
		alignas(16) ut::Matrix<4, 4> view_proj;
		alignas(16) ut::Matrix<4, 4> view_proj_inversed;
		alignas(16) ut::Vector<4> camera_position;
	};

	// Represents a set of buffers and targets
	// to render the environment.
	struct SceneBuffer
	{
		ut::Array<Buffer> view_ub; // 6 for a cubemap, 1 otherwise
		Target depth_stencil;
		lighting::ViewData lighting;
	};

	// Per-frame gpu data.
	struct FrameData
	{
		SceneBuffer environment_map;
		SceneBuffer scene;
		lighting::IBL::ViewData ibl;
		HitMask::ViewData hitmask;
		postprocess::ViewData post_process;
		ut::Optional<Image&> final_img;
	};

	// Gpu resources.
	struct GpuData : public Resource
	{
		const ut::DynamicType& Identify() const { return ut::Identify(this); }
		ut::Array<FrameData> frames;
	};

	// Identify() method must be implemented for the polymorphic types.
	const ut::DynamicType& Identify() const;

	// Registers this view unit into the reflection tree.
	//    @param snapshot - reference to the reflection tree.
	void Reflect(ut::meta::Snapshot& snapshot);

	// Current mode.
	Mode mode = mode_complete;

	// View matrices.
	ut::Matrix<4, 4, float> view_matrix;
	ut::Matrix<4, 4, float> proj_matrix;

	// Position of the camera associated with this view.
	ut::Vector<3> camera_position;

	// Final image format
	pixel::Format format = pixel::r8g8b8a8_srgb;

	// View metrics in pixels.
	ut::uint32 width = 640;
	ut::uint32 height = 480;

	// near and far frustum planes
	float znear = 0.1f;
	float zfar = 1e+5f;

	// The identifier of the UI widget associated with this view.
	ui::Viewport::Id viewport_id = 0;

	// Indicates if the hitmak pass must be performed for this view on the next
	// frame.
	bool draw_hitmask = false;

	// Indicates if this viewport will be processed by the render engine.
	bool is_active = true;

	// Hitmask pixel data.
	ut::Array<ve::Entity::Id> hitmask;

	// GPU resources.
	RcRef<GpuData> data;

	// Timer.
	double frame_time_ms = 0.0;
	double total_time_ms = 0.0;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//