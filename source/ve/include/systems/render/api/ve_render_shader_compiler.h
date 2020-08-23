//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "systems/render/api/ve_render_shader.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// ve::render::ShaderCompiler compiles shader programs.
class ShaderCompiler : public PlatformShaderCompiler
{
	friend class Device;
	friend class Context;
public:
	// Constructor.
	ShaderCompiler(PlatformShaderCompiler platform_compiler = PlatformShaderCompiler());

	// Move constructor.
	ShaderCompiler(ShaderCompiler&&) noexcept;

	// Move operator.
	ShaderCompiler& operator =(ShaderCompiler&&) noexcept;

	// Copying is prohibited.
	ShaderCompiler(const ShaderCompiler&) = delete;
	ShaderCompiler& operator =(const ShaderCompiler&) = delete;

	// Compiles a shader from memory.
	//    @param stage - type of the shader (vertex/pixel/geometry etc.)
	//    @param shader_name - string with the name of this
	//                         particular shader build.
	//    @param entry_point - string with a name of entry point.
	//    @param code - const string with the code of the shader.
	//    @param macros - preprocessor macros to build shader with.
	//    @return - Shader::Info object or ut::Error if failed.
	ut::Result<Shader::Info, ut::Error> Compile(Shader::Stage stage,
	                                            ut::String shader_name,
	                                            ut::String entry_point,
	                                            const ut::String& code,
	                                            Shader::Macros macros);
};

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
