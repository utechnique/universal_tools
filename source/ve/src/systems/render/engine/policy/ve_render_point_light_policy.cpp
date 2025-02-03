//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "systems/render/engine/policy/ve_render_point_light_policy.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// Constructor.
Policy<PointLight>::Policy(Toolset &toolset,
                           UnitSelector& unit_selector,
                           Policies& engine_policies) : tools(toolset)
{}

// Initializes a provided light unit.
void Policy<PointLight>::Initialize(PointLight& light)
{
	ut::Array<PointLight::FrameData> frames;
	for (ut::uint32 i = 0; i < tools.config.frames_in_flight; i++)
	{
		Buffer::Info buffer_info;
		buffer_info.type = Buffer::Type::uniform;
		buffer_info.usage = render::memory::Usage::gpu_read_cpu_write;
		buffer_info.size = sizeof(Light::Uniforms);
		ut::Result<Buffer, ut::Error> uniform_buffer = tools.device.CreateBuffer(ut::Move(buffer_info));
		if (!uniform_buffer)
		{
			throw ut::Error(uniform_buffer.MoveAlt());
		}

		PointLight::FrameData frame_data{ uniform_buffer.Move() };
		if (!frames.Add(ut::Move(frame_data)))
		{
			throw ut::Error(ut::error::out_of_memory);
		}
	}

	PointLight::GpuData gpu_data;
	gpu_data.frames = ut::Move(frames);
	light.data = tools.rc_mgr.AddResource(ut::Move(gpu_data));
}

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//