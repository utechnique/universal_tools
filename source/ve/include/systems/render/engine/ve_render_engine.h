//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "systems/render/ve_render_api.h"
#include "ve_render_cfg.h"
#include "ve_render_viewport_mgr.h"
#include "ve_render_frame.h"
#include "ve_render_image_loader.h"
#include "ve_shader_loader.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// ve::render::Engine is a class that can render (visualize) entities. It must
// be used only in the thread it was created in.
class Engine : public ViewportManager
{
public:
	// Constructor.
	Engine(Device& render_device, ViewportManager viewport_mgr);

	// Renders the whole environment to the internal images and presents
	// the result to user.
	void ProcessNextFrame();

private:
	// Function for recording all commands needed to draw current frame.
	void RecordFrameCommands(Context& context, ut::Array< ut::Ref<ViewportContainer> >& active_viewports);

	// Executes viewport tasks (resize, close, etc.) in a safe manner.
	void ProcessViewportEvents();

	// Updates buffer contents with provided data. Can be used as a convenient
	// wrapper around MapBuffer + UnmapBuffer functions. Note that buffer must be
	// created with  ve::render::Buffer::gpu_cpu flag to be compatible with this
	// function.
	//    @param context - reference to the ve::render::Context object to perform
	//                     perform update operation.
	//    @param buffer - reference to the ve::render::Buffer object to be updated.
	//    @param data - reference to the ve::render::Buffer object to be updated.
	ut::Optional<ut::Error> UpdateBuffer(Context& context, Buffer& buffer, const void* data);

	// Creates a set of frame data.
	//    @return - ve::render::Frame object or ut::Error if failed.
	ut::Result<Frame, ut::Error> CreateFrame();

	// render configuration
	Config<Settings> config;

	// render device
	Device& device;	

	// loads images form file
	ImageLoader img_loader;

	// loads shaders form file
	ShaderLoader shader_loader;

	// per-frame data (frames in flight)
	ut::Array<Frame> frames;

	// index of the element in @frames array that will be
	// used to render current frame
	ut::uint32 current_frame_id;

	// test
	ut::Array< ut::Color<4> > backbuffer_clear_values;
	ut::UniquePtr<Buffer> screen_space_quad;
	ut::UniquePtr<Image> img_1d;
	ut::UniquePtr<Image> img_2d;
	ut::UniquePtr<Image> img_cube;
	ut::UniquePtr<Image> img_3d;
	ut::UniquePtr<Image> img_dynamic;
	ut::UniquePtr<Sampler> linear_sampler;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
