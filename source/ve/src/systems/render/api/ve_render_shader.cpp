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
	snapshot.Add(reinterpret_cast<ut::uint32&>(type), "type");
	snapshot.Add(name, "name");
	snapshot.Add(binding, "binding");
	snapshot.Add(array_dim, "array_dim");
}

// Returns a total number of single elements in this parameter
ut::uint32 Shader::Parameter::GetElementCount() const
{
	ut::uint32 element_count = 1;

	const size_t array_dim_count = array_dim.Count();
	for (size_t i = 0; i < array_dim_count; i++)
	{
		element_count *= array_dim[i];
	}
	return element_count;
}

//----------------------------------------------------------------------------//
// Registers shader info into reflection tree.
//    @param snapshot - reference to the reflection tree
void Shader::Info::Reflect(ut::meta::Snapshot& snapshot)
{
	snapshot.Add(reinterpret_cast<ut::uint32&>(stage), "stage");
	snapshot.Add(name, "name");
	snapshot.Add(entry_point, "entry");
	snapshot.Add(ut::meta::Binary(bytecode), "bytecode");
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