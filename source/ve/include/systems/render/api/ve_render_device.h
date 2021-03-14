//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "systems/render/api/ve_render_platform.h"
#include "systems/render/api/ve_render_context.h"
#include "systems/render/api/ve_render_image.h"
#include "systems/render/api/ve_render_sampler.h"
#include "systems/render/api/ve_render_target.h"
#include "systems/render/api/ve_render_display.h"
#include "systems/render/api/ve_render_cmd_buffer.h"
#include "systems/render/api/ve_render_framebuffer.h"
#include "systems/render/api/ve_render_pass.h"
#include "systems/render/api/ve_render_buffer.h"
#include "systems/render/api/ve_render_shader.h"
#include "systems/render/api/ve_render_pipeline_state.h"
#include "systems/render/api/ve_render_descriptor.h"
#include "systems/ui/ve_ui.h"
#include "ve_dedicated_thread.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// ve::render::Device class creates rendering resources.
class Device : private PlatformDevice
{
public:
	// Type of a special thread for rendering.
	typedef DedicatedThread<Device> Thread;

	// Information about the physical device limits and features.
	struct Info
	{
		size_t max_uniform_buffer_size = 0;
		size_t uniform_buffer_padding = 16;
		size_t max_storage_buffer_size = 0;
		ut::uint32 max_1D_image_dimension = 0;
		ut::uint32 max_2D_image_dimension = 0;
		ut::uint32 max_3D_image_dimension = 0;
		ut::uint32 max_cube_image_dimension = 0;
		ut::uint32 max_image_array_size = 0;
		bool supports_geometry_shader = false;
		bool supports_tesselation_shader = false;
		bool supports_wide_lines = false;

		// true if Context::MapBuffer() and Context::MapImage() are thread-safe
		bool supports_async_rc_mapping = false;

		// true if Context::DrawInstanced() and Context::DrawIndexInstanced()
		// affect SV_InstanceID starting value
		bool supports_sv_instance_offset = false;
	};

	// Constructor.
	Device(ut::SharedPtr<ui::Frontend::Thread> ui_frontend);

	// Move constructor.
	Device(Device&&) noexcept;

	// Move operator.
	Device& operator =(Device&&) noexcept;

	// Copying is prohibited.
	Device(const Device&) = delete;
	Device& operator =(const Device&) = delete;

	// Creates a new image.
	//    @param info - reference to the Image::Info object describing an image.
	//    @return - new image object of error if failed.
	ut::Result<Image, ut::Error> CreateImage(Image::Info info);

	// Creates a new sampler.
	//    @param info - reference to the Sampler::Info object describing a sampler.
	//    @return - new sampler object of error if failed.
	ut::Result<Sampler, ut::Error> CreateSampler(const Sampler::Info& info);

	// Creates a new render target.
	//    @param info - reference to the Target::Info object describing a target.
	//    @return - new render target object of error if failed.
	ut::Result<Target, ut::Error> CreateTarget(const Target::Info& info);

	// Creates platform-specific representation of the rendering area inside a UI viewport.
	//    @param viewport - reference to UI viewport containing rendering area.
	//    @param vsync - boolean whether to enable vertical synchronization or not.
	//    @return - new display object or error if failed.
	ut::Result<Display, ut::Error> CreateDisplay(ui::PlatformViewport& viewport, bool vsync);

	// Creates an empty command buffer.
	//    @param cmd_buffer_info - reference to the information about
	//                             the command buffer to be created.
	//    @return - new command buffer or error if failed.
	ut::Result<CmdBuffer, ut::Error> CreateCmdBuffer(const CmdBuffer::Info& cmd_buffer_info);

	// Creates render pass object.
	//    @param in_color_slots - array of color slots.
	//    @param in_depth_stencil_slot - optional depth stencil slot.
	//    @return - new render pass or error if failed.
	ut::Result<RenderPass, ut::Error> CreateRenderPass(ut::Array<RenderTargetSlot> in_color_slots,
	                                                   ut::Optional<RenderTargetSlot> in_depth_stencil_slot = ut::Optional<RenderTargetSlot>());

	// Creates a framebuffer. All targets must have the same width and height.
	//    @param render_pass - const reference to the renderpass to be bound to.
	//    @param color_targets - array of references to the colored render
	//                           targets to be bound to a render pass.
	//    @param depth_stencil_target - optional reference to the depth-stencil
	//                                  target to be bound to a render pass.
	//    @return - new framebuffer or error if failed.
	ut::Result<Framebuffer, ut::Error> CreateFramebuffer(const RenderPass& render_pass,
	                                                     ut::Array< ut::Ref<Target> > color_targets,
	                                                     ut::Optional<Target&> depth_stencil_target = ut::Optional<Target&>());

	// Creates a buffer.
	//    @param info - ve::render::Buffer::Info object to initialize a buffer with.
	//    @return - new shader or error if failed.
	ut::Result<Buffer, ut::Error> CreateBuffer(Buffer::Info info);

	// Creates a shader.
	//    @param info - ve::render::Shader::Info object to initialize a shader with.
	//    @return - new shader or error if failed.
	ut::Result<Shader, ut::Error> CreateShader(Shader::Info info);

	// Creates a pipeline state.
	//    @param info - ve::render::PipelineState::Info object to
	//                  initialize a pipeline with.
	//    @param render_pass - reference to the ve::render::RenderPass object
	//                         pipeline will be bound to.
	//    @return - new pipeline sate or error if failed.
	ut::Result<PipelineState, ut::Error> CreatePipelineState(PipelineState::Info info, RenderPass& render_pass);

	// Resets given command buffer. This command buffer must be created without
	// CmdBufferInfo::usage_dynamic flag (use ResetCmdPool() instead).
	//    @param cmd_buffer - reference to the buffer to be reset.
	void ResetCmdBuffer(CmdBuffer& cmd_buffer);

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
	void Record(CmdBuffer& cmd_buffer,
	            ut::Function<void(Context&)> function,
	            ut::Optional<RenderPass&> render_pass = ut::Optional<RenderPass&>(),
	            ut::Optional<Framebuffer&> framebuffer = ut::Optional<Framebuffer&>());

	// Waits for all commands from the provided buffer to be executed.
	//    @param cmd_buffer - reference to the command buffer.
	void WaitCmdBuffer(CmdBuffer& cmd_buffer);

	// Submits a command buffer to a queue. Also it's possible to enqueue presentation
	// for displays used in the provided command buffer. Presentation is supported
	// only for command buffers created with CmdBufferInfo::usage_dynamic flag.
	//    @param cmd_buffer - reference to the command buffer to enqueue.
	//    @param present_queue - array of references to displays waiting for their
	//                           buffer to be presented to user.
	void Submit(CmdBuffer& cmd_buffer,
	            ut::Array< ut::Ref<Display> > present_queue = ut::Array< ut::Ref<Display> >());

	// Acquires next buffer in a swapchain to be filled.
	//    @param display - reference to the display.
	void AcquireNextDisplayBuffer(Display& display);

	// Call this function to wait on the host for the completion of all
	// queue operations for all queues on this device.
	void WaitIdle();

	// Returns a const reference to the object with
	// information about this device.
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
