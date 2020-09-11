//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "systems/render/ve_render_api.h"
#include "ve_shader_cache.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// ve::render::ShaderLoader encapsulates loading shaders from file.
class ShaderLoader
{
public:
	// Constructor.
	ShaderLoader(Device& device_ref) noexcept;

	// Compiles a shader from file. Returns cached version if source didn't
	// change or compiles it from the scratch otherwise.
	//    @param stage - type of the shader (vertex/pixel/geometry etc.).
	//    @param shader_name - string with the name of this
	//                         particular shader build.
	//    @param entry_point - string with a name of entry point.
	//    @param filename - const string with the name of shader file, can be
	//                      relative to ve::directories::skRc directory.
	//    @param macros - preprocessor macros to build shader with.
	//    @return - Shader::Info object or ut::Error if failed.
	ut::Result<Shader, ut::Error> Load(Shader::Stage stage,
	                                   ut::String shader_name,
	                                   ut::String entry_point,
	                                   const ut::String& filename,
	                                   Shader::Macros macros = Shader::Macros());

	// Saves cache to the default path.
	ut::Optional<ut::Error> SaveCache();

private:
	// render device
	Device& device;

	// shader cache optimizes loading
	ShaderCache cache;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
