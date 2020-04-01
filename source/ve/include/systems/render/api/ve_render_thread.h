//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "systems/render/api/ve_render_device.h"
#include "ve_dedicated_thread.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
//----------------------------------------------------------------------------//
// Specialized version of the ve::DedicatedThread template for OpenGL device.
// Xlib is not thread-safe and this means render thread must be isolated from
// X11 calls. UI must idle when a task is being processed in the render thread.
#if UT_LINUX && VE_OPENGL
template<>
class DedicatedThread<render::Device> : public BaseDedicatedThread<render::Device>
{
	typedef BaseDedicatedThread<render::Device> Base;
public:
	// Constructor.
	template<typename... Args>
	DedicatedThread(Args... args) : Base(ut::Forward<Args>(args)...)
	{}

	// Make a wrapper around Enqueue() method of the base class
	void Enqueue(Base::DedicatedTask task)
	{
		Base::DedicatedTask wrap([&](render::Device& d) { Fl::lock(); task(d); Fl::unlock(); });
		Base::Enqueue(ut::Move(wrap));
	}
};
#endif // VE_OPENGL
//----------------------------------------------------------------------------//
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
