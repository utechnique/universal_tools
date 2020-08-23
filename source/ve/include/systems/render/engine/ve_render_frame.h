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
		QuadDescriptorSet() : DescriptorSet(ub) {}
		Descriptor ub = "g_cb_display";
	} quad_desc_set;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
