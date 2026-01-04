//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "ve_render_vertex_factory.h"
#include "ve_shader_loader.h"
#include "templates/ut_grid.h"

//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// ve::render::QuadRenderer class helps to draw simple 2D textured quads.
class QuadRenderer
{
public:
	// Vertex format: 2d position and 2d texcoord
	typedef Vertex<float, 2, float, 2> Vertex;

	// Color space convertion schemes that can be
	// applied while sampling a texture.
	enum class ColorSpaceCvt
	{
		none,
		srgb2rgb,
		rgb2srgb,
		count
	};
	
	// Shader permutation grid.
	static constexpr ut::uint32 color_space_cvt_column = 0;
	typedef ut::Grid<static_cast<size_t>(ColorSpaceCvt::count)> ShaderGrid;

	// Constructor, loads vertex and pixel shaders.
	QuadRenderer(Device& device, ShaderLoader& shader_loader);

	// Creates the IA state for the QuadRenderer::Vertex format.
	static InputAssemblyState CreateInputAssemblyState();

	// Shaders.
	Shader vs;
	ut::Array<Shader> ps;

private:
	// Creates the vertex shader.
	static Shader CreateVertexShader(ShaderLoader& shader_loader);

	// Creates a set of pixel shaders permutated by QuadRenderer::ShaderGrid.
	static ut::Array<Shader> CreatePixelShader(ShaderLoader& shader_loader);
};

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
