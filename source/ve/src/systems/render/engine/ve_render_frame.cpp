//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "systems/render/engine/ve_render_frame.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// Constructor.
Frame::Frame(CmdBuffer in_cmd_buffer,
             Buffer in_display_ub) : cmd_buffer(ut::Move(in_cmd_buffer))
                                   , display_quad_ub(ut::Move(in_display_ub))
                                   , clear_color(0, 0, 0, 0)
{}

// Move constructor.
Frame::Frame(Frame&&) noexcept = default;

// Move operator.
Frame& Frame::operator =(Frame&&) noexcept = default;

// Creates input assembly state for primitives that can be
// rendered directly to a display.
InputAssemblyState Frame::CreateInputAssemblyState()
{
	InputAssemblyState input_assembly;
	input_assembly.topology = primitive::triangle_list;
	input_assembly.elements = QuadVertex::CreateLayout();
	input_assembly.vertex_stride = QuadVertex::size;
	return input_assembly;
}

//----------------------------------------------------------------------------//
// Constructor.
FrameManager::FrameManager(Device& device_ref) : device(device_ref)
                                               , current_frame_id(0)
{}

// Allocates desired number of frames.
//    @param frame_count - number of frames.
//    @param display_quad_ps - pixel shader rendering a quad directly
//                             to the backbuffer.
//    @return - optional error if failed.
ut::Optional<ut::Error> FrameManager::AllocateFrames(ut::uint32 frame_count,
                                                     Shader& display_quad_ps)
{
	if (!frames.IsEmpty())
	{
		return ut::Error(ut::error::not_empty, "Frames can be allocated only once.");
	}

	while (frame_count--)
	{
		// initialize command buffer info
		CmdBuffer::Info cmd_buffer_info;
		cmd_buffer_info.usage = CmdBuffer::usage_dynamic;

		// create command buffer
		ut::Result<CmdBuffer, ut::Error> cmd_buffer = device.CreateCmdBuffer(cmd_buffer_info);
		if (!cmd_buffer)
		{
			return cmd_buffer.MoveAlt();
		}

		// create display uniform buffer
		Buffer::Info buffer_info;
		buffer_info.type = Buffer::uniform;
		buffer_info.usage = render::memory::gpu_read_cpu_write;
		buffer_info.size = sizeof(Frame::DisplayUB);
		ut::Result<Buffer, ut::Error> display_ub = device.CreateBuffer(ut::Move(buffer_info));
		if (!display_ub)
		{
			return display_ub.MoveAlt();
		}

		// create frame
		if (!frames.Add(Frame(cmd_buffer.Move(), display_ub.Move())))
		{
			return ut::Error(ut::error::out_of_memory);
		}

		// connect descriptors
		frames.GetLast().quad_desc_set.Connect(display_quad_ps);
	}

	return ut::Optional<ut::Error>();
}

// Returns a reference to the current frame.
Frame& FrameManager::GetCurrentFrame()
{
	return frames[current_frame_id];
}

// Returns an id of the current frame.
ut::uint32 FrameManager::GetCurrentFrameId() const
{
	return current_frame_id;
}

// Swaps frames so that the next call to GetCurrentFrame()
// will return next frame instead of current.
void FrameManager::SwapFrames()
{
	if (++current_frame_id >= frames.Count())
	{
		current_frame_id = 0;
	}
}

// Waits until all frames finish rendering.
void FrameManager::WaitCmdBuffers()
{
	const size_t frame_count = frames.Count();
	for (size_t i = 0; i < frame_count; i++)
	{
		device.WaitCmdBuffer(frames[i].cmd_buffer);
	}
}

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//