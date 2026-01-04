//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "systems/render/engine/ve_render_quad.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// Constructor, loads vertex and pixel shaders.
QuadRenderer::QuadRenderer(Device& device,
                           ShaderLoader& shader_loader) : vs(CreateVertexShader(shader_loader))
                                                        , ps(CreatePixelShader(shader_loader))
{}

// Creates the IA state for the QuadRenderer::Vertex format.
InputAssemblyState QuadRenderer::CreateInputAssemblyState()
{
	InputAssemblyState input_assembly;
	input_assembly.topology = primitive::Topology::triangle_list;
	input_assembly.elements = Vertex::CreateLayout();
	input_assembly.vertex_stride = Vertex::size;
	return input_assembly;
}

// Creates the vertex shader.
Shader QuadRenderer::CreateVertexShader(ShaderLoader& shader_loader)
{
	return shader_loader.Load(Shader::Stage::vertex,
	                          "quad_vs",
	                          "VS",
	                          "quad_vs.hlsl").MoveOrThrow();
}

// Creates a set of pixel shaders permutated by QuadRenderer::ShaderGrid.
ut::Array<Shader> QuadRenderer::CreatePixelShader(ShaderLoader& shader_loader)
{
	ut::Array<Shader> shaders;

	constexpr size_t permutation_count = ShaderGrid::size;
	for (size_t i = 0; i < permutation_count; i++)
	{
		Shader::Macros macros;
		ut::String cvt_suffix;

		const ColorSpaceCvt color_space_cvt = static_cast<ColorSpaceCvt>(ShaderGrid::GetCoordinate<color_space_cvt_column>(i));
		if (color_space_cvt != ColorSpaceCvt::none)
		{
			cvt_suffix = color_space_cvt == ColorSpaceCvt::rgb2srgb ? "linear2srgb" : "srgb2linear";

			Shader::MacroDefinition conversion;
			conversion.name = color_space_cvt == ColorSpaceCvt::rgb2srgb ? "RGB_TO_SRGB" : "SRGB_TO_RGB";
			conversion.value = "1";
			macros.Add(ut::Move(conversion));
		}

		shaders.Add(shader_loader.Load(Shader::Stage::pixel,
		                               ut::String("img_quad_ps_") + cvt_suffix,
		                               "PS",
		                               "img_quad_ps.hlsl",
		                               macros).MoveOrThrow());
	}

	return shaders;
}

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//