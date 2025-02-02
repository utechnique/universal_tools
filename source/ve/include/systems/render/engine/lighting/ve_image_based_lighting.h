//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "systems/render/engine/ve_render_toolset.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
START_NAMESPACE(lighting)
//----------------------------------------------------------------------------//
// Encapsulates Image Based Lighting techniques.
class IBL
{
public:
	// Per-view gpu data.
	struct ViewData
	{
		Target filtered_cubemap;
		ut::Array< ut::Array<Framebuffer> > filter_framebuffer;
		ut::uint32 face_id;
		bool initialized;
	};

	// Constructor.
	IBL(Toolset& toolset);

	// Creates IBL (per-view) data.
	ut::Result<IBL::ViewData, ut::Error> CreateViewData();

	// Filters IBL cubemap.
	void FilterCubemap(Context& context,
	                   IBL::ViewData& data,
	                   Image& cubemap,
	                   Image::Cube::Face face);

	// Returns the number of mip levels in the IBL cubemap.
	ut::uint32 GetMipCount() const
	{
		return mip_count;
	}

	// Creates a projection matrix that is common for all faces.
	static ut::Matrix<4> CreateFaceProjectionMatrix(float znear, float zfar);

	// Creates a view matrix for the desired cube face.
	static ut::Matrix<4> CreateFaceViewMatrix(Image::Cube::Face face,
	                                          const ut::Vector<3>& position);

private:
	// Contents of the uniform buffer for the IBL filtering.
	struct FilterUniforms
	{
		alignas(16) ut::Matrix<4, 4> view_proj;
		alignas(16) ut::Vector<4> filter_parameters;
	};

	// Creates vertex and pixel shaders for filtering an ibl cubemap.
	BoundShader CreateFilterShader();

	// Creates a render pass for the IBL filtering.
	RenderPass CreateFilterPass();

	// Creates all permutations of ibl filtering pipeline states.
	ut::Array<PipelineState> CreateFilterPipelines();

	// Creates uniform buffers for all mips.
	ut::Array< ut::Array<Buffer> > CreateFilterUniformBuffers();

	// Creates a cube for rendering.
	ut::Result<RcRef<Mesh>, ut::Error> CreateCube();

	// Calculates a number of mips in the IBL cubemap.
	static ut::uint32 CalculateMipCount(ut::uint32 size);

	Toolset& tools;
	ut::uint32 mip_count;
	RcRef<Mesh> cube;
	BoundShader filter_shader;
	RenderPass filter_pass;
	ut::Array<PipelineState> filter_pipeline;
	ut::Array< ut::Array<Buffer> > filter_ub;

	struct FilterDescriptorSet : public DescriptorSet
	{
		FilterDescriptorSet() : DescriptorSet(filter_ub, sampler, cubemap)
		{}

		Descriptor filter_ub = "g_filter_ub";
		Descriptor sampler = "g_sampler";
		Descriptor cubemap = "g_texcube_scene";
	} filter_desc_set;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(lighting)
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
