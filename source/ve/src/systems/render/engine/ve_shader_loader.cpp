//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "systems/render/engine/ve_shader_loader.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// Constructor.
ShaderLoader::ShaderLoader(Device& device_ref) noexcept : device(device_ref)
{
	// load shader cache
	ut::time::Counter timer;
	timer.Start();
	cache.Load();
	ut::log.Lock() << "Shader cache loaded in " << timer.GetTime<ut::time::Unit::second>()
	               << "s." << ut::cret;
}

//----------------------------------------------------------------------------->
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
ut::Result<Shader, ut::Error> ShaderLoader::Load(Shader::Stage stage,
                                                 ut::String shader_name,
                                                 ut::String entry_point,
                                                 const ut::String& filename,
                                                 Shader::Macros macros)
{
	ut::Result<Shader::Info, ut::Error> result = cache.CompileFromFile(stage,
	                                                                   ut::Move(shader_name),
	                                                                   ut::Move(entry_point),
	                                                                   filename,
	                                                                   ut::Move(macros));
	if (!result)
	{
		return ut::MakeError(result.MoveAlt());
	}
	return device.CreateShader(result.Move());
}

//----------------------------------------------------------------------------->
// Returns const reference to the cache object.
ut::Optional<ut::Error> ShaderLoader::SaveCache()
{
	return cache.Save();
}

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//