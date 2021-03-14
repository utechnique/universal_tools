//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "systems/render/ve_render_unit.h"
#include "systems/render/ve_render_api.h"
#include "systems/render/ve_render_resource.h"
#include "systems/render/engine/post_process/ve_post_process_view_data.h"
#include "systems/render/engine/lighting/ve_lighting_view_data.h"
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
	enum Mode
	{
		mode_complete,
		mode_diffuse,
		mode_normal,
	};

	// Explicitly declare defaulted constructors and move operator.
	View() = default;
	View(View&&) = default;
	View& operator =(View&&) = default;

	// Copying is prohibited.
	View(const View&) = delete;
	View& operator =(const View&) = delete;

	const ut::DynamicType& Identify() const;
	void Reflect(ut::meta::Snapshot& snapshot);

	ut::Matrix<4, 4, float> view_matrix;
	ut::Matrix<4, 4, float> proj_matrix;

	Mode mode = mode_complete;

	// final image format
	pixel::Format format = pixel::r8g8b8a8_srgb;

	ut::uint32 width = 640;
	ut::uint32 height = 480;

	ui::Viewport::Id viewport_id = 0;

	bool is_active = true;

	struct Uniforms
	{
		alignas(16) ut::Matrix<4, 4> view_proj;
	};

	struct FrameData
	{
		FrameData(Buffer view_uniform_buffer,
		          Target depth_stencil_buffer,
		          lighting::ViewData lighting_data,
		          postprocess::ViewData post_process_data) : uniform_buffer(ut::Move(view_uniform_buffer))
		                                                   , depth_stencil(ut::Move(depth_stencil_buffer))
		                                                   , lighting(ut::Move(lighting_data))
		                                                   , post_process(ut::Move(post_process_data))                                           
		{}

		Buffer uniform_buffer;
		Target depth_stencil;
		lighting::ViewData lighting;
		postprocess::ViewData post_process;
		ut::Optional<Image&> final_img;
	};

	struct GpuData : public Resource
	{
		GpuData(ut::Array<FrameData> in_frames) : frames(ut::Move(in_frames))
		{}

		const ut::DynamicType& Identify() const override
		{
			return ut::Identify(this);
		}

		ut::Array<FrameData> frames;
	};

	RcRef<GpuData> data;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//