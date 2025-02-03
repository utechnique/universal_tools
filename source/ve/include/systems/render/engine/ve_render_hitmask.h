//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "systems/render/engine/ve_render_toolset.h"
#include "systems/render/engine/ve_render_batcher.h"
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
		Framebuffer framebuffer;
		bool submitted;
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
	          Batcher& batcher);

	// Copies gpu data to the provided cpu buffer.
	void Read(Context& context,
	          HitMask::ViewData& gpu_data,
	          ut::Array<Entity::Id>& cpu_buffer);

	// Encodes the provided entity identifier into the hitmask compatible value.
	static MeshInstance::EntityIdBuffer::Type EncodeEntityId(Entity::Id entity_id);

	// Decodes the provided hitmask value into the entity identifier.
	static Entity::Id DecodeEntityId(const ut::Vector<4, ut::byte>& hitmask_value);

private:
	// Mesh instance rendering options.
	struct MeshInstRendering
	{
		enum class CullMode
		{
			none,
			back,
			count
		};

		enum class AlphaMode
		{
			opaque,
			alpha_test,
			count
		};

		typedef ut::Grid<static_cast<size_t>(Mesh::VertexFormat::count),
		                 static_cast<size_t>(AlphaMode::count),
		                 static_cast<size_t>(CullMode::count),
		                 static_cast<size_t>(Mesh::PolygonMode::count)> PipelineGrid;

		typedef ut::Grid<static_cast<size_t>(Mesh::VertexFormat::count),
		                 static_cast<size_t>(AlphaMode::count)> ShaderGrid;

		static constexpr ut::uint32 vertex_format_column = 0;
		static constexpr ut::uint32 alpha_mode_column = 1;
		static constexpr ut::uint32 cull_mode_column = 2;
		static constexpr ut::uint32 polygon_mode_column = 3;

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

	// Renders specified range of mesh instances.
	void DrawMeshInstancesJob(Context& context,
	                          HitMask::ViewData& data,
	                          Buffer& view_uniform_buffer,
	                          Batcher& batcher,
	                          ut::uint32 thread_id,
	                          ut::uint32 drawcall_index_offset,
	                          ut::uint32 drawcall_count);

	// Creates shaders to draw mesh instances.
	ut::Array<BoundShader> CreateMeshInstShader();

	// Creates the hitmask render pass.
	RenderPass CreateRenderPass();

	// Creates a pipeline state to render geometry to the hitmask.
	ut::Result<PipelineState, ut::Error> CreateMeshInstPipeline(Mesh::VertexFormat vertex_format,
	                                                            Mesh::PolygonMode polygon_mode,
	                                                            MeshInstRendering::AlphaMode alpha_mode,
	                                                            MeshInstRendering::CullMode cull_mode);

	// Creates all possible pipeline state permutations for a mesh instance.
	ut::Array<PipelineState> CreateMeshInstPipelinePermutations();

	// Connects all descriptor sets to the corresponding shaders.
	void ConnectDescriptors();

	// Common rendering tools.
	Toolset& tools;

	// Shaders to draw mesh instances.
	ut::Array<BoundShader> mesh_inst_shader;

	// Render pass.
	RenderPass pass;

	// Pipeline states.
	ut::Array<PipelineState> mesh_inst_pipeline;

	// Descriptor sets.
	MeshInstRendering::AlphaTestOffDescriptors mesh_inst_at_off_desc_set;
	MeshInstRendering::AlphaTestOnDescriptors mesh_inst_at_on_desc_set;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
