//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "systems/render/ve_render_api.h"
#include "ve_render_vertex_factory.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// ve::render::Frame encapsulates render resources needed to update all display
// buffers once per logical frame (tick).
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

	// backbuffer clear color
	ut::Color<4> clear_color;

	// command buffer for the frame
	CmdBuffer cmd_buffer;

	// uniform buffer to render final image to the backbuffer
	Buffer display_quad_ub;

	// descriptor set for quad shader
	struct QuadDescriptorSet : public DescriptorSet
	{
		QuadDescriptorSet() : DescriptorSet(ub, tex2d, sampler) {}
		Descriptor ub = "g_ub_display_quad";
		Descriptor tex2d = "g_tex2d";
		Descriptor sampler = "g_sampler";
	} quad_desc_set;

	// g_ub_display_quad uniform buffer reflection
	struct DisplayUB
	{
		alignas(16) ut::Color<4> color;
	};

	// Creates input assembly state for primitives that can be
	// rendered directly to a display.
	static InputAssemblyState CreateInputAssemblyState();

private:
	// vertex type for rendering quads directly to a display
	typedef Vertex<float, 2, float, 2> QuadVertex;
};

//----------------------------------------------------------------------------//
// ve::render::FrameManager encapsulates operations with frames (swapping,
// allocating, etc.).
class FrameManager
{
public:
	// Constructor.
	FrameManager(Device& device_ref);

	// Allocates desired number of frames.
	//    @param frame_count - number of frames.
	//    @param display_quad_shader - shader that renders a quad
	//                                 directly to the backbuffer.
	//    @return - optional error if failed.
	ut::Optional<ut::Error> AllocateFrames(ut::uint32 frame_count,
	                                       BoundShader& display_quad_shader);

	// Returns a reference to the current frame.
	Frame& GetCurrentFrame();

	// Returns an id of the current frame.
	ut::uint32 GetCurrentFrameId() const;

	// Swaps frames so that the next call to GetCurrentFrame()
	// will return next frame instead of current.
	void SwapFrames();

	// Waits until all frames finish rendering.
	void WaitCmdBuffers();

private:
	Device& device;

	// per-frame data (frames in flight)
	ut::Array<Frame> frames;

	// index of the element in @frames array that will be
	// used to render current frame
	ut::uint32 current_frame_id;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
