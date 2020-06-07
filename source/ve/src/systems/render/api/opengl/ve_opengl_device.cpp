//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "systems/render/api/ve_render_device.h"
//----------------------------------------------------------------------------//
#if VE_OPENGL
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// Constructor.
PlatformDevice::PlatformDevice() : context(ut::MakeUnique<Context>(PlatformContext(CreateGLContextAndInitPlatform())))
{}

// Move constructor.
PlatformDevice::PlatformDevice(PlatformDevice&&) noexcept = default;

// Move operator.
PlatformDevice& PlatformDevice::operator =(PlatformDevice&&) noexcept = default;

// Presents final image to user.
//    @param window - reference to the destination window.
//    @param src_framebuffer - framebuffer to copy contents from.
//    @param src_attachment_id - id of the attachment in @src_framebuffer.
//    @param width - width of the backbuffer in pixels.
//    @param height - height of the backbuffer in pixels.
//    @param vsync - whether to use vertical synchronization.
void PlatformDevice::Present(OpenGLWindow& window,
                             GlRc<gl::framebuffer>::Handle src_framebuffer,
                             GLenum src_attachment_id,
                             GLint width,
                             GLint height,
                             bool vsync)
{
    // lock UI thread - singlethreaded UI like X11 can't present
    // simultaneously with processing other UI events
	ui::DisplayScopeLock ui_lock;

	// switch context using window's hdc
	ut::Optional<ut::Error> make_current_error = context->MakeCurrent(window);
	if (make_current_error)
	{
		throw ut::Error(make_current_error.Move());
	}

	// enable srgb
	glEnable(GL_FRAMEBUFFER_SRGB);

	// set backbuffer as a destination and texture of the
	// render target as a source
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glDrawBuffer(GL_FRONT);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, src_framebuffer);
	glReadBuffer(src_attachment_id);

	// copy buffer
	glBlitFramebuffer(0, 0, width, height,
		              0, 0, width, height,
		              GL_COLOR_BUFFER_BIT, GL_NEAREST);

	// swap back and front buffers and draw
	window.SwapBuffer(vsync);

	// set context back to windowless mode
	make_current_error = context->MakeCurrent();
	if (make_current_error)
	{
		throw ut::Error(make_current_error.Move());
	}

	// disable srgb
	glDisable(GL_FRAMEBUFFER_SRGB);
}

//----------------------------------------------------------------------------//
// Constructor.
Device::Device(ut::SharedPtr<ui::Frontend::Thread> ui_frontend)
{}

// Move constructor.
Device::Device(Device&&) noexcept = default;

// Move operator.
Device& Device::operator =(Device&&) noexcept = default;

// Creates new texture.
//    @param width - width of the texture in pixels.
//    @param height - height of the texture in pixels.
//    @return - new texture object of error if failed.
ut::Result<Texture, ut::Error> Device::CreateTexture(pixel::Format format, ut::uint32 width, ut::uint32 height)
{
	ImageInfo img_info;
	img_info.format = format;
	img_info.width = width;
	img_info.height = height;
	img_info.depth = 1;

	const GLenum gl_pixel_format = ConvertPixelFormatToOpenGL(img_info.format);
	GLuint texture_handle;
	glGenTextures(1, &texture_handle);
	glBindTexture(GL_TEXTURE_2D, texture_handle);
	glTexImage2D(GL_TEXTURE_2D, 0, gl_pixel_format, img_info.width, img_info.height, 0, gl_pixel_format, GL_UNSIGNED_BYTE, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	return Texture(texture_handle, img_info);
}

// Creates platform-specific representation of the rendering area inside a UI viewport.
//    @param viewport - reference to UI viewport containing rendering area.
//    @param vsync - boolean whether to enable vertical synchronization or not.
//    @return - new display object or error if failed.
ut::Result<Display, ut::Error> Device::CreateDisplay(ui::PlatformViewport& viewport, bool vsync)
{
	// get size of the viewport
	const ut::uint32 width = static_cast<ut::uint32>(viewport.w());
	const ut::uint32 height = static_cast<ut::uint32>(viewport.h());

	// create texture (backbuffer) for the render target
	ut::Result<Texture, ut::Error> texture = CreateTexture(pixel::r8g8b8a8, width, height);
	if (!texture)
	{
		return ut::MakeError(texture.MoveAlt());
	}

	// target info
	RenderTargetInfo info(RenderTargetInfo::usage_present);

	// create render target that will be associated with provided viewport
	Target target(PlatformRenderTarget(), texture.MoveResult(), info);

	// array of targets
	ut::Array<Target> display_targets;
	if (!display_targets.Add(ut::Move(target)))
	{
		return ut::MakeError(ut::error::out_of_memory);
	}

	// success
	return Display(PlatformDisplay(OpenGLWindow(viewport)), ut::Move(display_targets), width, height, vsync);
}

// Creates an empty command buffer.
//    @param cmd_buffer_info - reference to the information about
//                             the command buffer to be created.
//    @return - new command buffer or error if failed.
ut::Result<CmdBuffer, ut::Error> Device::CreateCmdBuffer(const CmdBufferInfo& cmd_buffer_info)
{
	return CmdBuffer(PlatformCmdBuffer(), cmd_buffer_info);
}

// Creates render pass object.
//    @param in_color_slots - array of color slots.
//    @param in_depth_stencil_slot - optional depth stencil slot.
//    @return - new render pass or error if failed.
ut::Result<RenderPass, ut::Error> Device::CreateRenderPass(ut::Array<RenderTargetSlot> in_color_slots,
                                                           ut::Optional<RenderTargetSlot> in_depth_stencil_slot)
{
	return RenderPass(PlatformRenderPass(), ut::Move(in_color_slots), ut::Move(in_depth_stencil_slot));
}

// Creates framebuffer. All targets must have the same width and height.
//    @param render_pass - const reference to the renderpass to be bound to.
//    @param color_targets - array of references to the colored render
//                           targets to be bound to a render pass.
//    @param depth_stencil_target - optional reference to the depth-stencil
//                                  target to be bound to a render pass.
//    @return - new framebuffer or error if failed.
ut::Result<Framebuffer, ut::Error> Device::CreateFramebuffer(const RenderPass& render_pass,
	                                                         ut::Array< ut::Ref<Target> > color_targets,
	                                                         ut::Optional<Target&> depth_stencil_target)
{
	// determine width and heights of the framebuffer in pixels
	ut::uint32 width;
	ut::uint32 height;
	if (color_targets.GetNum() != 0)
	{
		const Target& color_target = color_targets.GetFirst();
		width = color_target.buffer.GetInfo().width;
		height = color_target.buffer.GetInfo().height;
	}
	else if (depth_stencil_target)
	{
		const Target& ds_target = depth_stencil_target.Get();
		width = ds_target.buffer.GetInfo().width;
		height = ds_target.buffer.GetInfo().height;
	}
	else
	{
		return ut::MakeError(ut::Error(ut::error::invalid_arg, "OpenGL: no targets for the framebuffer."));
	}

	// check width and height of the color targets
	const size_t color_target_count = color_targets.GetNum();
	for (size_t i = 0; i < color_target_count; i++)
	{
		const ImageInfo& img_info = color_targets[i]->buffer.GetInfo();
		if (img_info.width != width || img_info.height != height)
		{
			return ut::MakeError(ut::Error(ut::error::invalid_arg, "OpenGL: different width/height for the framebuffer."));
		}
	}

	// check width and height of the depth target
	if (depth_stencil_target)
	{
		const ImageInfo& img_info = depth_stencil_target.Get().buffer.GetInfo();
		if (img_info.width != width || img_info.height != height)
		{
			return ut::MakeError(ut::Error(ut::error::invalid_arg, "OpenGL: different width/height for the framebuffer."));
		}
	}

	// initialize info
	FramebufferInfo info(width, height);

	// create opengl framebuffer
	GLuint framebuffer_handle;
	glGenFramebuffers(1, &framebuffer_handle);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_handle);

	// validate the number of color attachments
	const ut::uint32 color_attachment_count = static_cast<ut::uint32>(color_targets.GetNum());
	if (color_attachment_count > skMaxGlColorAttachments)
	{
		return ut::MakeError(ut::Error(ut::error::out_of_bounds, "OpenGL: too many color attachments"));
	}

	// bind color attachments
	for (ut::uint32 i = 0; i < color_attachment_count; i++)
	{
		glFramebufferTexture(GL_FRAMEBUFFER,
		                     GetColorAttachmentId(i),
		                     color_targets[i]->buffer.GetGlHandle(),
		                     0);
	}

	// bind depth attachment
	if (depth_stencil_target)
	{
		glFramebufferTexture(GL_FRAMEBUFFER_EXT,
		                     GL_DEPTH_ATTACHMENT,
		                     depth_stencil_target.Get().buffer.GetGlHandle(),
		                     0);
	}

	// success
	return Framebuffer(PlatformFramebuffer(framebuffer_handle), info, ut::Move(color_targets), ut::Move(depth_stencil_target));
}

// Resets all command buffers created with CmdBufferInfo::usage_once flag
// enabled. This call is required before re-recording such command buffers.
// Usualy it's called once per frame.
void Device::ResetDynamicCmdPool()
{}

// Resets given command buffer. This command buffer must be created without
// CmdBufferInfo::usage_once flag (use ResetCmdPool() instead).
//    @param cmd_buffer - reference to the buffer to be reset.
void Device::ResetCmdBuffer(CmdBuffer& cmd_buffer)
{}

// Records commands by calling a provided function.
//    @param cmd_buffer - reference to the buffer to record commands to.
//    @param function - function containing commands to record.
//    @param render_pass - optional reference to the current renderpass,
//                         this parameter applies only for secondary buffer
//                         with CmdBufferInfo::usage_inside_render_pass flag
//                         enabled, othwerwise it's ignored.
//    @param framebuffer - optional reference to the current framebuffer,
//                         this parameter applies only for secondary buffer
//                         with CmdBufferInfo::usage_inside_render_pass flag
//                         enabled, othwerwise it's ignored.
void Device::Record(CmdBuffer& cmd_buffer,
	                ut::Function<void(Context&)> function,
	                ut::Optional<RenderPass&> render_pass,
	                ut::Optional<Framebuffer&> framebuffer)
{
	cmd_buffer.proc = function;
}

// Submits a command buffer to a queue. Also it's possible to enqueue presentation
// for displays used in the provided command buffer. Presentation is supported
// only for command buffers created with CmdBufferInfo::usage_once flag.
//    @param cmd_buffer - reference to the command buffer to enqueue.
//    @param present_queue - array of references to displays waiting for their
//                           buffer to be presented to user. Pass empty array
//                           if @cmd_buffer has no CmdBufferInfo::usage_once flag.
void Device::Submit(CmdBuffer& cmd_buffer,
	                ut::Array< ut::Ref<Display> > present_queue)
{
	if (!cmd_buffer.proc)
	{
		throw ut::Error(ut::error::invalid_arg, "OpenGL: command buffer has no recorded commands.");
	}

	// execute recorded function
	ut::Function<void(Context&)>& procedure = cmd_buffer.proc.Get();
	procedure(context.GetRef());

	// present
	const ut::uint32 present_count = static_cast<ut::uint32>(present_queue.GetNum());
	for (size_t display_id = 0; display_id < present_count; display_id++)
	{
		Display& display = present_queue[display_id].Get();
		const ut::uint32 buffer_count = static_cast<ut::uint32>(display.targets.GetNum());
		for (size_t target_id = 0; target_id < buffer_count; target_id++)
		{
			// search for the present request by backbuffer id
			const GlRc<gl::texture>::Handle buffer_handle = display.targets[target_id].buffer.GetGlHandle();
			ut::Optional<PlatformContext::PresentRequest&> request = context->present_queue.Find(buffer_handle);
			if (!request)
			{
				continue;
			}

			// copy framebuffer contents to the display and swap buffers
			Present(display.window,
			        request.Get().framebuffer,
			        request.Get().attachment_id,
			        static_cast<GLint>(display.GetWidth()),
			        static_cast<GLint>(display.GetHeight()),
			        display.vsync);

			// only one present per display
			break;
		}
	}
	context->present_queue.Empty();
}

// Call this function to wait on the host for the completion of all
// queue operations for all queues on this device.
void Device::WaitIdle()
{
	glFlush();
}

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif // VE_OPENGL
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
