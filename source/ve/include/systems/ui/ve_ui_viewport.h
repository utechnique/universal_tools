//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "systems/ui/ve_ui_platform.h"

//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(ui)
//----------------------------------------------------------------------------//
// ve::ui::Viewport is a special UI window hosting drawable surface.
class Viewport
{
public:
	// Every viewport must have unique id.
	typedef ut::uint32 Id;

	// Projection types.
	enum Projection
	{
		perspective,
		orthographic_negative_x,
		orthographic_positive_x,
		orthographic_negative_y,
		orthographic_positive_y,
		orthographic_negative_z,
		orthographic_positive_z
	};

	// Resolution variants.
	enum Resolution
	{
		resolution_auto, // the same as viewport
		resolution_4k,
		resolution_full_hd,
		resolution_hd
	};

	// Rendering mode.
	enum RenderMode
	{
		render_mode_complete,
		render_mode_diffuse,
		render_mode_normal
	};

	// Mode describes how this viewport interacts with user and how
	// it renders stuff.
	struct Mode
	{
		// Indicates if this viewport is currently active (visible).
		bool is_active = true;

		// Current projection type of this viewport.
		Projection projection = perspective;

		// Viewport resolution.
		Resolution resolution = resolution_auto;

		// Indicates if this viewport accepts input form user.
		bool has_input_focus = false;

		// Width and height of the viewport in pixels.
		ut::uint32 width = 0;
		ut::uint32 height = 0;

		// Rendering mode affects what parts of the rendering
		// pipeline will be displayed to user.
		ut::uint32 render_mode = render_mode_complete;
	};

	// Constructor.
	//    @param viewport_id - id associated with this viewport.
	//    @param viewport_name - name of the viewport.
	//    @param x - the initial horizontal position of the window in pixels.
	//    @param y - the initial vertical position of the window in pixels.
	//    @param w - the initial width of the window in pixels.
	//    @param h - the initial height of the window in pixels.
	Viewport(Id viewport_id, ut::String viewport_name);

	// ve::ui::Viewport is a polymorphic class, so it must have virtual destructor.
	virtual ~Viewport() = default;
	
	// Resizes UI widget if resizing is pending.
	virtual void ResizeCanvas();

	// Connects provided function with signal that is triggered on resize.
	void ConnectResize(ut::Function<void(Id id, ut::uint32 w, ut::uint32 h)> slot);

	// Connects provided function with signal that is triggered in destructor.
	void ConnectClose(ut::Function<void(Id id)> slot);

	// Resets all signals.
	void ResetSignals();

	// Returns current mode. Thread-safe.
	Mode GetMode();

	// Sets new mode. Thread-safe.
	void SetMode(const Mode& new_mode);

	// Returns relative mouse position inside this viewport
	// or nothing if it's outside. Position (0,0) is located
	// in the center of the viewport, X-axis is right, Y-axis
	// is up. Distance to the viewport border is 1.
	virtual ut::Optional< ut::Vector<2> > GetMousePosition();

	// Returns relative mouse position offset from the call of this
	// function or nothing if it's outside.
	//    @param reset - boolean indicating if current offset must be reset.
	virtual ut::Optional< ut::Vector<2> > GetMouseOffset(bool reset);

	// Returns unique identifier of the viewport.
	Id GetId() const;

	// Returns name of the viewport.
	const ut::String& GetName() const;

protected:
	// Current mode.
	ut::Synchronized<Mode> mode;

	// Name of the viewport.
	const ut::String name;

	// Unique id associated with this viewport.
	const Id id;

	// Signal that is triggered on resize.
	ut::Signal<void(Id id, ut::uint32 w, ut::uint32 h)> resize_signal;

	// Signal that is triggered in destructor.
	ut::Signal<void(Id id)> close_signal;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(ui)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//