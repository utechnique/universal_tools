//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "systems/render/api/ve_render_platform.h"
#include "systems/render/api/ve_render_shader.h"
#include "systems/render/api/ve_render_pixel_format.h"
#include "systems/render/api/ve_render_vertex.h"
#include "systems/render/api/ve_render_primitive_topology.h"
#include "systems/render/api/ve_render_descriptor.h"
#include "systems/render/api/ve_render_cmp.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// The purpose of the input-assembler stage is to read primitive data (points,
// lines and/or triangles) from user-filled buffers and assemble the data into
// primitives that will be used by the other pipeline stages.
class InputAssemblyState
{
public:
	// primitive topology
	primitive::Topology topology;

	// stride (in bytes) to the next primitive
	ut::uint32 vertex_stride;

	// stride (in bytes) to the next instance
	ut::uint32 instance_stride;

	// elements a primitive's vertex consists of
	ut::Array<VertexElement> elements;

	// per-instance elements
	ut::Array<VertexElement> instance_elements;
};

//----------------------------------------------------------------------------//
// A viewport maps vertex positions(in clip space) into render target positions.
struct Viewport
{
	Viewport(float in_x = 0.0f,
	         float in_y = 0.0f,
	         float in_width = 0.0f,
	         float in_height = 0.0f,
	         float in_min_depth = 0.0f,
	         float in_max_depth = 0.0f,
	         ut::uint32 in_buffer_width = 0,
	         ut::uint32 in_buffer_height = 0) : x(in_x)
		                                      , y(in_y)
	                                          , width(in_width)
	                                          , height(in_height)
	                                          , min_depth(in_min_depth)
	                                          , max_depth(in_max_depth)
	                                          , buffer_width(in_buffer_width)
	                                          , buffer_height(in_buffer_height)
	{}

	float x = 0.0f; // left position in pixels
	float y = 0.0f; // top position in pixels
	float width = 0.0f; // width in pixels
	float height = 0.0f; // height in pixels
	float min_depth = 0.0f; // 0 - 1
	float max_depth = 0.0f; // 0 - 1
	ut::uint32 buffer_width = 0; // actual width of render buffer in pixels
	ut::uint32 buffer_height = 0; // actual height of render buffer in pixels
	ut::Optional< ut::Rect<int> > scissor; // everything out of bounds is ignored
};

//----------------------------------------------------------------------------//
// The rasterization stage converts vector information (composed of shapes or
// primitives) into a raster image (composed of pixels) for the purpose of
// displaying real-time 3D graphics.
struct RasterizationState
{
	// Polygon rendering mode
	enum PolygonMode
	{
		fill,
		line,
		point
	};

	// Polygon facing direction used for primitive culling.
	enum CullMode
	{
		no_culling,
		front_culling,
		back_culling
	};

	// Specifies the front-facing triangle orientation to be used for culling.
	enum FrontFace
	{
		counter_clockwise,
		clockwise
	};

	// Constructor.
	RasterizationState(PolygonMode in_polygon_mode = fill,
	                   CullMode in_cull_mode = no_culling,
	                   FrontFace in_front_face = clockwise,
	                   float in_line_width = 1.0f,
	                   bool in_discard_enable = false,
	                   bool in_depth_bias_enable = false,
	                   float in_depth_bias_constant_factor = 0.0f,
	                   float in_depth_bias_clamp = 0.0f,
	                   float in_depth_bias_slope_factor = 0.0f) : polygon_mode(in_polygon_mode)
	                                                            , cull_mode(in_cull_mode)
	                                                            , front_face(in_front_face)
	                                                            , line_width(in_line_width)
	                                                            , discard_enable(in_discard_enable)
	                                                            , depth_bias_enable(in_depth_bias_enable)
	                                                            , depth_bias_constant_factor(in_depth_bias_constant_factor)
	                                                            , depth_bias_clamp(in_depth_bias_clamp)
	                                                            , depth_bias_slope_factor(in_depth_bias_slope_factor)
	{}

	PolygonMode polygon_mode;
	CullMode cull_mode;
	FrontFace front_face;
	float line_width;
	bool discard_enable;
	bool depth_bias_enable;
	float depth_bias_constant_factor;
	float depth_bias_clamp;
	float depth_bias_slope_factor;
};

//----------------------------------------------------------------------------//
// Structure specifying stencil operation state.
struct StencilOpState
{
	// Operation type.
	enum Operation
	{
		keep,
		zero,
		replace,
		increment_and_clamp,
		decrement_and_clamp,
		invert,
		increment_and_wrap,
		decrement_and_wrap,
	};

	// Constructor.
	StencilOpState(compare::Operation in_compare_op = compare::always,
	               Operation in_fail_op = keep,
	               Operation in_pass_op = keep,
	               Operation in_depth_fail_op = keep,
	               ut::uint32 in_compare_mask = 0,
	               ut::uint32 in_write_mask = 0,
	               ut::uint32 in_reference = 0) : compare_op(in_compare_op)
	                                            , fail_op(in_fail_op)
	                                            , pass_op(in_pass_op)
	                                            , depth_fail_op(in_depth_fail_op)
	                                            , compare_mask(in_compare_mask)
	{}

	compare::Operation compare_op;
	Operation fail_op;
	Operation pass_op;
	Operation depth_fail_op;
	ut::uint32 compare_mask;
	
};

//----------------------------------------------------------------------------//
// Depth-stencil state object encapsulates depth-stencil test information for
// the output-merger stage.
struct DepthStencilState
{
	DepthStencilState(bool in_depth_test_enable = false,
	                  bool in_stencil_test_enable = false,
	                  bool in_depth_write_enable = false,
	                  compare::Operation in_depth_compare_op = compare::always,
	                  StencilOpState in_front = StencilOpState(),
	                  StencilOpState in_back = StencilOpState()) : depth_test_enable(in_depth_test_enable)
	                                                             , stencil_test_enable(in_stencil_test_enable)
	                                                             , depth_write_enable(in_depth_write_enable)
	                                                             , depth_compare_op(in_depth_compare_op)
	                                                             , front(in_front)
	                                                             , back(in_back)
	                                                             , stencil_write_mask(0xff)
	                                                             , stencil_reference(0)
	{}

	bool depth_test_enable;
	bool stencil_test_enable;
	bool depth_write_enable;
	compare::Operation depth_compare_op;
	StencilOpState front;
	StencilOpState back;
	ut::uint32 stencil_write_mask;
	ut::uint32 stencil_reference;
};

//----------------------------------------------------------------------------//
// Describes the blend state for a render target.
struct Blending
{
	// Blend factor, which modulates values for the pixel shader
	// and render target.
	enum Factor
	{
		zero,
		one,
		src_color,
		inverted_src_color,
		dst_color,
		inverted_dst_color,
		src_alpha,
		inverted_src_alpha,
		dst_alpha,
		inverted_dst_alpha,
		src1_color,
		inverted_src1_color,
		src1_alpha,
		inverted_src1_alpha,
	};

	// RGB or alpha blending operation.
	enum Operation
	{
		add,
		subtract,
		reverse_subtract,
		min,
		max,
	};

	enum Type
	{
		alpha,
		additive
	};

	// Constructor
	Blending(bool in_blend_enable = false,
	         Factor in_src_blend = src_alpha,
	         Factor in_dst_blend = inverted_src_alpha,
	         Operation in_color_op = add,
	         Factor in_src_blend_alpha = one,
	         Factor in_dst_blend_alpha = one,
	         Operation in_alpha_op = max,
	         ut::uint8 in_write_mask = 0xf) : blend_enable(in_blend_enable)
	                                        , src_blend(in_src_blend)
	                                        , dst_blend(in_dst_blend)
	                                        , color_op(in_color_op)
	                                        , src_blend_alpha(in_src_blend_alpha)
	                                        , dst_blend_alpha(in_dst_blend_alpha)
	                                        , alpha_op(in_alpha_op)
	                                        , write_mask(in_write_mask)
	{}

	bool blend_enable;
	Factor src_blend;
	Factor dst_blend;
	Operation color_op;
	Factor src_blend_alpha;
	Factor dst_blend_alpha;
	Operation alpha_op;
	ut::uint8 write_mask;
};

//----------------------------------------------------------------------------//
// Blending applies a simple function to combine output values from a pixel
// shader with data in a render target. You have control over how the pixels are
// blended by using a predefined set of blending operations and preblending
// operations.
struct BlendState
{
	// Default constructor. Note that it doesn't create any blending attachments.
	BlendState()
	{}

	// Constructor
	BlendState(ut::Array<Blending> in_attachments) : attachments(ut::Move(in_attachments))
	{}

	// Blending templates
	static Blending CreateNoBlending()
	{
		return Blending(false,
		                Blending::src_alpha,
		                Blending::inverted_src_alpha,
		                Blending::add,
		                Blending::one,
		                Blending::one,
		                Blending::max,
		                0xf);
	}
	static Blending CreateAlphaBlending()
	{
		return Blending(true,
		                Blending::src_alpha,
		                Blending::inverted_src_alpha,
		                Blending::add,
		                Blending::one,
		                Blending::one,
		                Blending::max,
		                0xf);
	}
	static Blending CreateAdditiveBlending()
	{
		return Blending(true,
		                Blending::one,
		                Blending::one,
		                Blending::add,
		                Blending::one,
		                Blending::one,
		                Blending::add,
		                0xf);
	}

	ut::Array<Blending> attachments;
};

//----------------------------------------------------------------------------//
class PipelineState : public PlatformPipelineState
{
	friend class Context;
	friend class Device;
public:
	// ve::render::PipelineInfo conveniently stores all essential
	// information about the graphics pipeline.
	class Info
	{
	public:
		// Constructor.
		Info() : tessellation_enable(false)
		{}

		// shaders bound to the pipeline
		ut::Optional<Shader&> stages[Shader::skStageCount];

		// viewports
		ut::Array<Viewport> viewports;

		// tessellation
		bool tessellation_enable;

		// states
		InputAssemblyState input_assembly_state;
		RasterizationState rasterization_state;
		DepthStencilState depth_stencil_state;
		BlendState blend_state;
	};

	// Constructor.
	PipelineState(PlatformPipelineState platform_pipeline, Info pipeline_info);

	// Move constructor.
	PipelineState(PipelineState&&) noexcept;

	// Move operator.
	PipelineState& operator =(PipelineState&&) noexcept;

	// Copying is prohibited.
	PipelineState(const PipelineState&) = delete;
	PipelineState& operator =(const PipelineState&) = delete;

	// Returns a const reference to the object with
	// information about this image.
	const Info& GetInfo() const
	{
		return info;
	}

private:
	Info info;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//