//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "systems/render/api/ve_render_shader.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// Registers shader info into reflection tree.
//    @param snapshot - reference to the reflection tree
void Shader::MacroDefinition::Reflect(ut::meta::Snapshot& snapshot)
{
	snapshot.Add(name, "name");
	snapshot.Add(value, "value");
}

//----------------------------------------------------------------------------//
// Registers shader info into reflection tree.
//    @param snapshot - reference to the reflection tree
void Shader::Parameter::Reflect(ut::meta::Snapshot& snapshot)
{
	snapshot.Add(type, "type");
	snapshot.Add(name, "name");
	snapshot.Add(binding, "binding");
}

//----------------------------------------------------------------------------//
// Registers shader info into reflection tree.
//    @param snapshot - reference to the reflection tree
void Shader::Info::Reflect(ut::meta::Snapshot& snapshot)
{
	snapshot.Add(stage, "stage");
	snapshot.Add(name, "name");
	snapshot.Add(entry_point, "entry");
	snapshot.Add(bytecode, "bytecode");
	snapshot.Add(parameters, "parameters");
	snapshot.Add(hash, "hash");
	snapshot.Add(macros, "macros");
}

//----------------------------------------------------------------------------//
// Constructor.
Shader::Shader(PlatformShader platform_shader,
               Shader::Info in_info) : PlatformShader(ut::Move(platform_shader))
                                     , info(ut::Move(in_info))
{}

// Move constructor.
Shader::Shader(Shader&&) noexcept = default;

// Move operator.
Shader& Shader::operator =(Shader&&) noexcept = default;

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//