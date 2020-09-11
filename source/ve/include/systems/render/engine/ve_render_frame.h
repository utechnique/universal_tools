//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "systems/render/ve_render_api.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// ve::render::Frame encapsulates render resources for a frame.
class Frame
{
public:
	// Constructor.
	Frame(CmdBuffer in_cmd_buffer, Buffer in_display_ub);

	// Move constructor.
	Frame(Frame&&) noexcept;

	// Move operator.
	Frame& operator =(Frame&&) noexcept;

	// Copying is prohibited.
	Frame(const Frame&) = delete;
	Frame& operator =(const Frame&) = delete;

	CmdBuffer cmd_buffer;
	Buffer display_ub;

	struct QuadDescriptorSet : public DescriptorSet
	{
		QuadDescriptorSet() : DescriptorSet(ub, tex1d, tex2d, tex_cube, tex3d,
		                                    tex_cube_face, sampler) {}
		Descriptor ub = "g_cb_display";
		Descriptor tex1d = "g_tex1d";
		Descriptor tex2d = "g_tex2d";
		Descriptor tex_cube = "g_tex_cube";
		Descriptor tex3d = "g_tex3d";
		Descriptor tex_cube_face = "g_tex_cube_face";
		Descriptor sampler = "g_sampler";
	} quad_desc_set;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
