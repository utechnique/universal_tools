//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "systems/render/api/ve_render_platform.h"
#include "systems/render/api/ve_render_context.h"
#include "systems/render/api/ve_render_target.h"
#include "systems/render/api/ve_render_display.h"
#include "systems/render/api/ve_render_cmd_buffer.h"
#include "systems/render/api/ve_render_framebuffer.h"
#include "systems/render/api/ve_render_pass.h"
#include "systems/ui/desktop/ve_desktop_viewport.h"
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

	// Constructor.
	Device(ut::SharedPtr<ui::Frontend::Thread> ui_frontend);

	// Move constructor.
	Device(Device&&) noexcept;

	// Move operator.
	Device& operator =(Device&&) noexcept;

	// Copying is prohibited.
	Device(const Device&) = delete;
	Device& operator =(const Device&) = delete;

	// Creates new texture.
	//    @param format - texture format, see ve::render::pixel::Format.
	//    @param width - width of the texture in pixels.
	//    @param height - height of the texture in pixels.
	//    @return - new texture object of error if failed.
	ut::Result<Texture, ut::Error> CreateTexture(pixel::Format format,
	                                             ut::uint32 width,
	                                             ut::uint32 height);

	// Creates platform-specific representation of the rendering area inside a UI viewport.
	//    @param viewport - reference to UI viewport containing rendering area.
	//    @param vsync - boolean whether to enable vertical synchronization or not.
	//    @return - new display object or error if failed.
	ut::Result<Display, ut::Error> CreateDisplay(ui::PlatformViewport& viewport, bool vsync);

	// Creates an empty command buffer.
	//    @param cmd_buffer_info - reference to the information about
	//                             the command buffer to be created.
	//    @return - new command buffer or error if failed.
	ut::Result<CmdBuffer, ut::Error> CreateCmdBuffer(const CmdBufferInfo& cmd_buffer_info);

	// Creates render pass object.
	//    @param in_color_slots - array of color slots.
	//    @param in_depth_stencil_slot - optional depth stencil slot.
	//    @return - new render pass or error if failed.
	ut::Result<RenderPass, ut::Error> CreateRenderPass(ut::Array<RenderTargetSlot> in_color_slots,
	                                                   ut::Optional<RenderTargetSlot> in_depth_stencil_slot = ut::Optional<RenderTargetSlot>());

	// Creates framebuffer. All targets must have the same width and height.
	//    @param render_pass - const reference to the renderpass to be bound to.
	//    @param color_targets - array of references to the colored render
	//                           targets to be bound to a render pass.
	//    @param depth_stencil_target - optional reference to the depth-stencil
	//                                  target to be bound to a render pass.
	//    @return - new framebuffer or error if failed.
	ut::Result<Framebuffer, ut::Error> CreateFramebuffer(const RenderPass& render_pass,
	                                                     ut::Array< ut::Ref<Target> > color_targets,
	                                                     ut::Optional<Target&> depth_stencil_target = ut::Optional<Target&>());

	// Resets all command buffers created with CmdBufferInfo::usage_once flag
	// enabled. This call is required before re-recording such command buffers.
	// Usualy it's called once per frame.
	void ResetDynamicCmdPool();

	// Resets given command buffer. This command buffer must be created without
	// CmdBufferInfo::usage_once flag (use ResetCmdPool() instead).
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

	// Submits a command buffer to a queue. Also it's possible to enqueue presentation
	// for displays used in the provided command buffer. Presentation is supported
	// only for command buffers created with CmdBufferInfo::usage_once flag.
	//    @param cmd_buffer - reference to the command buffer to enqueue.
	//    @param present_queue - array of references to displays waiting for their
	//                           buffer to be presented to user. Pass empty array
	//                           if @cmd_buffer has no CmdBufferInfo::usage_once flag.
	void Submit(CmdBuffer& cmd_buffer,
	            ut::Array< ut::Ref<Display> > present_queue = ut::Array< ut::Ref<Display> >());

	// Call this function to wait on the host for the completion of all
	// queue operations for all queues on this device.
	void WaitIdle();
};

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
