//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "systems/render/ve_render_api.h"
#include "systems/render/engine/ve_render_image_loader.h"
#include "systems/render/engine/ve_render_frame.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// ve::render::ResourceManager is a helper class to conveniently operate with
// render resources.
class ResourceManager
{
public:
	// Constructor.
	ResourceManager(Device& device_ref) noexcept;

	// Updates buffer contents with provided data. Can be used as a convenient
	// wrapper around MapBuffer + UnmapBuffer functions. Note that buffer must be
	// created with memory usage flag that provides appropriate kind of cpu access.
	//    @param context - reference to the ve::render::Context object to perform
	//                     perform update operation.
	//    @param buffer - reference to the ve::render::Buffer object to be updated.
	//    @param data - reference to the ve::render::Buffer object to be updated.
	ut::Optional<ut::Error> UpdateBuffer(Context& context,
	                                     Buffer& buffer,
	                                     const void* data);

	// Creates vertex buffer representing a fullscreen quad.
	ut::Result<Buffer, ut::Error> CreateFullscreenQuad();

private:
	Device& device;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
