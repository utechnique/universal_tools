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
const ut::uint32 ViewportBox::skResizeBorder = 64;

// Height of the elements in pixels.
const ut::uint32 ViewportTab::skElementHeight = 22;

// Element margin in pixels.
const ut::uint32 ViewportTab::skElementMargin = 2;

// Height of the the tab in pixels.
const ut::uint32 ViewportTab::skHeight = ViewportTab::skElementHeight + ViewportTab::skElementMargin * 2;

// Offset to the tab in pixels.
const ut::uint32 ViewportArea::skTabMargin = 2;

// Size of the text font for all tab elements.
const ut::uint32 ViewportTab::skFontSize = 13;

// Projection types.
static const ut::uint32 skProjectionTypeCount = 7;
static const char* skProjectionTypeNames[skProjectionTypeCount] =
{
	"Perspective",
	"Ortho X-",
	"Ortho X+",
	"Ortho Y-",
	"Ortho Y+",
	"Ortho Z-",
	"Ortho Z+"
};

// Resolution variants.
static const ut::uint32 skResolutionTypeCount = 6;
static const char* skResolutionTypeNames[skResolutionTypeCount] =
{
	"Auto",
	"4K",
	"1080p",
	"720p",
	"480p",
	"320p",
};

// Render mode variants.
static const ut::uint32 skRenderModeCount = 3;
static const char* skRenderModeNames[skRenderModeCount] =
{
	"Complete",
	"G-Buffer (Diffuse)",
	"G-Buffer (Normal)",
};

//----------------------------------------------------------------------------//
// Constructor.
ViewportBox::ViewportBox(const Settings& settings,
                         Viewport::Id id,
                         ut::Function<void(Viewport::Id)> set_focus_cb,
                         ut::uint32 x,
                         ut::uint32 y,
                         ut::uint32 w,
                         ut::uint32 h) : Fl_Group(x, y, w, h)
                                       , set_input_focus_cb(ut::Move(set_focus_cb))
                                       , hover_color(fl_rgb_color(settings.theme.viewport_hover_color.R(),
                                                                  settings.theme.viewport_hover_color.G(),
                                                                  settings.theme.viewport_hover_color.B()))
                                       , focus_color(fl_rgb_color(settings.theme.viewport_focus_color.R(),
                                                                  settings.theme.viewport_focus_color.G(),
                                                                  settings.theme.viewport_focus_color.B()))
                                       , bg_color(fl_rgb_color(settings.theme.background_color.R(),
                                                               settings.theme.background_color.G(),
                                                               settings.theme.background_color.B()))
{
	// background frame
	background = ut::MakeUnique<Fl_Box>(x, y, w, h);
	background->box(FL_FLAT_BOX);
	background->color(bg_color);
	background->show();

	// viewport
	const ut::uint32 border = settings.viewport_frame_size;
	ut::String viewport_name = ut::Print(id);
	viewport = ut::MakeUnique<DesktopViewport>(id,
	                                           viewport_name,
	                                           x + border / 2,
	                                           y + border / 2,
	                                           w - border,
	                                           h - border);
	viewport->size_range(skResizeBorder, skResizeBorder);
	viewport->end();
	viewport->show();

	// finish this box
	this->resizable(viewport.Get());
	this->end();
}

// Returns a reference to the viewport widget.
DesktopViewport& ViewportBox::GetViewport()
{
	return viewport.GetRef();
}

// Assigns correct color to the viewport frame.
void ViewportBox::UpdateFrameColor()
{
	const Viewport::Mode mode = viewport->GetMode();
	if (has_input_focus)
	{
		background->color(focus_color);
	}
	else
	{
		background->color(bg_color);
	}
}

// Overriden virtual function of the base class (Fl_Group).
void ViewportBox::resize(int x, int y, int w, int h)
{
	UpdateFrameColor();

	Fl_Group* p = parent();
	if (p != nullptr)
	{
		parent()->redraw();
	}
	else
	{
		redraw();
	}

	Fl::flush();

	Fl_Group::resize(x, y, w, h);
}

// Returns relative (to the viewport) cursor position.
ut::Optional< ut::Vector<2> > ViewportBox::CalculateMousePosition() const
{
	const int x = Fl::event_x() - viewport->x();
	const int y = Fl::event_y() - viewport->y();
	const int w = viewport->w();
	const int h = viewport->h();

	float rx = static_cast<float>(x) / static_cast<float>(w);
	rx = 2.0f * rx - 1.0f;
	float ry = static_cast<float>(y) / static_cast<float>(h);
	ry = 1.0f - 2.0f * ry;

	ut::Optional< ut::Vector<2> > relative_position;
	if (ut::Abs(rx) <= 1.0f && ut::Abs(ry) <= 1.0f)
	{
		relative_position = ut::Vector<2>(rx, ry);
	}

	return relative_position;
}

// Overriden virtual function of the base class (Fl_Group).
// Catches mouse events.
int ViewportBox::handle(int event)
{
	const Viewport::Mode mode = viewport->GetMode();
	int ret = Fl_Group::handle(event);
	switch (event)
	{
	case FL_PUSH:
		set_input_focus_cb(viewport->GetId());
		background->color(focus_color);
		ret = 1;
		break;
	case FL_MOVE:
	case FL_DRAG:
		viewport->SetMousePosition(CalculateMousePosition());
		if (!has_input_focus)
		{
			background->color(hover_color);
		}
		redraw();
		ret = 1;
		break;
	case FL_LEAVE:
		viewport->SetMousePosition(ut::Optional< ut::Vector<2> >());
		if (!has_input_focus)
		{
			background->color(bg_color);
		}
		redraw();
		ret = 1;
		break;
	}
	return(ret);
}

//----------------------------------------------------------------------------//
// Constructor.
ViewportLayout::ViewportLayout(Arrangement in_arrangement,
                               ut::uint32 icon_size,
                               ut::Task<void(size_t)> task) : arrangement(ut::Move(in_arrangement))
                                                            , icon_buffer(GenerateIconData(arrangement, icon_size))
                                                            , icon(GenerateIcon(icon_buffer, icon_size))
                                                            , apply_task(ut::Move(task))
{}

// Generates icon buffer.
ViewportLayout::IconBuffer ViewportLayout::GenerateIconData(const Arrangement& arrangement, ut::uint32 size)
{
	IconBuffer icon_buffer(size * size);

	const float margin = 0.06f;

	const size_t element_count = arrangement.Count();

	for (ut::uint32 i = 0; i < size; i++)
	{
		for (ut::uint32 j = 0; j < size; j++)
		{
			ut::Color<4, ut::byte>& color = icon_buffer[i*size + j];
			color = ut::Color<4, ut::byte>(0, 0, 0, 0);

			float x = static_cast<float>(j) / size;
			float y = static_cast<float>(i) / size;
			
			for (size_t k = 0; k < element_count; k++)
			{
				const ut::Rect<float>& rect = arrangement[k];

				if ((x > rect.offset.X() + margin) && x < (rect.offset.X() + rect.extent.X() - margin) &&
					(y > rect.offset.Y() + margin) && y < (rect.offset.Y() + rect.extent.Y() - margin))
				{
					color = ut::Color<4, ut::byte>(162, 162, 0, 255);
					break;
				}
			}
		}
	}

	return icon_buffer;
}

// Generates icon.
ut::UniquePtr<Fl_RGB_Image> ViewportLayout::GenerateIcon(const IconBuffer& buffer,
                                                         ut::uint32 size)
{
	return ut::MakeUnique<Fl_RGB_Image>(buffer.GetFirst().GetData(), size, size, 4);
}

//----------------------------------------------------------------------------//
// Constructor.
ViewportTab::ViewportTab(ViewportArea& in_viewport_area,
                         const Settings& settings,
                         LayoutArray in_layouts,
                         ut::uint32 x,
                         ut::uint32 y,
                         ut::uint32 w,
                         ut::uint32 h) : Fl_Group(x, y, w, h)
                                       , viewport_area(in_viewport_area)
                                       , background(ut::MakeUnique<Fl_Box>(x, y, w, h))
                                       , layouts(ut::Move(in_layouts))
{
	background->box(FL_FLAT_BOX);
	background->color(fl_rgb_color(settings.theme.primary_tab_color.R(),
	                               settings.theme.primary_tab_color.G(),
	                               settings.theme.primary_tab_color.B()));

	controls_group = ut::MakeUnique<Fl_Group>(x, y, skElementHeight * 20, h);
	
	layout_choice = CreateLayoutChoice(layouts, x, y);
	proj_choice = CreateProjChoice(layout_choice->x() + layout_choice->w(), y);
	resolution_choice = CreateResolutionChoice(proj_choice->x() + proj_choice->w(), y);
	render_mode_choice = CreateRenderModeChoice(resolution_choice->x() + resolution_choice->w(), y);

	controls_group->end();
	controls_group->resizable(nullptr);

	this->resizable(background.Get());
	this->end();
}

// Changes viewport projection type.
void ViewportTab::ChangeViewportProjection(Viewport::Projection projection)
{
	ut::Array< ut::Ref<ViewportBox> > viewport_boxes = viewport_area.GetViewportBoxes();
	const size_t viewport_count = viewport_boxes.Count();
	for (size_t i = 0; i < viewport_count; i++)
	{
		ViewportBox& box = viewport_boxes[i];
		if (!box.has_input_focus)
		{
			continue;
		}

		Viewport& viewport = box.GetViewport();
		Viewport::Mode mode = viewport.GetMode();
		mode.projection = projection;
		viewport.SetMode(mode);
	}
}

// Changes viewport resolution type.
void ViewportTab::ChangeViewportResolution(Viewport::Resolution resolution)
{
	ut::Array< ut::Ref<ViewportBox> > viewport_boxes = viewport_area.GetViewportBoxes();
	const size_t viewport_count = viewport_boxes.Count();
	for (size_t i = 0; i < viewport_count; i++)
	{
		ViewportBox& box = viewport_boxes[i];
		if (!box.has_input_focus)
		{
			continue;
		}

		Viewport& viewport = box.GetViewport();
		Viewport::Mode mode = viewport.GetMode();
		mode.resolution = resolution;
		viewport.SetMode(mode);
	}
}

// Changes viewport rendering mode.
void ViewportTab::ChangeViewportRenderMode(Viewport::RenderMode render_mode)
{
	ut::Array< ut::Ref<ViewportBox> > viewport_boxes = viewport_area.GetViewportBoxes();
	const size_t viewport_count = viewport_boxes.Count();
	for (size_t i = 0; i < viewport_count; i++)
	{
		ViewportBox& box = viewport_boxes[i];
		if (!box.has_input_focus)
		{
			continue;
		}

		Viewport& viewport = box.GetViewport();
		Viewport::Mode mode = viewport.GetMode();
		mode.render_mode = render_mode;
		viewport.SetMode(mode);
	}
}

// Creates choice widget.
ut::UniquePtr<Fl_Choice> ViewportTab::CreateLayoutChoice(LayoutArray& layouts, int x, int y)
{
	// create widget
	ut::UniquePtr<Fl_Choice> choice = ut::MakeUnique<Fl_Choice>(x + skElementMargin,
	                                                            y + skElementMargin,
	                                                            48,
	                                                            skElementHeight);

	// add layout choices
	for (size_t i = 0; i < layouts.Count(); i++)
	{
		ViewportLayout& layout = layouts[i].GetRef();

		int menu_id = choice->add("layout", 0, nullptr);
		Fl_Menu_Item *item = (Fl_Menu_Item*)&(choice->menu()[menu_id]);
		item->image(layout.icon.Release());   // note: this clobbers the item's label()
		item->callback(ChangeLayoutCallback, &layout.apply_task);
	}

	// choose a layout
	choice->value(0);

	// success
	return choice;
}

// Creates projection choice widget.
ut::UniquePtr<Fl_Choice> ViewportTab::CreateProjChoice(int x, int y)
{
	// create widget
	ut::UniquePtr<Fl_Choice> choice = ut::MakeUnique<Fl_Choice>(x + skElementMargin,
	                                                            y + skElementMargin,
	                                                            96,
	                                                            skElementHeight);

	// add projection types
	for (size_t i = 0; i < skProjectionTypeCount; i++)
	{
		int menu_id = choice->add(skProjectionTypeNames[i], 0, nullptr);
		Fl_Menu_Item *item = (Fl_Menu_Item*)&(choice->menu()[menu_id]);
		item->callback(ChangeProjectionCallback, this);
		item->labelsize(skFontSize);
	}

	// choose a layout
	choice->value(static_cast<int>(Viewport::Projection::perspective));

	// success
	return choice;
}

// Creates resolution choice widget.
ut::UniquePtr<Fl_Choice> ViewportTab::CreateResolutionChoice(int x, int y)
{
	// create widget
	ut::UniquePtr<Fl_Choice> choice = ut::MakeUnique<Fl_Choice>(x + skElementMargin,
	                                                            y + skElementMargin,
	                                                            64,
	                                                            skElementHeight);

	// add resolution types
	for (size_t i = 0; i < skResolutionTypeCount; i++)
	{
		int menu_id = choice->add(skResolutionTypeNames[i], 0, nullptr);
		Fl_Menu_Item *item = (Fl_Menu_Item*)&(choice->menu()[menu_id]);
		item->callback(ChangeResolutionCallback, this);
		item->labelsize(skFontSize);
	}

	// choose a layout
	choice->value(static_cast<int>(Viewport::Resolution::fit));

	// choose a layout
	choice->value(0);

	// success
	return choice;
}

// Creates rendering mode choice widget.
ut::UniquePtr<Fl_Choice> ViewportTab::CreateRenderModeChoice(int x, int y)
{
	// create widget
	ut::UniquePtr<Fl_Choice> choice = ut::MakeUnique<Fl_Choice>(x + skElementMargin,
	                                                            y + skElementMargin,
	                                                            128,
	                                                            skElementHeight);

	// add mode types
	for (size_t i = 0; i < skRenderModeCount; i++)
	{
		int menu_id = choice->add(skRenderModeNames[i], 0, nullptr);
		Fl_Menu_Item *item = (Fl_Menu_Item*)&(choice->menu()[menu_id]);
		item->callback(ChangeRenderModeCallback, this);
		item->labelsize(skFontSize);
	}

	// choose a layout
	choice->value(static_cast<int>(Viewport::Resolution::fit));

	// choose a layout
	choice->value(0);

	// success
	return choice;
}

// Callback that is called when a layout is changed.
void ViewportTab::ChangeLayoutCallback(Fl_Widget* widget, void* data)
{
	UT_ASSERT(data != nullptr);
	ut::Task<void(size_t)>* task = static_cast<ut::Task<void(size_t)>*>(data);
	task->Execute();
}

// Callback that is called when a projection type is changed.
void ViewportTab::ChangeProjectionCallback(Fl_Widget* widget, void* data)
{
	UT_ASSERT(widget != nullptr);
	UT_ASSERT(data != nullptr);

	const Fl_Choice* choice = static_cast<Fl_Choice*>(widget);
	ViewportTab* tab = static_cast<ViewportTab*>(data);
	Viewport::Projection projection = static_cast<Viewport::Projection>(choice->value());
	tab->ChangeViewportProjection(projection);
}

// Callback that is called when a resolution type is changed.
void ViewportTab::ChangeResolutionCallback(Fl_Widget* widget, void* data)
{
	const Fl_Choice* choice = static_cast<Fl_Choice*>(widget);
	ViewportTab* tab = static_cast<ViewportTab*>(data);
	Viewport::Resolution resolution = static_cast<Viewport::Resolution>(choice->value());
	tab->ChangeViewportResolution(resolution);
}

// Callback that is called when a render mode is changed.
void ViewportTab::ChangeRenderModeCallback(Fl_Widget* widget, void* data)
{
	const Fl_Choice* choice = static_cast<Fl_Choice*>(widget);
	ViewportTab* tab = static_cast<ViewportTab*>(data);
	Viewport::RenderMode render_mode = static_cast<Viewport::RenderMode>(choice->value());
	tab->ChangeViewportRenderMode(render_mode);
}

//----------------------------------------------------------------------------//
// Constructor.
ViewportArea::ViewportArea(const Settings& settings,
                           ut::uint32 x,
                           ut::uint32 y,
                           ut::uint32 w,
                           ut::uint32 h) : Fl_Group(x, y, w, h)
{
	// create a set of default layouts
	ut::Array< ut::UniquePtr<ViewportLayout> > layouts = GenerateDefaultLayouts();

	// tab
	tab = ut::MakeUnique<ViewportTab>(*this,
	                                  settings,
	                                  ut::Move(layouts),
	                                  x + skTabMargin,
	                                  y + skTabMargin,
	                                  w - skTabMargin * 2,
	                                  ViewportTab::skHeight);

	// tile
	const ut::uint32 vertical_offset = ViewportTab::skHeight + skTabMargin * 2;
	tile = ut::MakeUnique<Fl_Tile>(x,
	                               y + vertical_offset,
	                               w,
	                               h - vertical_offset);

	// border
	border_box = ut::MakeUnique<Fl_Box>(ViewportBox::skResizeBorder,
	                                    ViewportBox::skResizeBorder,
	                                    w - ViewportBox::skResizeBorder * 2,
	                                    h - ViewportBox::skResizeBorder * 2);
	tile->resizable(border_box.Get());

	// choose layout that has all viewports activated
	ut::Optional<size_t> layout_id;
	for (size_t i = 0; i < tab->layouts.Count(); i++)
	{
		if (tab->layouts[i]->arrangement.Count() == skMaxViewports)
		{
			layout_id = i;
			break;
		}
	}

	// check if a valid layout was found
	if (!layout_id)
	{
		throw ut::Error(ut::error::not_found,
		                "Must be at least one layout with all viewports active.");
	}
	else
	{
		tab->layout_choice->value(static_cast<int>(layout_id.Get()));
	}

	// calculate viewport position and size
	ViewportLayout& layout = tab->layouts[layout_id.Get()].GetRef();
	const size_t active_vp_count = layout.arrangement.Count();
	ut::Array< ut::Rect<int> > rects = CalculateViewportSize(layout);

	// viewports call this callback when receive mouse input
	auto set_focus_cb = ut::MemberFunction<ViewportArea, void(Viewport::Id)>(this, &ViewportArea::SetViewportFocus);

	// create viewports
	for (ut::uint32 i = 0; i < skMaxViewports; i++)
	{
		// generate unique id for the viewport
		Viewport::Id viewport_id = id_generator.Generate();

		// create dummy viewport if it is inactive
		if (i >= active_vp_count)
		{
			viewport_boxes[i] = ut::MakeUnique<ViewportBox>(settings,
		                                                    viewport_id,
			                                                set_focus_cb,
		                                                    32, 32, 32, 32);

			ViewportBox& box = viewport_boxes[i].GetRef();
			Viewport& viewport = box.GetViewport();

			// deactivate viewport
			ui::Viewport::Mode mode = viewport.GetMode();
			mode.is_active = false;
			viewport.SetMode(mode);
			box.hide();

			continue;
		}

		// create viewport
		const ut::Rect<int>& rect = rects[i];
		viewport_boxes[i] = ut::MakeUnique<ViewportBox>(settings,
		                                                viewport_id,
		                                                set_focus_cb,
		                                                rect.offset.X(),
		                                                rect.offset.Y(),
		                                                rect.extent.X(),
		                                                rect.extent.Y());
	}

	tile->end();
	this->resizable(tile.Get());

	// finish the tile widget
	this->end();

	// at least one viewport must have input focus
	if (active_vp_count > 0)
	{
		SetViewportFocus(viewport_boxes[0]->GetViewport().GetId());
	}
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

// Generates and returns an array of references to the viewport boxes.
ut::Array< ut::Ref<ViewportBox> > ViewportArea::GetViewportBoxes()
{
	ut::Array< ut::Ref<ViewportBox> > out;
	for (ut::uint32 i = 0; i < skMaxViewports; i++)
	{
		out.Add(viewport_boxes[i].GetRef());
	}
	return out;
}

// Returns an array of viewport rectangles.
ut::Array< ut::Rect<ut::uint32> > ViewportArea::GetViewportRects() const
{
	ut::Array< ut::Rect<ut::uint32> > out;

	for (size_t i = 0; i < skMaxViewports; i++)
	{
		const ViewportBox& box = viewport_boxes[i].GetRef();
		ut::Rect<ut::uint32> rect;
		rect.offset = ut::Vector<2, ut::uint32>(box.x(), box.y());
		rect.extent = ut::Vector<2, ut::uint32>(box.w(), box.h());
		out.Add(rect);
	}

	return out;
}

// Returns an array of current projections for all viewports.
ut::Array<ut::uint32> ViewportArea::GetViewportProjections()
{
	ut::Array<ut::uint32> out;

	for (size_t i = 0; i < skMaxViewports; i++)
	{
		const Viewport::Mode mode = viewport_boxes[i]->GetViewport().GetMode();
		out.Add(static_cast<ut::uint32>(mode.projection));
	}

	return out;
}

// Returns an array of current render modes for all viewports.
ut::Array<ut::uint32> ViewportArea::GetViewportRenderModes()
{
	ut::Array<ut::uint32> out;

	for (size_t i = 0; i < skMaxViewports; i++)
	{
		const Viewport::Mode mode = viewport_boxes[i]->GetViewport().GetMode();
		out.Add(static_cast<ut::uint32>(mode.render_mode));
	}

	return out;
}

// Updates viewport position and size.
ut::Optional<ut::Error> ViewportArea::ResizeViewports(const ut::Array< ut::Rect<ut::uint32> >& viewport_rects)
{
	if (viewport_rects.Count() != skMaxViewports)
	{
		return ut::Error(ut::error::out_of_bounds);
	}

	const size_t visible_viewports = tab->layouts[GetCurrentLayoutId()]->arrangement.Count();

	for (size_t i = 0; i < skMaxViewports; i++)
	{
		ViewportBox& box = viewport_boxes[i].GetRef();
		Viewport& viewport = box.GetViewport();
		const ut::Rect<ut::uint32>& rect = viewport_rects[i];

		// viewport must be shifted beyond the parent widget
		// to remove dummy resize areas
		const bool shift_out = i < visible_viewports;

		box.resize(rect.offset.X() + (shift_out ? 0 : w()),
		           rect.offset.Y() + (shift_out ? 0 : h()),
		           rect.extent.X(),
		           rect.extent.Y());
	}

	return ut::Optional<ut::Error>();
}

// Updates projection type for all viewports.
void ViewportArea::SetViewportProjections(const ut::Array<ut::uint32>& projections)
{
	const size_t proj_count = ut::Min<size_t>(projections.Count(), skMaxViewports);
	for (size_t i = 0; i < proj_count; i++)
	{
		ViewportBox& viewport_box = viewport_boxes[i].GetRef();
		Viewport& viewport = viewport_box.GetViewport();
		ui::Viewport::Mode mode = viewport.GetMode();
		mode.projection = static_cast<Viewport::Projection>(projections[i]);
		viewport.SetMode(mode);

		if (viewport_box.has_input_focus)
		{
			tab->proj_choice->value(static_cast<int>(mode.projection));
		}
	}
}

// Updates render modes for all viewports.
void ViewportArea::SetViewportRenderModes(const ut::Array<ut::uint32>& render_modes)
{
	const size_t proj_count = ut::Min<size_t>(render_modes.Count(), skMaxViewports);
	for (size_t i = 0; i < proj_count; i++)
	{
		ViewportBox& viewport_box = viewport_boxes[i].GetRef();
		Viewport& viewport = viewport_box.GetViewport();
		ui::Viewport::Mode mode = viewport.GetMode();
		mode.render_mode = static_cast<Viewport::RenderMode>(render_modes[i]);
		viewport.SetMode(mode);

		if (viewport_box.has_input_focus)
		{
			tab->render_mode_choice->value(static_cast<int>(mode.render_mode));
		}
	}
}

// Changes viewport layout.
//    @param layout_id - id of the layout to be set.
void ViewportArea::ChangeLayout(size_t layout_id)
{
	UT_ASSERT(tab->layouts.Count() != 0);

	if (layout_id >= tab->layouts.Count())
	{
		ut::log.Lock() << "Warning! Invalid layout id: " << layout_id << ut::cret;
		layout_id = tab->layouts.Count() - 1;
	}

	ViewportLayout& layout = tab->layouts[layout_id].GetRef();
	const size_t active_vp_count = layout.arrangement.Count();
	ut::Array< ut::Rect<int> > rects = CalculateViewportSize(layout);
	
	// iterate all viewports
	for (ut::uint32 i = 0; i < skMaxViewports; i++)
	{
		ViewportBox& box = viewport_boxes[i].GetRef();
		DesktopViewport& viewport = box.GetViewport();
		Viewport::Mode mode = viewport.GetMode();

		// remove input focus from all viewports
		box.has_input_focus = false;
		mode.is_interactive = false;

		// hide inactive viewports
		if (i >= active_vp_count)
		{
			// hide viewport widget
			box.hide();

			// viewport must be shifted beyond the parent widget
			// to remove dummy resize areas
			box.resize(w(), h(), box.w(), box.h());

			// deactivate viewport
			mode.is_active = false;
			viewport.SetMode(mode);

			continue;
		}

		// resize viewport
		box.resize(rects[i].offset.X(),
		           rects[i].offset.Y(),
		           rects[i].extent.X(),
		           rects[i].extent.Y());
		
		// update width and height
		mode.width = rects[i].extent.X();
		mode.height = rects[i].extent.Y();

		// activate viewport
		mode.is_active = true;
		viewport.SetMode(mode);
		box.show();

		// remove viewport from the tile widget
		tile->remove(&box);
	}

	// insert all viewports back to the tile widget
	// to update tile pattern
	for (ut::uint32 i = 0; i < active_vp_count; i++)
	{
		tile->insert(viewport_boxes[i].GetRef(), nullptr);
	}

	// select icon
	tab->layout_choice->value(static_cast<int>(layout_id));

	// at least one viewport must have input focus
	if (active_vp_count > 0)
	{
		SetViewportFocus(viewport_boxes[0]->GetViewport().GetId());
	}

	// redraw everything
	redraw();
}

// Returns an id of the current layout.
ut::uint32 ViewportArea::GetCurrentLayoutId() const
{
	return static_cast<ut::uint32>(tab->layout_choice->value());
}

int ViewportArea::handle(int e)
{
	const bool got_focus = e == FL_FOCUS;
	const bool lost_focus = e == FL_UNFOCUS;

	if (!got_focus && !lost_focus)
	{
		return Fl_Group::handle(e);
	}

	for (size_t i = 0; i < skMaxViewports; i++)
	{
		ViewportBox& box = viewport_boxes[i].GetRef();
		DesktopViewport& viewport = box.GetViewport();
		Viewport::Mode mode = viewport.GetMode();

		if (got_focus && box.has_input_focus)
		{
			mode.is_interactive = true;
		}
		else
		{
			mode.is_interactive = false;
		}

		viewport.SetMode(mode);
	}

	return Fl_Group::handle(e);
}

// Generates array of different layouts.
ut::Array< ut::UniquePtr<ViewportLayout> > ViewportArea::GenerateDefaultLayouts()
{
	ut::Array<ViewportLayout::Arrangement> layout_arrangements;
	ut::Array< ut::Rect<float> > elements;

	elements.Resize(1);
	elements[0] = ut::Rect<float>(0, 0, 1, 1);
	layout_arrangements.Add(ut::Move(elements));

	elements.Resize(4);
	elements[0] = ut::Rect<float>(0, 0, 0.5f, 0.5f);
	elements[1] = ut::Rect<float>(0.5f, 0, 0.5f, 0.5f);
	elements[2] = ut::Rect<float>(0, 0.5f, 0.5f, 0.5f);
	elements[3] = ut::Rect<float>(0.5f, 0.5f, 0.5f, 0.5f);
	layout_arrangements.Add(ut::Move(elements));

	elements.Resize(2);
	elements[0] = ut::Rect<float>(0.0f,        0.0f, 1.0f / 2.0f, 1);
	elements[1] = ut::Rect<float>(1.0f / 2.0f, 0.0f, 1.0f / 2.0f, 1);
	layout_arrangements.Add(ut::Move(elements));

	elements.Resize(2);
	elements[0] = ut::Rect<float>(0.0f, 0.0f,        1.0f, 1.0f / 2.0f);
	elements[1] = ut::Rect<float>(0.0f, 1.0f / 2.0f, 1.0f, 1.0f / 2.0f);
	layout_arrangements.Add(ut::Move(elements));

	elements.Resize(3);
	elements[0] = ut::Rect<float>(0.0f,        0.0f,        1.0f,        1.0f / 2.0f);
	elements[1] = ut::Rect<float>(0.0f,        1.0f / 2.0f, 1.0f / 2.0f, 1.0f / 2.0f);
	elements[2] = ut::Rect<float>(1.0f / 2.0f, 1.0f / 2.0f, 1.0f / 2.0f, 1.0f / 2.0f);
	layout_arrangements.Add(ut::Move(elements));

	elements.Resize(3);
	elements[0] = ut::Rect<float>(0.0f,        0.0f,        1.0f / 2.0f, 1.0f / 2.0f);
	elements[1] = ut::Rect<float>(1.0f / 2.0f, 0.0f,        1.0f / 2.0f, 1.0f / 2.0f);
	elements[2] = ut::Rect<float>(0.0f,        1.0f / 2.0f, 1.0f,        1.0f / 2.0f);
	layout_arrangements.Add(ut::Move(elements));

	elements.Resize(3);
	elements[0] = ut::Rect<float>(0.0f,        0, 1.0f / 3.0f, 1.0f);
	elements[1] = ut::Rect<float>(1.0f / 3.0f, 0, 1.0f / 3.0f, 1.0f);
	elements[2] = ut::Rect<float>(2.0f / 3.0f, 0, 1.0f / 3.0f, 1.0f);
	layout_arrangements.Add(ut::Move(elements));

	elements.Resize(4);
	elements[0] = ut::Rect<float>(0.0f,        0.0f,        1.0f / 2.0f, 1.0f);
	elements[1] = ut::Rect<float>(1.0f / 2.0f, 0.0f,        1.0f / 2.0f, 1.0f / 3.0f);
	elements[2] = ut::Rect<float>(1.0f / 2.0f, 1.0f / 3.0f, 1.0f / 2.0f, 1.0f / 3.0f);
	elements[3] = ut::Rect<float>(1.0f / 2.0f, 2.0f / 3.0f, 1.0f / 2.0f, 1.0f / 3.0f);
	layout_arrangements.Add(ut::Move(elements));

	elements.Resize(4);
	elements[0] = ut::Rect<float>(0.0f,        0.0f, 1.0f / 3.0f, 1.0f);
	elements[1] = ut::Rect<float>(1.0f / 3.0f, 0.0f, 1.0f / 3.0f, 1.0f);
	elements[2] = ut::Rect<float>(2.0f / 3.0f, 0.0f, 1.0f / 3.0f, 0.5f);
	elements[3] = ut::Rect<float>(2.0f / 3.0f, 0.5f, 1.0f / 3.0f, 0.5f);
	layout_arrangements.Add(ut::Move(elements));

	ut::Array< ut::UniquePtr<ViewportLayout> > layouts;
	for (size_t i = 0; i < layout_arrangements.Count(); i++)
	{
		auto layout_func = ut::MemberFunction<ViewportArea, void(size_t)>(this, &ViewportArea::ChangeLayout);

		ut::UniquePtr<ViewportLayout> layout = ut::MakeUnique<ViewportLayout>(ut::Move(layout_arrangements[i]),
		                                                                      ViewportTab::skElementHeight - ViewportTab::skElementMargin * 2,
		                                                                      ut::Task<void(size_t)>(layout_func, i));

		layouts.Add(ut::Move(layout));
	}

	return layouts;
}

// Transforms size of the viewport from relative to absolute.
ut::Array< ut::Rect<int> > ViewportArea::CalculateViewportSize(const ViewportLayout& layout)
{
	const size_t active_vp_count = layout.arrangement.Count();
	ut::Array< ut::Rect<int> > rects(active_vp_count);

	// calculate new viewport position and size
	for (size_t i = 0; i < active_vp_count; i++)
	{
		rects[i].offset.X() = static_cast<int>(tile->x() + tile->w() * layout.arrangement[i].offset.X());
		rects[i].offset.Y() = static_cast<int>(tile->y() + tile->h() * layout.arrangement[i].offset.Y());
		rects[i].extent.X() = static_cast<int>(tile->w() * layout.arrangement[i].extent.X());
		rects[i].extent.Y() = static_cast<int>(tile->h() * layout.arrangement[i].extent.Y());
	}

	// adgust edges
	AdjustViewportEdges(rects);

	// success
	return rects;
}

// Modifies provided rects so that they had zero margin.
void ViewportArea::AdjustViewportEdges(ut::Array< ut::Rect<int> >& rects)
{
	const size_t count = rects.Count();
	for (size_t i = 0; i < count; i++)
	{
		for (size_t j = 0; j < count; j++)
		{
			if (i == j)
			{
				continue;
			}


			int dx = ut::Abs(rects[j].offset.X() - rects[i].offset.X() - rects[i].extent.X());
			int dy = ut::Abs(rects[j].offset.Y() - rects[i].offset.Y() - rects[i].extent.Y());

			if (dx != 0 && dx < 10)
			{
				rects[j].offset.X() = rects[i].offset.X() + rects[i].extent.X();
			}

			if (dy != 0 && dy < 10)
			{
				rects[j].offset.Y() = rects[i].offset.Y() + rects[i].extent.Y();
			}


			int dw = ut::Abs(tile->w() - rects[j].offset.X() - rects[j].extent.X());
			int dh = ut::Abs(tile->h() - rects[j].offset.Y() - rects[j].extent.Y());

			if (dw != 0 && dw < 10)
			{
				rects[j].extent.X() = tile->w() - rects[j].offset.X();
			}

			if (dh != 0 && dh < 10)
			{
				rects[j].extent.Y() = tile->h() - rects[j].offset.Y();
			}
		}
	}
}

// Assigns input focus to the desired viewport. Other viewports loose focus.
void ViewportArea::SetViewportFocus(Viewport::Id id)
{
	for (size_t i = 0; i < skMaxViewports; i++)
	{
		ViewportBox& box = viewport_boxes[i].GetRef();
		DesktopViewport& viewport = box.GetViewport();
		Viewport::Mode mode = viewport.GetMode();
		
		box.has_input_focus = viewport.GetId() == id;
		box.UpdateFrameColor();

		if (box.has_input_focus)
		{
			tab->proj_choice->value(static_cast<int>(mode.projection));
			tab->resolution_choice->value(static_cast<int>(mode.resolution));
			tab->render_mode_choice->value(static_cast<int>(mode.render_mode));
		}

		mode.is_interactive = box.has_input_focus;
		viewport.SetMode(mode);

		box.redraw();
	}
}

//----------------------------------------------------------------------------//
END_NAMESPACE(ui)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif // VE_DESKTOP
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
