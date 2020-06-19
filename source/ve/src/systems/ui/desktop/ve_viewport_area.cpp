//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "systems/ui/desktop/ve_viewport_area.h"
//----------------------------------------------------------------------------//
#if VE_DESKTOP
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(ui)
//----------------------------------------------------------------------------//
// Boundary for viewport resizing.
const ut::uint32 ViewportArea::skResizeBorder = 64;

//----------------------------------------------------------------------------//
// Constructor.
ViewportBox::ViewportBox(const Settings& settings,
                         Viewport::Id id,
                         ut::uint32 x,
                         ut::uint32 y,
                         ut::uint32 w,
                         ut::uint32 h) : Fl_Group(x, y, w, h)
{
	// viewport
	const ut::uint32 border = settings.viewport_frame_size;
	ut::String viewport_name = ut::Print(id);
	viewport = ut::MakeUnique<DesktopViewport>(id,
	                                           viewport_name,
	                                           x + border / 2,
	                                           y + border / 2,
	                                           w - border,
	                                           h - border);
	viewport->end();

	// finish this box
	this->resizable(viewport.Get());
	this->end();
}

// Returns a reference to the viewport widget.
DesktopViewport& ViewportBox::GetViewport()
{
	return viewport.GetRef();
}

//----------------------------------------------------------------------------//
// Constructor.
ViewportArea::ViewportArea(const Settings& settings,
                           ut::uint32 x,
                           ut::uint32 y,
                           ut::uint32 w,
                           ut::uint32 h) : Fl_Tile(x, y, w, h)
{
	// resize border
	border_box = ut::MakeUnique<Fl_Box>(skResizeBorder,
	                                    skResizeBorder,
	                                    w - skResizeBorder * 2,
	                                    h - skResizeBorder * 2);
	this->resizable(border_box.Get());

	const ut::uint32 half_width = w / 2;
	const ut::uint32 half_height = h / 2;

	ut::Rect<ut::uint32> layout[skMaxViewports];
	layout[0].offset = ut::Vector<2, ut::uint32>(0, 0);
	layout[0].extent = ut::Vector<2, ut::uint32>(half_width, half_height);

	layout[1].offset = ut::Vector<2, ut::uint32>(half_width, 0);
	layout[1].extent = ut::Vector<2, ut::uint32>(half_width, half_height);

	layout[2].offset = ut::Vector<2, ut::uint32>(0, half_height);
	layout[2].extent = ut::Vector<2, ut::uint32>(half_width, half_height);

	layout[3].offset = ut::Vector<2, ut::uint32>(half_width, half_height);
	layout[3].extent = ut::Vector<2, ut::uint32>(half_width, half_height);

	for (ut::uint32 i = 0; i < skMaxViewports; i++)
	{
		// create viewport
		Viewport::Id viewport_id = id_generator.Generate();
		viewport_boxes[i] = ut::MakeUnique<ViewportBox>(settings,
		                                                viewport_id,
		                                                layout[i].offset.X(),
		                                                layout[i].offset.Y(),
		                                                layout[i].extent.X(),
		                                                layout[i].extent.Y());
	}

	// finish the tile widget
	this->end();
}

// Generates and returns an array of references to the viewports.
ut::Array< ut::Ref<Viewport> > ViewportArea::GetViewports()
{
	ut::Array< ut::Ref<Viewport> > out;
	for (ut::uint32 i = 0; i < skMaxViewports; i++)
	{
		out.Add(viewport_boxes[i]->GetViewport());
	}
	return out;
}

//----------------------------------------------------------------------------//
END_NAMESPACE(ui)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif // VE_DESKTOP
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
