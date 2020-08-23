//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "systems/render/api/ve_render_shader_compiler.h"
#include "ve_default.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// Constructor.
ShaderCompiler::ShaderCompiler(PlatformShaderCompiler platform_compiler) : PlatformShaderCompiler(ut::Move(platform_compiler))
{}

// Move constructor.
ShaderCompiler::ShaderCompiler(ShaderCompiler&&) noexcept = default;

// Move operator.
ShaderCompiler& ShaderCompiler::operator =(ShaderCompiler&&) noexcept = default;

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//