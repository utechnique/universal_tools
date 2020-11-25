//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "systems/render/ve_render_unit.h"
#include "systems/render/ve_render_api.h"
#include "systems/render/ve_render_resource.h"
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

	float near_plane = 0.1f;
	float far_plane = 1e+5f;

	ut::uint32 width = 640;
	ut::uint32 height = 480;

	struct GBuffer
	{
		GBuffer(Target depth_target,
		        Target diffuse_target,
		        Target normal_target,
		        Target material_target,
		        Framebuffer geometry_fb) : depth(ut::Move(depth_target))
		                                  , diffuse(ut::Move(diffuse_target))
		                                  , normal(ut::Move(normal_target))
		                                  , material(ut::Move(material_target))
		                                  , framebuffer(ut::Move(geometry_fb))
		{}

		Target depth, diffuse, normal, material;
		Framebuffer framebuffer;
		
	};

	struct FrameData
	{
		FrameData(GBuffer geometry_buffer) : g_buffer(ut::Move(geometry_buffer))
		{}

		GBuffer g_buffer;
	};

	struct GpuData : public Resource
	{
		GpuData(ut::Array<FrameData> in_frames,
		        PipelineState gbuffer_pipeline) : frames(ut::Move(in_frames))
		                                        , geometry_pass_pipeline(ut::Move(gbuffer_pipeline))
		{}

		ut::Array<FrameData> frames;

		PipelineState geometry_pass_pipeline;
	};

	RcRef<GpuData> data;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//