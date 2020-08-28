//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#if VE_DX11
//----------------------------------------------------------------------------//
#include "systems/render/api/ve_render_shader_compiler.h"
#include <d3dcompiler.h>
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
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

	// compilation flags
	DWORD flags = D3DCOMPILE_ENABLE_STRICTNESS;

	// convert dx11 macro structs
	ut::Array<D3D10_SHADER_MACRO> d3d_macros(macros.GetNum() + 1);
	for (UINT i = 0; i < macros.GetNum(); i++)
	{
		d3d_macros[i].Name = macros[i].name.GetAddress();
		d3d_macros[i].Definition = macros[i].value.GetAddress();
	}
	d3d_macros.GetLast().Name = nullptr;
	d3d_macros.GetLast().Definition = nullptr;

	// convert shader stage
	LPCSTR target = nullptr;
	switch (stage)
	{
	case Shader::vertex:   target = "vs_5_0"; break;
	case Shader::hull:     target = "hs_5_0"; break;
	case Shader::domain:   target = "ds_5_0"; break;
	case Shader::geometry: target = "gs_5_0"; break;
	case Shader::pixel:    target = "ps_5_0"; break;
	case Shader::compute:  target = "cs_5_0"; break;
	}

	// compile
	ID3DBlob* blob_out;
	ID3DBlob* blob_error;
	HRESULT result = D3DCompile(code.GetAddress(),
	                            code.Length(),
	                            nullptr,
	                            d3d_macros.GetAddress(),
	                            nullptr,
	                            entry_point.GetAddress(),
	                            target,
	                            flags,
	                            0,
	                            &blob_out,
	                            &blob_error );

	// validate compilation result
	if (FAILED(result))
	{
		if (blob_error != nullptr)
		{
			LPCSTR d3d_err_msg = static_cast<LPCSTR>(blob_error->GetBufferPointer());
			ut::String err_msg = ut::Print(result) + " failed to compile \"" + shader_name + "\":" + ut::cret + d3d_err_msg;
			blob_error->Release();
			return ut::MakeError(ut::Error(ut::error::fail, ut::Move(err_msg)));
		}
	}

	// free error buffer
	if (blob_error != nullptr)
	{
		blob_error->Release();
	}

	// create shader info
	Shader::Info info;
	info.stage = stage;
	info.name = ut::Move(shader_name);
	info.entry_point = ut::Move(entry_point);
	info.macros = ut::Move(macros);

	// copy shader bytecode
	info.bytecode.Resize(blob_out->GetBufferSize());
	ut::memory::Copy(info.bytecode.GetAddress(), blob_out->GetBufferPointer(), blob_out->GetBufferSize());

	// find all parameters
	ID3D11ShaderReflection* Reflector = nullptr;
	result = D3DReflect(blob_out->GetBufferPointer(),
	                    blob_out->GetBufferSize(),
	                    IID_ID3D11ShaderReflection,
	                    reinterpret_cast<void**>(&Reflector));
	if (FAILED(result))
	{
		return ut::MakeError(ut::Error(ut::error::fail, ut::Print(result) + " failed to reflect shader \"" + shader_name + "\""));
	}

	// shader description
	D3D11_SHADER_DESC shader_desc;
	result = Reflector->GetDesc(&shader_desc);
	if (FAILED(result))
	{
		return ut::MakeError(ut::Error(ut::error::fail, ut::Print(result) + " Reflector->GetDesc \"" + shader_name + "\""));
	}

	// iterate all resources
	for (UINT i = 0; i < shader_desc.BoundResources; i++)
	{
		D3D11_SHADER_INPUT_BIND_DESC bind_desc;
		Reflector->GetResourceBindingDesc(i, &bind_desc);

		Shader::Parameter::Type parameter_type = Shader::Parameter::unknown;
		ut::String parameter_name = bind_desc.Name;
		ut::uint32 bind_id = bind_desc.BindPoint;

		if (bind_desc.Type == D3D10_SIT_CBUFFER || bind_desc.Type == D3D10_SIT_TBUFFER)
		{
			ID3D11ShaderReflectionConstantBuffer* ConstantBuffer = Reflector->GetConstantBufferByName(bind_desc.Name);
			D3D11_SHADER_BUFFER_DESC cb_desc;
			ConstantBuffer->GetDesc(&cb_desc);
			parameter_type = Shader::Parameter::uniform_buffer;
			parameter_name = cb_desc.Name;
		}
		else if (bind_desc.Type == D3D10_SIT_TEXTURE)
		{
			char desc_name[128];
			strcpy_s(desc_name, 128, bind_desc.Name);
			parameter_type = Shader::Parameter::image;

			// bracket character means that there is a texture array, in that case
			// we should find first element and include only this first element
			char* bracket_char = ut::StrChr<char>(desc_name, '[');
			if (bracket_char)
			{
				if (*(bracket_char + 1) == '0' && *(bracket_char + 2) == ']')
				{
					// end string here
					*bracket_char = '\0';
					parameter_name = desc_name;
				}
			}
		}
		else if (bind_desc.Type == D3D10_SIT_SAMPLER)
		{
			parameter_type = Shader::Parameter::sampler;
		}
		else if (bind_desc.Type == D3D11_SIT_STRUCTURED)
		{
			parameter_type = Shader::Parameter::storage_buffer;
		}
		else if (bind_desc.Type == D3D11_SIT_UAV_RWSTRUCTURED)
		{
			parameter_type = Shader::Parameter::storage_buffer;
		}
		else
		{
			continue;
		}

		// add parameter to the array
		if (!info.parameters.Add(Shader::Parameter(parameter_type,
		                                           parameter_name,
		                                           bind_id)))
		{
			return ut::MakeError(ut::error::out_of_memory);
		}
	}

	// success
	return info;
}

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif // VE_DX11
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
