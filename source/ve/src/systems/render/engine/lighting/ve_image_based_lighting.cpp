//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "systems/render/engine/lighting/ve_image_based_lighting.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
START_NAMESPACE(lighting)
//----------------------------------------------------------------------------//
// Constructor.
IBL::IBL(Toolset& toolset) : tools(toolset)
                           , mip_count(CalculateMipCount(toolset.config.ibl_size))
                           , cube(CreateCube().MoveOrThrow())
                           , filter_shader(CreateFilterShader())
                           , filter_pass(CreateFilterPass())
                           , filter_pipeline(CreateFilterPipelines())
                           , filter_ub(CreateFilterUniformBuffers())
{
	filter_desc_set.Connect(filter_shader);
}

// Creates IBL (per-view) data.
ut::Result<IBL::ViewData, ut::Error> IBL::CreateViewData()
{
	// cubemap
	Target::Info info;
	info.type = Image::Type::cubic;
	info.format = tools.formats.ibl;
	info.usage = Target::Info::Usage::color;
	info.mip_count = mip_count;
	info.width = tools.config.ibl_size;
	info.height = tools.config.ibl_size;
	info.depth = 1;
	ut::Result<Target, ut::Error> cubemap = tools.device.CreateTarget(info);
	if (!cubemap)
	{
		return ut::MakeError(cubemap.MoveAlt());
	}

	// create framebuffers and pipeline states
	ut::uint32 mip_size = tools.config.ibl_size;
	ut::Array< ut::Array<Framebuffer> > filter_framebuffers(6);
	for (ut::uint32 mip_id = 0; mip_id < mip_count; mip_id++)
	{
		// create framebuffers for all faces
		for (ut::uint32 face_id = 0; face_id < 6; face_id++)
		{
			ut::Array<Framebuffer::Attachment> color_targets;
			color_targets.Add(Framebuffer::Attachment(cubemap.Get(), face_id, mip_id));
			ut::Result<Framebuffer, ut::Error> filter_framebuffer = tools.device.CreateFramebuffer(filter_pass,
			                                                                                       ut::Move(color_targets));
			if (!filter_framebuffer)
			{
				return ut::MakeError(filter_framebuffer.MoveAlt());
			}
			filter_framebuffers[face_id].Add(filter_framebuffer.Move());
		}

		// calculate the mip size for the next iteration
		mip_size /= 2;
	}

	return IBL::ViewData{ cubemap.Move(),
	                      ut::Move(filter_framebuffers),
	                      0, false };
}

// Filters IBL cubemap.
void IBL::FilterCubemap(Context& context,
                        IBL::ViewData& data,
                        Image& cubemap,
                        Image::Cube::Face face)
{
	filter_desc_set.sampler.BindSampler(tools.sampler_cache.linear_wrap);
	filter_desc_set.cubemap.BindImage(cubemap);

	Mesh& mesh = cube.Get();
	Mesh::Subset& mesh_subset = mesh.subsets.GetFirst();

	ut::uint32 mip_size = tools.config.ibl_size;
	for (ut::uint32 mip = 0; mip < mip_count; mip++)
	{
		filter_desc_set.filter_ub.BindUniformBuffer(filter_ub[static_cast<ut::uint32>(face)][mip]);

		ut::Rect<ut::uint32> render_area(0, 0, mip_size, mip_size);
		context.BeginRenderPass(filter_pass,
		                        data.filter_framebuffer[static_cast<ut::uint32>(face)][mip],
		                        render_area,
		                        ut::Color<4>(0));
		context.BindPipelineState(filter_pipeline[mip]);
		context.BindDescriptorSet(filter_desc_set);
		context.BindVertexBuffer(mesh_subset.vertex_buffer.GetRef(), 0);

		if (mesh_subset.index_buffer)
		{
			const ut::uint32 index_count = mesh_subset.count;
			context.BindIndexBuffer(mesh_subset.index_buffer.GetRef(),
			                        0,
			                        mesh_subset.index_buffer->format);
			context.DrawIndexed(index_count, 0, 0);
		}
		else
		{
			context.Draw(mesh_subset.count, 0);
		}

		context.EndRenderPass();

		mip_size /= 2;
	}
}

// Creates a projection matrix that is common for all faces.
ut::Matrix<4> IBL::CreateFaceProjectionMatrix(float znear, float zfar)
{
	const float fov = ut::ToRadiands(90.0f);
	const float scale_y = 1.0f / ut::Tan(fov / 2.0f);
	const float scale_x = scale_y;
	return ut::Matrix<4>(scale_x, 0, 0, 0,
	                     0, scale_y, 0, 0,
	                     0, 0, zfar / (zfar - znear), 1,
	                     0, 0, -znear*zfar / (zfar - znear), 0);
}

// Creates a view matrix for the desired cube face.
ut::Matrix<4> IBL::CreateFaceViewMatrix(Image::Cube::Face face,
                                        const ut::Vector<3>& position)
{
	// calculate basis
	ut::Vector<3> direction, up, right;
	switch (face)
	{
		case Image::Cube::Face::positive_x:
			direction = ut::Vector<3>(1, 0, 0);
			up = ut::Vector<3>(0, 1, 0);
			right = ut::Vector<3>(0, 0, -1);
			break;
		case Image::Cube::Face::positive_y:
			direction = ut::Vector<3>(0, 1, 0);
			up = ut::Vector<3>(0, 0, -1);
			right = ut::Vector<3>(1, 0, 0);
			break;
		case Image::Cube::Face::positive_z:
			direction = ut::Vector<3>(0, 0, 1);
			up = ut::Vector<3>(0, 1, 0);
			right = ut::Vector<3>(1, 0, 0);
			break;
		case Image::Cube::Face::negative_x:
			direction = ut::Vector<3>(-1, 0, 0);
			up = ut::Vector<3>(0, 1, 0);
			right = ut::Vector<3>(0, 0, 1);
			break;
		case Image::Cube::Face::negative_y:
			direction = ut::Vector<3>(0, -1, 0);
			up = ut::Vector<3>(0, 0, 1);
			right = ut::Vector<3>(1, 0, 0);
			break;
		case Image::Cube::Face::negative_z:
			direction = ut::Vector<3>(0, 0, -1);
			up = ut::Vector<3>(0, 1, 0);
			right = ut::Vector<3>(-1, 0, 0);
			break;
	}

	return ut::Matrix<4>(right.X(), up.X(), direction.X(), 0.0f,
	                     right.Y(), up.Y(), direction.Y(), 0.0f,
	                     right.Z(), up.Z(), direction.Z(), 0.0f,
	                     -position.Dot(right), -position.Dot(up),
	                     -position.Dot(direction), 1.0f);
}

// Creates vertex and pixel shaders for filtering an ibl cubemap.
BoundShader IBL::CreateFilterShader()
{
	Shader::Macros macros;
	Shader::MacroDefinition mip_count_macro;
	mip_count_macro.name = "MIP_COUNT";
	mip_count_macro.value = ut::Print(mip_count);
	macros.Add(ut::Move(mip_count_macro));

	ut::Result<Shader, ut::Error> vs = tools.shader_loader.Load(Shader::Stage::vertex, "ibl_filter_vs", "FilterVS", "ibl.hlsl");
	ut::Result<Shader, ut::Error> ps = tools.shader_loader.Load(Shader::Stage::pixel, "ibl_filter_ps", "FilterPS", "ibl.hlsl", macros);
	return BoundShader(vs.MoveOrThrow(), ps.MoveOrThrow());
}

// Creates a render pass for the IBL filtering.
RenderPass IBL::CreateFilterPass()
{
	RenderTargetSlot color_slot(tools.formats.ibl,
	                            RenderTargetSlot::LoadOperation::dont_care,
	                            RenderTargetSlot::StoreOperation::save,
	                            false);
	ut::Array<RenderTargetSlot> color_slots;
	color_slots.Add(color_slot);
	return tools.device.CreateRenderPass(ut::Move(color_slots)).MoveOrThrow();
}

// Creates all permutations of ibl filtering pipeline states.
ut::Array<PipelineState> IBL::CreateFilterPipelines()
{
	ut::Array<PipelineState> pipeline_states;

	ut::uint32 mip_size = tools.config.ibl_size;
	for (ut::uint32 mip_id = 0; mip_id < mip_count; mip_id++)
	{
		// create pipeline for the current mip
		PipelineState::Info info;
		info.SetShader(Shader::Stage::vertex,
		               filter_shader.GetShader(Shader::Stage::vertex));
		info.SetShader(Shader::Stage::pixel,
		               filter_shader.GetShader(Shader::Stage::pixel));
		info.input_assembly_state = cube->subsets.GetFirst().CreateIaState();
		info.depth_stencil_state.depth_test_enable = false;
		info.depth_stencil_state.depth_write_enable = false;
		info.rasterization_state.polygon_mode = RasterizationState::PolygonMode::fill;
		info.rasterization_state.cull_mode = RasterizationState::CullMode::off;
		info.blend_state.attachments.Add(BlendState::CreateNoBlending());
		ut::Result<PipelineState, ut::Error> filter_pipeline =
			tools.device.CreatePipelineState(ut::Move(info), filter_pass);
		if (!filter_pipeline)
		{
			throw ut::Error(filter_pipeline.MoveAlt());
		}
		pipeline_states.Add(filter_pipeline.Move());

		// calculate the mip size for the next iteration
		mip_size /= 2;
	}

	return pipeline_states;
}

// Creates uniform buffers for all mips.
ut::Array< ut::Array<Buffer> > IBL::CreateFilterUniformBuffers()
{
	const ut::Matrix<4> proj_matrix = CreateFaceProjectionMatrix(0.2f, 2.0f);
	ut::Array< ut::Array<Buffer> > out(6);
	for (ut::uint32 face_id = 0; face_id < 6; face_id++)
	{
		const ut::Matrix<4> view_matrix = CreateFaceViewMatrix(static_cast<Image::Cube::Face>(face_id),
		                                                       ut::Vector<3>(0));
		const ut::Matrix<4> view_proj = view_matrix * proj_matrix;
		for (ut::uint32 mip_id = 0; mip_id < mip_count; mip_id++)
		{
			Buffer::Info buffer_info;
			buffer_info.type = Buffer::Type::uniform;
			buffer_info.usage = render::memory::Usage::gpu_immutable;
			buffer_info.size = sizeof(IBL::FilterUniforms);
			buffer_info.data.Resize(buffer_info.size);
			FilterUniforms& uniforms = *(reinterpret_cast<FilterUniforms*>(buffer_info.data.GetAddress()));
			uniforms.view_proj = view_proj;
			uniforms.filter_parameters.X() = static_cast<float>(mip_id);
			uniforms.filter_parameters.Y() = 1.0f;
			ut::Result<Buffer, ut::Error> uniform_buffer = tools.device.CreateBuffer(ut::Move(buffer_info));
			out[face_id].Add(uniform_buffer.MoveOrThrow());
		}
	}
	return out;
}

// Creates a cube for rendering.
ut::Result<RcRef<Mesh>, ut::Error> IBL::CreateCube()
{
	ResourceCreator<Mesh>& mesh_creator = tools.rc_mgr.GetCreator<Mesh>();

	// generate vertices
	Mesh::VertexFormat vertex_format = Mesh::VertexFormat::pos3_float;
	Mesh::PolygonMode polygon_mode = Mesh::PolygonMode::triangle;
	ResourceCreator<Mesh>::GeometryData cube_data = mesh_creator.GenBoxVertices(vertex_format,
	                                                                            polygon_mode,
	                                                                            ut::Vector<3>(0),
	                                                                            ut::Vector<3>(1));

	// create vertex buffer
	Buffer::Info vertex_buffer_info;
	vertex_buffer_info.type = Buffer::Type::vertex;
	vertex_buffer_info.usage = render::memory::Usage::gpu_immutable;
	vertex_buffer_info.size = cube_data.vertex_buffer.GetSize();
	vertex_buffer_info.stride = static_cast<ut::uint32>(vertex_buffer_info.size) / cube_data.vertex_count;
	vertex_buffer_info.data = ut::Move(cube_data.vertex_buffer);
	ut::Result<Buffer, ut::Error> vertex_buffer = tools.device.CreateBuffer(ut::Move(vertex_buffer_info));
	if (!vertex_buffer)
	{
		return ut::MakeError(vertex_buffer.MoveAlt());
	}

	// create index buffer
	Buffer::Info index_buffer_info;
	index_buffer_info.type = Buffer::Type::index;
	index_buffer_info.usage = render::memory::Usage::gpu_immutable;
	index_buffer_info.size = sizeof(ut::uint32) * cube_data.index_count;
	index_buffer_info.stride = sizeof(ut::uint32);
	index_buffer_info.data = ut::Move(cube_data.index_buffer);
	ut::Result<Buffer, ut::Error> index_buffer = tools.device.CreateBuffer(ut::Move(index_buffer_info));
	if (!index_buffer)
	{
		return ut::MakeError(index_buffer.MoveAlt());
	}

	// create a mesh subset
	ut::Array<Mesh::Subset> subsets;
	Mesh::Subset subset;
	subset.vertex_buffer = ut::MakeUnsafeShared<Mesh::VertexBuffer>(vertex_buffer.Move(),
	                                                                vertex_format);
	subset.index_buffer = ut::MakeUnsafeShared<Mesh::IndexBuffer>(index_buffer.Move(),
	                                                              Mesh::index_format);
	subset.polygon_mode = polygon_mode;
	subset.offset = 0;
	subset.count = cube_data.index_count;
	subsets.Add(ut::Move(subset));

	// create mesh
	return tools.rc_mgr.AddResource<Mesh>(Mesh(ut::Move(subsets)));
}

// Calculates a number of mips in the IBL cubemap.
ut::uint32 IBL::CalculateMipCount(ut::uint32 size)
{
	return static_cast<ut::uint32>(ut::Logarithm2(static_cast<float>(size))) + 1;
}

//----------------------------------------------------------------------------//
END_NAMESPACE(lighting)
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//