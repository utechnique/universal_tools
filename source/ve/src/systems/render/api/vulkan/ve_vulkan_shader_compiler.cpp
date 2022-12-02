//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#if VE_VULKAN
//----------------------------------------------------------------------------//
#include "systems/render/api/ve_render_shader_compiler.h"

#undef min
#undef max

#include <shaderc/shaderc.hpp>
#include <spirv_cross/spirv_cross.hpp>

//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// Converts from ve::render::Shader::Type to shaderc_shader_kind
shaderc_shader_kind ConvertShaderTypeToShaderc(Shader::Stage stage)
{
	switch (stage)
	{
	case Shader::vertex: return shaderc_glsl_vertex_shader;
	case Shader::hull: return shaderc_glsl_tess_control_shader;
	case Shader::domain: return shaderc_glsl_tess_evaluation_shader;
	case Shader::geometry: return shaderc_glsl_geometry_shader;
	case Shader::pixel: return shaderc_glsl_fragment_shader;
	case Shader::compute: return shaderc_glsl_compute_shader;
	}
	return shaderc_glsl_default_vertex_shader;
}

//----------------------------------------------------------------------------//
// ve::render::FileIncluder implements shaderc IncluderInterface to include
// shader files.
class ShaderFileIncluder : public shaderc::CompileOptions::IncluderInterface
{
public:
	// Default constructor.
	ShaderFileIncluder() = default;

	// Destructor.
	~ShaderFileIncluder() override = default;

	// Generates an error with @message text.
	static shaderc_include_result* MakeErrorIncludeResult(const char* message)
	{
		return new shaderc_include_result{ "", 0, message, strlen(message) };
	}

	// Resolves a requested source file of a given type from a requesting
	// source into a shaderc_include_result whose contents will remain valid
	// until it's released.
	shaderc_include_result* GetInclude(const char* requested_source,
	                                   shaderc_include_type include_type,
	                                   const char* requesting_source,
	                                   size_t) override
	{
		// allocate memory for the new include file
		FileInfo* info = new FileInfo;

		// add working directory name in front of the path in
		// case if requesting source is relative
		if (include_type == shaderc_include_type_relative)
		{
			info->full_path = ut::String(requesting_source);
			info->full_path.IsolateLocation(true);
		}

		// add included filename to the final path
		info->full_path += ut::String(requested_source);

		// read file contents
		ut::Result<ut::Array<ut::byte>, ut::Error> read_result = ut::ReadFile(info->full_path);
		if (!read_result)
		{
			return MakeErrorIncludeResult("Cannot open file");
		}

		// move file contents to the managing object
		info->contents = read_result.Move();

		// generate and return shaderc_include_result object
		return new shaderc_include_result { info->full_path.GetAddress(),
		                                    info->full_path.Length(),
		                                    reinterpret_cast<const char*>(info->contents.GetAddress()),
		                                    info->contents.Count(),
		                                    info };
	}

	// Releases an include result.
	void ReleaseInclude(shaderc_include_result* include_result) override
	{
		FileInfo* info = static_cast<FileInfo*>(include_result->user_data);
		delete info;
		delete include_result;
	}

private:
	// The full path and content of a source file. This data must be valid
	// until ReleaseInclude() is called.
	struct FileInfo
	{
		ut::String full_path;
		ut::Array<ut::byte> contents;
	};
};

//----------------------------------------------------------------------------//
// Wrapper around spirv_cross::ShaderResources to conveniently reflect
// resources to ve::render::Shader::Parameter.
struct SpirvCrossResources
{
	const Shader::Parameter::Type type;
	const spirv_cross::SmallVector<spirv_cross::Resource>& slots;
};

//----------------------------------------------------------------------------//
// Constructor.
PlatformShaderCompiler::PlatformShaderCompiler()
{}

// Move constructor.
PlatformShaderCompiler::PlatformShaderCompiler(PlatformShaderCompiler&&) noexcept = default;

// Move operator.
PlatformShaderCompiler& PlatformShaderCompiler::operator =(PlatformShaderCompiler&&) noexcept = default;

//----------------------------------------------------------------------------//
// Compiles a shader from memory.
//    @param stage - type of the shader (vertex/pixel/geometry etc.)
//    @param shader_name - string with the name of this
//                         particular shader build.
//    @param entry_point - string with a name of entry point.
//    @param code - const string with the code of the shader.
//    @param macros - preprocessor macros to build shader with.
//    @return - Shader::Info object or ut::Error if failed.
ut::Result<Shader::Info, ut::Error> ShaderCompiler::Compile(Shader::Stage stage,
	                                                        ut::String shader_name,
	                                                        ut::String entry_point,
	                                                        const ut::String& code,
	                                                        Shader::Macros macros)
{
	// initialize shaderc compiler
	shaderc::Compiler compiler;
	shaderc::CompileOptions options;

	// hlsl is the only shader language supported
	options.SetSourceLanguage(shaderc_source_language_hlsl);

	// assign include handler
	std::unique_ptr<shaderc::CompileOptions::IncluderInterface> includer(new ShaderFileIncluder());
	options.SetIncluder(std::move(includer));

	// add macros
	const size_t macro_count = macros.Count();
	for (size_t i = 0; i < macro_count; i++)
	{
		const Shader::MacroDefinition& macro = macros[i];
		options.AddMacroDefinition(macro.name.GetAddress(),
		                           macro.name.Length(),
		                           macro.value.GetAddress(),
		                           macro.value.Length());
	}

	// compile
	shaderc::SpvCompilationResult result = compiler.CompileGlslToSpv(code.GetAddress(),
	                                                                 code.Length(),
	                                                                 ConvertShaderTypeToShaderc(stage),
	                                                                 shader_name.GetAddress(),
	                                                                 entry_point.GetAddress(),
	                                                                 options);
	if (result.GetCompilationStatus() != shaderc_compilation_status_success)
	{
		std::string error_message = result.GetErrorMessage();
		ut::log.Lock() << "Shader compiler: " << error_message.c_str() << ut::cret;
		return ut::MakeError(ut::error::fail);
	}

	// create shader info
	Shader::Info info;
	info.stage = stage;
	info.name = ut::Move(shader_name);
	info.entry_point = ut::Move(entry_point);
	info.macros = ut::Move(macros);

	// copy shader bytecode
	const uint32_t* start = result.cbegin();
	const size_t size = result.cend() - start;
	info.bytecode.Resize(size * sizeof(uint32_t));
	ut::memory::Copy(info.bytecode.GetAddress(), start, size * sizeof(uint32_t));

	// initialize spirv_cross compiler to make reflection possible
	spirv_cross::Compiler comp(reinterpret_cast<const uint32_t*>(info.bytecode.GetAddress()), size);

	// get reflection data
	auto active = comp.get_active_interface_variables();
	spirv_cross::ShaderResources shader_resources = comp.get_shader_resources(active);
	comp.set_enabled_interface_variables(move(active));

	// initialize separate groups of reflected resources
	ut::Array<SpirvCrossResources> spirv_resources;
	spirv_resources.Add(SpirvCrossResources{ Shader::Parameter::uniform_buffer, shader_resources.uniform_buffers });
	spirv_resources.Add(SpirvCrossResources{ Shader::Parameter::image, shader_resources.separate_images });
	spirv_resources.Add(SpirvCrossResources{ Shader::Parameter::sampler, shader_resources.separate_samplers });
	spirv_resources.Add(SpirvCrossResources{ Shader::Parameter::storage_buffer, shader_resources.storage_buffers });

	// initialize shader parameters (name and binding)
	for (size_t rc_id = 0; rc_id < spirv_resources.Count(); rc_id++)
	{
		SpirvCrossResources& resources = spirv_resources[rc_id];
		Shader::Parameter::Type type = resources.type;

		size_t slot_count = resources.slots.size();
		for (size_t slot_id = 0; slot_id < slot_count; slot_id++)
		{
			const spirv_cross::Resource& slot = resources.slots[slot_id];

			// name and binding
			ut::String name(slot.name.c_str());
			ut::uint32 binding = comp.get_decoration(slot.id, spv::DecorationBinding);

			// array traits
			const spirv_cross::SPIRType &spirv_type = comp.get_type(slot.type_id);
			const size_t dimensions = spirv_type.array.size();
			ut::Array<ut::uint32> array_dim(dimensions);
			for (ut::uint32 dim_id = 0; dim_id < dimensions; dim_id++)
			{
				array_dim[dim_id] = spirv_type.array[dim_id];
			}

			// add parameter
			info.parameters.Add(Shader::Parameter(type, ut::Move(name), binding, ut::Move(array_dim)));
		}
	}

	// success
	return info;
}

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif // VE_VULKAN
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
