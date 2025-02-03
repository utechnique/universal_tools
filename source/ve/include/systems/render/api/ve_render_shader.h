//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "systems/render/api/ve_render_platform.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// ve::render::Shader is a class encapsulating a shader program.
class Shader : public PlatformShader
{
	friend class Device;
	friend class Context;
public:
	// Total number of different shader stages
	static const ut::uint32 skStageCount = 6;

	// Shader stage type.
	enum class Stage : ut::uint32
	{
		vertex   = 0,
		hull     = 1,
		domain   = 2,
		geometry = 3,
		pixel    = 4,
		compute  = 5
	};

	// Macro definition utilized by preprocessor to compile a shader.
	struct MacroDefinition : public ut::meta::Reflective
	{
		void Reflect(ut::meta::Snapshot& snapshot);
		ut::String name;
		ut::String value;
	};
	typedef ut::Array<MacroDefinition> Macros;

	// Shader uniform.
	class Parameter : public ut::meta::Reflective
	{
	public:
		// Type of a shader parameter.
		enum class Type : ut::uint32
		{
			uniform_buffer,
			image,
			sampler,
			storage_buffer,
			unknown
		};

		// Constructor.
		Parameter(Type in_type = Type::unknown,
				  ut::String in_name = ut::String(),
				  ut::uint32 in_binding = 0,
		          ut::Array<ut::uint32> in_arr =
		          ut::Array<ut::uint32>()) : type(in_type)
			                               , name(ut::Move(in_name))
			                               , binding(in_binding)
		                                   , array_dim(ut::Move(in_arr))
		{}

		// Registers shader info into reflection tree.
		//    @param snapshot - reference to the reflection tree
		void Reflect(ut::meta::Snapshot& snapshot);

		// Returns a type of this uniform.
		Parameter::Type GetType() const
		{
			return static_cast<Parameter::Type>(type);
		}

		// Returns the name of this uniform.
		const ut::String& GetName() const
		{
			return name;
		}

		// Returns binding id of this uniform.
		ut::uint32 GetBinding() const
		{
			return binding;
		}

		// Returns a number of dimensions, '0' means a single variable,
		// otherwise it's a number of dimensions in an array
		ut::uint32 GetArrayDimensions() const
		{
			return static_cast<ut::uint32>(array_dim.Count());
		}

		// Returns a number of elements in a desired array dimension.
		ut::uint32 GetArrayElementCount(ut::uint32 dimension) const
		{
			UT_ASSERT(dimension < array_dim.Count());
			return array_dim[dimension];
		}

		// Returns a total number of single elements in this parameter
		ut::uint32 GetElementCount() const;

	private:
		// type of the parameter, see ve::render::Shader::Parameter::Type
		Type type;

		// name of the parameter how it's defined in a shader code
		ut::String name;

		// binding id
		ut::uint32 binding;

		// number of elements in this array represents a number of dimensions,
		// '0' means a single variable, otherwise it's an array, each element
		// is a number of elements in appropriate dimension
		ut::Array<ut::uint32> array_dim;
	};

	// Info structure fully describing a shader.
	struct Info : public ut::meta::Reflective
	{
		// Registers shader info into reflection tree.
		//    @param snapshot - reference to the reflection tree
		void Reflect(ut::meta::Snapshot& snapshot);

		// shader stage
		Stage stage;

		// unique shader name
		ut::String name;

		// name of the entry point
		ut::String entry_point;

		// set of macros that were used to compile this shader
		Shader::Macros macros;

		// shader bytecode
		ut::Array<ut::byte> bytecode;

		// array of uniforms used by this shader
		ut::Array<Parameter> parameters;

		// hash of the shader bytecode
		ut::String hash;
	};

	// Constructor.
	Shader(PlatformShader platform_shader, Shader::Info in_info);

	// Move constructor.
	Shader(Shader&&) noexcept;

	// Move operator.
	Shader& operator =(Shader&&) noexcept;

	// Copying is prohibited.
	Shader(const Shader&) = delete;
	Shader& operator =(const Shader&) = delete;

	// Returns a const reference to the object with
	// information about this shader.
	const Info& GetInfo() const
	{
		return info;
	}

private:
	Info info;
};

//----------------------------------------------------------------------------//
// ve::render::BoundShader encapsulates all shader stages in one object.
class BoundShader
{
public:
	// Constructor (vs->ps)
	BoundShader(Shader vs,
	            Shader ps)
	{
		stages[static_cast<size_t>(Shader::Stage::vertex)] = ut::Move(vs);
		stages[static_cast<size_t>(Shader::Stage::pixel)] = ut::Move(ps);
	}

	// Constructor (vs->>hs->ds->gs->ps)
	BoundShader(Shader vs,
	            Shader hs,
	            Shader ds,
	            Shader gs,
	            Shader ps)
	{
		stages[static_cast<size_t>(Shader::Stage::vertex)] = ut::Move(vs);
		stages[static_cast<size_t>(Shader::Stage::hull)] = ut::Move(hs);
		stages[static_cast<size_t>(Shader::Stage::domain)] = ut::Move(ds);
		stages[static_cast<size_t>(Shader::Stage::geometry)] = ut::Move(gs);
		stages[static_cast<size_t>(Shader::Stage::pixel)] = ut::Move(ps);
	}

	// Returns a reference to the shader bound to the corresponding
	// shader stage.
	ut::Optional<Shader&> GetShader(Shader::Stage stage)
	{
		ut::Optional<Shader>& shader = stages[static_cast<size_t>(stage)];
		if (shader)
		{
			return shader.Get();
		}
		return ut::Optional<Shader&>();
	}

	ut::Optional<Shader> stages[Shader::skStageCount];
};

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
