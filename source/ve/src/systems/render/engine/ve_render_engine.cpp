//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "systems/render/engine/ve_render_engine.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// Constructor.
Engine::Engine(Device& render_device, ViewportManager viewport_mgr) : ViewportManager(ut::Move(viewport_mgr))
                                                                    , device(render_device)
                                                                    , img_loader(render_device)
                                                                    , shader_loader(render_device)
                                                                    , current_frame_id(0)
{
	// load configuration file
	ut::Optional<ut::Error> load_cfg_error = config.Load();
	if (load_cfg_error)
	{
		const ut::error::Code error_code = load_cfg_error->GetCode();
		if (error_code == ut::error::no_such_file)
		{
			ut::log << "Render config file is absent. Using default configuration..." << ut::cret;
			config.Save();
		}
		else
		{
			ut::log << "Fatal error while loading render config file." << ut::cret;
			throw load_cfg_error.Move();
		}
	}

	// set vertical synchronization for viewports
	SetVerticalSynchronization(config.vsync);

	// initialize color to clear display with
	backbuffer_clear_values.Add(ut::Color<4>(0, 0, 0, 0));







	// load display shaders
	ut::Result<Shader, ut::Error> display_vs = shader_loader.Load(Shader::vertex, "display_vs", "VS", "display.hlsl");
	ut::Result<Shader, ut::Error> display_ps = shader_loader.Load(Shader::pixel, "display_ps", "PS", "display.hlsl");
	display_shader = ut::MakeUnique<BoundShader>(display_vs.MoveOrThrow(), display_ps.MoveOrThrow());


	struct QuadVertex
	{
		ut::Vector<3> p;
		ut::Vector<2> t;
	};

	// create screen space quad
	Buffer::Info buffer_info;
	buffer_info.type = Buffer::vertex;
	buffer_info.usage = render::memory::immutable;
	buffer_info.size = sizeof(QuadVertex) * 4;
	buffer_info.stride = sizeof(QuadVertex);
	buffer_info.data.Resize(buffer_info.size);

	QuadVertex* vertices = reinterpret_cast<QuadVertex*>(buffer_info.data.GetAddress());
	vertices[0].p = ut::Vector<3>(-1, 1, 0);
	vertices[0].t = ut::Vector<2>(0, 0);
	vertices[1].p = ut::Vector<3>(1, 1, 0);
	vertices[1].t = ut::Vector<2>(1, 0);
	vertices[2].p = ut::Vector<3>(-1, -1, 0);
	vertices[2].t = ut::Vector<2>(0, 1);
	vertices[3].p = ut::Vector<3>(1, -1, 0);
	vertices[3].t = ut::Vector<2>(1, 1);

	ut::Result<Buffer, ut::Error> buffer_result = device.CreateBuffer(ut::Move(buffer_info));
	screen_space_quad = ut::MakeUnique<Buffer>(buffer_result.MoveOrThrow());

	// 1d image
	Image::Info img_info_1d;
	img_info_1d.type = Image::type_1D;
	img_info_1d.format = pixel::r8g8b8a8;
	img_info_1d.usage = render::memory::immutable;
	img_info_1d.width = 6;
	img_info_1d.height = 1;
	img_info_1d.depth = 1;
	img_info_1d.mip_count = 1;
	img_info_1d.data.Resize(img_info_1d.width * pixel::GetSize(pixel::r8g8b8a8));
	ut::Color<4, byte>* pixel_1d = reinterpret_cast<ut::Color<4, byte>*>(img_info_1d.data.GetAddress());
	pixel_1d[0] = ut::Color<4, byte>(255, 0, 0, 0);
	pixel_1d[1] = ut::Color<4, byte>(0, 255, 0, 0);
	pixel_1d[2] = ut::Color<4, byte>(0, 0, 255, 0);
	pixel_1d[3] = ut::Color<4, byte>(255, 255, 0, 0);
	pixel_1d[4] = ut::Color<4, byte>(0, 255, 255, 0);
	pixel_1d[5] = ut::Color<4, byte>(255, 0, 255, 0);
	ut::Result<Image, ut::Error> img1d_result = device.CreateImage(ut::Move(img_info_1d));
	img_1d = ut::MakeUnique<Image>(img1d_result.MoveOrThrow());

	// 2d image
	ImageLoader::Info img_info_2d;
	img_info_2d.srgb = true;
	img_info_2d.high_quality_mips = false;
	ut::Result<Image, ut::Error> img2d_result = img_loader.Load("maps/lazb.jpg", img_info_2d);
	img_2d = ut::MakeUnique<Image>(img2d_result.MoveOrThrow());

	// cube image
	Image::Info img_info_cube;
	img_info_cube.type = Image::type_cube;
	img_info_cube.format = pixel::r8g8b8a8;
	img_info_cube.usage = render::memory::immutable;
	img_info_cube.width = 512;
	img_info_cube.height = 512;
	img_info_cube.depth = 1;
	img_info_cube.mip_count = 7;
	const ut::uint32 cubeface_size = img_info_cube.width * img_info_cube.height;
	
	ut::uint32 mip_offset = 0;
	for (ut::uint32 i = 0; i < 6; i++)
	{
		ut::uint32 mip_width = img_info_cube.width;
		ut::uint32 mip_height = img_info_cube.height;

		for (ut::uint32 m = 0; m < img_info_cube.mip_count; m++)
		{
			const ut::uint32 mip_size = mip_width * mip_height;

			img_info_cube.data.Resize(img_info_cube.data.GetSize() + mip_size * pixel::GetSize(pixel::r8g8b8a8));

			for (ut::uint32 j = 0; j < mip_height; j++)
			{
				for (ut::uint32 k = 0; k < mip_width; k++)
				{
					ut::Color<4, byte>* pixel_cube_data = reinterpret_cast<ut::Color<4, byte>*>(img_info_cube.data.GetAddress());

					int coef = i % 2 ? 2 : 1;

					ut::Color<4, byte> color(255 / coef, 255 / coef, 255 / coef, 255);

					switch (i)
					{
					case 0: color.G() = 0; color.B() = 0; break;
					case 1: color.G() = 0; color.B() = 0; break;
					case 2: color.R() = 0; color.B() = 0; break;
					case 3: color.R() = 0; color.B() = 0; break;
					case 4: color.R() = 0; color.G() = 0; break;
					case 5: color.R() = 0; color.G() = 0; break;
					}

					if (j < k)
					{
						color.R() /= 2;
						color.G() /= 2;
						color.B() /= 2;
					}

					const ut::uint32 id = mip_offset + j * mip_width + k;
					pixel_cube_data[id] = color;
				}
			}

			mip_offset += mip_size;
			mip_width /= 2;
			mip_height /= 2;
		}
	}
	ut::Result<Image, ut::Error> imgcube_result = device.CreateImage(ut::Move(img_info_cube));
	img_cube = ut::MakeUnique<Image>(imgcube_result.MoveOrThrow());

	// img_3d
	Image::Info img_info_3d;
	img_info_3d.type = Image::type_3D;
	img_info_3d.format = pixel::r8g8b8a8;
	img_info_3d.usage = render::memory::immutable;
	img_info_3d.width = 4;
	img_info_3d.height = 2;
	img_info_3d.depth = 2;
	img_info_3d.mip_count = 1;
	img_info_3d.data.Resize(img_info_3d.width * img_info_3d.height * img_info_3d.depth * pixel::GetSize(pixel::r8g8b8a8));
	ut::Color<4, byte>* pixel_3d = reinterpret_cast<ut::Color<4, byte>*>(img_info_3d.data.GetAddress());
	for (ut::uint32 i = 0; i < img_info_3d.depth; i++)
	{
		for (ut::uint32 j = 0; j < img_info_3d.height; j++)
		{
			for (ut::uint32 k = 0; k < img_info_3d.width; k++)
			{
				const ut::uint32 id = i * img_info_3d.height * img_info_3d.width + j * img_info_3d.width + k;
				pixel_3d[id] = ut::Color<4, byte>(255 / (i + 1), 255 / (j + 1), 255 / (k + 1), 255);
			}
		}
	}
	ut::Result<Image, ut::Error> img3d_result = device.CreateImage(ut::Move(img_info_3d));
	img_3d = ut::MakeUnique<Image>(img3d_result.MoveOrThrow());


	// sampler
	Sampler::Info sampler_info;
	sampler_info.mag_filter = Sampler::filter_linear;
	sampler_info.min_filter = Sampler::filter_linear;
	sampler_info.mip_filter = Sampler::filter_linear;
	sampler_info.address_u = Sampler::address_clamp;
	sampler_info.address_v = Sampler::address_clamp;
	sampler_info.address_w = Sampler::address_clamp;
	sampler_info.compare_op = compare::always;
	sampler_info.border_color = ut::Color<4, float>(0.0f);
	sampler_info.anisotropy_enable = false;
	sampler_info.max_anisotropy = 1.0f;
	sampler_info.mip_lod_bias = 0.0f;
	sampler_info.min_lod = 0.0f;
	sampler_info.max_lod = 4096.0f;
	ut::Result<Sampler, ut::Error> sampler_result = device.CreateSampler(sampler_info);
	linear_sampler = ut::MakeUnique<Sampler>(sampler_result.MoveOrThrow());

	// initialize per-frame data
	for (size_t i = 0; i < config.frames_in_flight; i++)
	{
		ut::Result<Frame, ut::Error> frame = CreateFrame();
		if (!frames.Add(frame.MoveOrThrow()))
		{
			throw ut::Error(ut::error::out_of_memory);
		}
	}

	// save shader cache
	shader_loader.SaveCache();
}

// Renders the whole environment to the internal images and presents
// the result to user.
void Engine::ProcessNextFrame()
{
	// wrap frame id
	if (current_frame_id >= frames.GetNum())
	{
		current_frame_id = 0;
	}

	// get current frame
	Frame& frame = frames[current_frame_id];

	// wait for the previous frame to finish
	device.WaitCmdBuffer(frame.cmd_buffer);

	// execute viewport tasks (resize, close, etc.)
	ProcessViewportEvents();

	// get active viewports
	ut::Array< ut::Ref<ViewportContainer> > active_viewports;
	for (size_t i = 0; i < viewports.GetNum(); i++)
	{
		ui::PlatformViewport& viewport = viewports[i].Get<ui::PlatformViewport&>();
		if (viewport.IsActive())
		{
			active_viewports.Add(viewports[i]);
		}
	}

	// generate an array of active displays
	ut::Array< ut::Ref<Display> > display_array;
	for (size_t i = 0; i < active_viewports.GetNum(); i++)
	{
		display_array.Add(active_viewports[i]->Get<Display>());
		device.AcquireNextDisplayBuffer(display_array.GetLast());
	}

	// record all commands for the current frame
	device.Record(frame.cmd_buffer, [&](Context& context) { RecordFrameCommands(context, active_viewports); });

	// submit commands and enqueue display presentation
	device.Submit(frame.cmd_buffer, display_array);

	// go to the next frame
	current_frame_id++;
}

// Function for recording all commands needed to draw current frame.
void Engine::RecordFrameCommands(Context& context, ut::Array< ut::Ref<ViewportContainer> >& active_viewports)
{
	// get current frame
	Frame& frame = frames[current_frame_id];

	for (size_t i = 0; i < active_viewports.GetNum(); i++)
	{
		Display& display = active_viewports[i]->Get<Display>();
		RenderPass& rp = active_viewports[i]->Get<RenderPass>();
		PipelineState& pipeline_state = active_viewports[i]->Get<PipelineState>();

		static int it = 0;
		it++;
		bool swap_col = it % 100 < 50;
		if (swap_col)
		{
			switch (i)
			{
			case 0: backbuffer_clear_values[0] = ut::Color<4>(1, 0, 0, 1); break;
			case 1: backbuffer_clear_values[0] = ut::Color<4>(0, 1, 0, 1); break;
			case 2: backbuffer_clear_values[0] = ut::Color<4>(0, 0, 1, 1); break;
			case 3: backbuffer_clear_values[0] = ut::Color<4>(1, 1, 0, 1); break;
			}
		}
		else
		{
			switch (i)
			{
			case 0: backbuffer_clear_values[0] = ut::Color<4>(0, 0, 1, 1); break;
			case 1: backbuffer_clear_values[0] = ut::Color<4>(1, 0, 1, 1); break;
			case 2: backbuffer_clear_values[0] = ut::Color<4>(1, 0, 0, 1); break;
			case 3: backbuffer_clear_values[0] = ut::Color<4>(0, 1, 1, 1); break;
			}
		}

		ut::Array<Framebuffer>& framebuffers = active_viewports[i]->Get< ut::Array<Framebuffer> >();

		Framebuffer& framebuffer = framebuffers[display.GetCurrentBufferId()];
		const Framebuffer::Info& fb_info = framebuffer.GetInfo();
		ut::Rect<ut::uint32> render_area(0, 0, fb_info.width, fb_info.height);

		// update uniform buffer
		ut::Color<4> display_color(1, 0, swap_col ? 1 : 0, 1);
		UpdateBuffer(context, frame.display_ub, &display_color);

		// draw quad
		frame.quad_desc_set.ub.BindUniformBuffer(frame.display_ub);
		frame.quad_desc_set.tex1d.BindImage(img_1d.GetRef());
		frame.quad_desc_set.tex2d.BindImage(img_2d.GetRef());
		frame.quad_desc_set.tex_cube.BindImage(img_cube.GetRef());
		frame.quad_desc_set.tex3d.BindImage(img_3d.GetRef());
		frame.quad_desc_set.tex_cube_face.BindCubeFace(img_cube.GetRef(), Image::Cube::positive_x, 0);
		frame.quad_desc_set.tex_cube_face.BindCubeFace(img_cube.GetRef(), Image::Cube::negative_x, 1);
		frame.quad_desc_set.tex_cube_face.BindCubeFace(img_cube.GetRef(), Image::Cube::positive_y, 2);
		frame.quad_desc_set.tex_cube_face.BindCubeFace(img_cube.GetRef(), Image::Cube::negative_y, 3);
		frame.quad_desc_set.tex_cube_face.BindCubeFace(img_cube.GetRef(), Image::Cube::positive_z, 4);
		frame.quad_desc_set.tex_cube_face.BindCubeFace(img_cube.GetRef(), Image::Cube::negative_z, 5);
		frame.quad_desc_set.sampler.BindSampler(linear_sampler.GetRef());
		context.BeginRenderPass(rp, framebuffer, render_area, backbuffer_clear_values);
		context.BindPipelineState(pipeline_state);
		context.BindDescriptorSet(frame.quad_desc_set);
		context.BindVertexBuffer(screen_space_quad.GetRef(), 0);
		context.Draw(4, 0);

		context.EndRenderPass();
	}
}

// Executes viewport tasks (resize, close, etc.) in a safe manner.
void Engine::ProcessViewportEvents()
{
	// synchronize viewport events in this point
	SyncViewportEvents();

	// execute viewport tasks
	if (HasPendingViewportTasks())
	{
		// all frames in flight must be finished to prevent
		// damaging resources bound to the rendering pipeline
		for (size_t i = 0; i < frames.GetNum(); i++)
		{
			device.WaitCmdBuffer(frames[i].cmd_buffer);
		}

		// viewport tasks can now be executed safely
		ExecuteViewportTasks();
	}
}

// Updates buffer contents with provided data. Can be used as a convenient
// wrapper around MapBuffer + UnmapBuffer functions. Note that buffer must be
// created with  ve::render::Buffer::gpu_cpu flag to be compatible with this
// function.
//    @param context - reference to the ve::render::Context object to perform
//                     perform update operation.
//    @param buffer - reference to the ve::render::Buffer object to be updated.
//    @param data - reference to the ve::render::Buffer object to be updated.
ut::Optional<ut::Error> Engine::UpdateBuffer(Context& context, Buffer& buffer, const void* data)
{
	ut::Result<void*, ut::Error> map_result = context.MapBuffer(buffer, ut::access_write);
	if (!map_result)
	{
		return map_result.MoveAlt();
	}

	ut::memory::Copy(map_result.Get(), data, buffer.GetInfo().size);

	context.UnmapBuffer(buffer);

	return ut::Optional<ut::Error>();
}

// Creates a set of frame data.
//    @return - ve::render::Frame object or ut::Error if failed.
ut::Result<Frame, ut::Error> Engine::CreateFrame()
{
	// initialize command buffer info
	CmdBuffer::Info cmd_buffer_info;
	cmd_buffer_info.usage = CmdBuffer::usage_dynamic;

	// create command buffer
	ut::Result<CmdBuffer, ut::Error> cmd_buffer = device.CreateCmdBuffer(cmd_buffer_info);
	if (!cmd_buffer)
	{
		return ut::MakeError(cmd_buffer.MoveAlt());
	}

	// create display uniform buffer
	Buffer::Info buffer_info;
	buffer_info.type = Buffer::uniform;
	buffer_info.usage = render::memory::gpu_cpu;
	buffer_info.size = sizeof(ut::Color<4>);
	buffer_info.data.Resize(buffer_info.size);
	ut::Color<4>* color = reinterpret_cast<ut::Color<4>*>(buffer_info.data.GetAddress());
	*color = ut::Color<4>(1, 0, 1, 0);
	ut::Result<Buffer, ut::Error> display_ub = device.CreateBuffer(ut::Move(buffer_info));
	if (!display_ub)
	{
		return ut::MakeError(display_ub.MoveAlt());
	}
	
	// create frame
	Frame frame(cmd_buffer.Move(), display_ub.Move());

	// connect descriptors
	frame.quad_desc_set.Connect(display_shader.GetRef());

	// success
	return frame;
}

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//