//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "systems/render/engine/ve_render_toolset.h"
#include "systems/render/engine/ve_render_model_batcher.h"
#include "templates/ut_grid.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// Encapsulates hitmask rendering techniques.
class HitMask
{
public:
	// Type of the data stored in a pixel of hitmask.
	typedef ut::uint32 PixelFormat;

	// Per-view gpu data.
	struct ViewData
	{
		Target target;
		RenderPass pass;
		Framebuffer framebuffer;
		ut::Array<PipelineState> model_pipeline;
		bool submitted = false;
	};

	// Constructor.
	HitMask(Toolset& toolset);

	// Creates hitmask (per-view) data.
	//    @param width - width of the view in pixels.
	//    @param height - height of the view in pixels.
	ut::Result<HitMask::ViewData, ut::Error> CreateViewData(Target& depth_stencil,
	                                                        ut::uint32 width,
	                                                        ut::uint32 height);

	// Renders the scnene to the provided hitmask.
	void Draw(Context& context,
	          Target& depth_stencil,
	          HitMask::ViewData& data,
	          Buffer& view_uniform_buffer,
	          ModelBatcher& batcher);

	// Copies gpu data to the provided cpu buffer.
	void Read(Context& context,
	          HitMask::ViewData& gpu_data,
	          ut::Array<Entity::Id>& cpu_buffer);

	// Encodes the provided entity identifier into the hitmask compatible value.
	static Model::EntityIdBuffer::Type EncodeEntityId(Entity::Id entity_id);

	// Decodes the provided hitmask value into the entity identifier.
	static Entity::Id DecodeEntityId(const ut::Vector<4, ut::byte>& hitmask_value);

private:
	// Model-rendering options.
	struct ModelRendering
	{
		enum CullMode
		{
			cull_none,
			cull_back,
			cull_mode_count
		};

		enum AlphaMode
		{
			alpha_opaque,
			alpha_test,
			alpha_mode_count
		};

		typedef ut::Grid<Mesh::vertex_format_count,
		                 alpha_mode_count,
		                 cull_mode_count> PipelineGrid;

		typedef ut::Grid<Mesh::vertex_format_count,
		                 alpha_mode_count> ShaderGrid;

		static constexpr ut::uint32 vertex_format_column = 0;
		static constexpr ut::uint32 alpha_mode_column = 1;
		static constexpr ut::uint32 cull_mode_column = 2;

		struct AlphaTestOffDescriptors : public DescriptorSet
		{
			AlphaTestOffDescriptors() : DescriptorSet(view_ub, transform_ub, hitmask_id_ub)
			{}

			Descriptor view_ub = "g_ub_view";
			Descriptor transform_ub = "g_ub_transform";
			Descriptor hitmask_id_ub = "g_ub_hitmask_id";
		};

		struct AlphaTestOnDescriptors : public DescriptorSet
		{
			AlphaTestOnDescriptors() : DescriptorSet(view_ub, transform_ub, hitmask_id_ub,
			                                         sampler, diffuse)
			{}

			Descriptor view_ub = "g_ub_view";
			Descriptor transform_ub = "g_ub_transform";
			Descriptor hitmask_id_ub = "g_ub_hitmask_id";
			Descriptor sampler = "g_sampler";
			Descriptor diffuse = "g_tex2d_diffuse";
		};
	};

	// Renders specified range of models.
	void DrawModelsJob(Context& context,
	                   HitMask::ViewData& data,
	                   Buffer& view_uniform_buffer,
	                   ModelBatcher& batcher,
	                   ut::uint32 thread_id,
	                   ut::uint32 drawcall_index_offset,
	                   ut::uint32 drawcall_count);

	// Creates shaders for model rendering.
	ut::Array<BoundShader> CreateModelShader();

	// Creates the hitmask render pass.
	ut::Result<RenderPass, ut::Error> CreateRenderPass(pixel::Format depth_stencil_format);

	// Creates a pipeline state to render geometry to the hitmask.
	ut::Result<PipelineState, ut::Error> CreateModelPipeline(RenderPass& render_pass,
	                                                         ut::uint32 width,
	                                                         ut::uint32 height,
	                                                         Mesh::VertexFormat vertex_format,
	                                                         ModelRendering::AlphaMode alpha_mode,
	                                                         ModelRendering::CullMode cull_mode);

	// Connects all descriptor sets to the corresponding shaders.
	void ConnectDescriptors();

	// Common rendering tools.
	Toolset& tools;

	// Shaders for model-rendering.
	ut::Array<BoundShader> model_shader;

	// Descriptor sets.
	ModelRendering::AlphaTestOffDescriptors model_at_off_desc_set;
	ModelRendering::AlphaTestOnDescriptors model_at_on_desc_set;

	// G-Buffer target format.
	static constexpr pixel::Format skHitMaskFormat = pixel::r8g8b8a8_unorm;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
