//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "systems/render/engine/ve_render_toolset.h"
#include "systems/render/engine/ve_render_batcher.h"
#include "systems/render/engine/units/ve_render_directional_light.h"
#include "systems/render/engine/units/ve_render_point_light.h"
#include "systems/render/engine/units/ve_render_spot_light.h"
#include "templates/ut_grid.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
START_NAMESPACE(lighting)
//----------------------------------------------------------------------------//
// Draws units in unlit mode.
class UnlitRenderer
{
public:
	// Per-view gpu data.
	struct ViewData
	{
		ut::Array<CmdBuffer> cmd;
	};

	// Constructor.
	UnlitRenderer(Toolset& toolset);

	// Creates unlit shading (per-view) data.
	//    @param depth_stencil - reference to the depth buffer.
	//    @return - a new UnlitRenderer::ViewData object or error if failed.
	ut::Result<UnlitRenderer::ViewData, ut::Error> CreateViewData(Target& depth_stencil);

	// Renders unlit units to a low dynamic range buffer.
	void DrawUnlitGeometryLdr(Context& context,
	                          Framebuffer& ldr_framebuffer,
	                          UnlitRenderer::ViewData& data,
	                          Buffer& view_uniform_buffer,
	                          Batcher& batcher);

private:
	struct MeshInstRendering
	{
		enum AlphaMode
		{
			alpha_opaque,
			alpha_test,
			alpha_blend,
			alpha_mode_count
		};

		enum CullMode
		{
			cull_none,
			cull_back,
			cull_mode_count
		};

		enum StencilMode
		{
			stencil_none,
			stencil_highlighted,
			stencil_mode_count
		};

		static constexpr ut::uint32 vertex_format_column = 0;
		static constexpr ut::uint32 alpha_mode_column = 1;
		static constexpr ut::uint32 cull_mode_column = 2;
		static constexpr ut::uint32 stencil_mode_column = 3;
		static constexpr ut::uint32 polygon_mode_column = 4;

		typedef ut::Grid<static_cast<size_t>(Mesh::VertexFormat::count),
		                 alpha_mode_count,
		                 cull_mode_count,
		                 stencil_mode_count,
		                 static_cast<size_t>(Mesh::PolygonMode::count)> PipelineGrid;

		typedef ut::Grid<static_cast<size_t>(Mesh::VertexFormat::count),
		                 alpha_mode_count> ShaderGrid;

		// Descriptor set for the light pass shaders.
		struct Descriptors : public DescriptorSet
		{
			Descriptors() : DescriptorSet(view_ub, transform_ub,
			                              material_ub, sampler, diffuse)
			{}

			Descriptor view_ub = "g_ub_view";
			Descriptor transform_ub = "g_ub_transform";
			Descriptor material_ub = "g_ub_material";
			Descriptor sampler = "g_sampler";
			Descriptor diffuse = "g_tex2d_diffuse";
		};
	};

	// Renders specified range of mesh instances.
	void RenderUnlitMeshInstancesJob(Context& context,
	                                 UnlitRenderer::ViewData& data,
	                                 Buffer& view_uniform_buffer,
	                                 Batcher& batcher,
	                                 ut::uint32 thread_id,
	                                 ut::uint32 offset,
	                                 ut::uint32 count);

	// Renders provided mesh.
	static void DrawMesh(Context& context,
	                     Mesh& mesh,
	                     PipelineState& pipeline,
	                     DescriptorSet& desc_set,
	                     Buffer& instance_buffer,
	                     ut::uint32 index_count,
	                     ut::uint32 index_offset,
	                     ut::uint32 instance_offset);

	// Creates all shader permutations for a mesh instance.
	ut::Array<BoundShader> CreateMeshInstShaders();

	// Creates unlit render pass.
	RenderPass CreatePass();

	// Creates a pipeline state for a mesh instance.
	ut::Result<PipelineState, ut::Error> CreateMeshInstPipeline(Mesh::VertexFormat vertex_format,
	                                                            Mesh::PolygonMode polygon_mode,
	                                                            MeshInstRendering::AlphaMode alpha_mode,
	                                                            MeshInstRendering::CullMode cull_mode,
	                                                            MeshInstRendering::StencilMode stencil_mode);

	// Creates all possible lighting pipeline state permutations for a mesh instance.
	ut::Array<PipelineState> CreateMeshInstPipelinePermutations();

	// Connects all descriptor sets to the corresponding shaders.
	void ConnectDescriptors();

	// Common rendering tools.
	Toolset& tools;

	// Shaders.
	ut::Array<BoundShader> mesh_inst_shader;

	// Render pass.
	RenderPass pass;

	// Pipeline states.
	ut::Array<PipelineState> mesh_inst_pipeline;

	// Descriptors.
	ut::Array<MeshInstRendering::Descriptors> mesh_inst_desc_set;

	// Secondary command buffers to parallelize cpu work.
	ut::Array< ut::Ref<CmdBuffer> > secondary_buffer_cache;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(lighting)
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//