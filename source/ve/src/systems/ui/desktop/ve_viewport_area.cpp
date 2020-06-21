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
const ut::uint32 ViewportTab::skElementHeight = 32;

// Element margin in pixels.
const ut::uint32 ViewportTab::skElementMargin = 2;

// Height of the the tab in pixels.
const ut::uint32 ViewportTab::skHeight = ViewportTab::skElementHeight + ViewportTab::skElementMargin * 2;

// Offset to the tab in pixels.
const ut::uint32 ViewportArea::skTabMargin = 2;

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
	viewport->size_range(skResizeBorder, skResizeBorder);
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

	const float margin = 0.04f;

	const size_t element_count = arrangement.GetNum();

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
ViewportTab::ViewportTab(const Settings& settings,
                         LayoutArray in_layouts,
                         ut::uint32 x,
                         ut::uint32 y,
                         ut::uint32 w,
                         ut::uint32 h) : Fl_Group(x, y, w, h)
                                       , background(ut::MakeUnique<Fl_Box>(x, y, w, h))
                                       , layouts(ut::Move(in_layouts))
{
	background->box(FL_FLAT_BOX);
	background->color(fl_rgb_color(settings.tab_color.R(), settings.tab_color.G(), settings.tab_color.B()));

	controls_group = ut::MakeUnique<Fl_Group>(x, y, skElementHeight * 3, h);

	layout_choice = CreateLayoutChoice(layouts, x, y);

	controls_group->end();
	controls_group->resizable(nullptr);

	this->resizable(background.Get());
	this->end();
}

// Creates choice widget.
ut::UniquePtr<Fl_Choice> ViewportTab::CreateLayoutChoice(LayoutArray& layouts, int x, int y)
{
	// create widget
	ut::UniquePtr<Fl_Choice> choice = ut::MakeUnique<Fl_Choice>(x + skElementMargin,
	                                                            y + skElementMargin,
	                                                            skElementHeight * 2,
	                                                            skElementHeight);

	// add layout choices
	for (size_t i = 0; i < layouts.GetNum(); i++)
	{
		ut::String name = ut::Print(i);

		ViewportLayout& layout = layouts[i].GetRef();

		int menu_id = choice->add("layout", 0, nullptr, (void*)"One");
		Fl_Menu_Item *item = (Fl_Menu_Item*)&(choice->menu()[menu_id]);
		item->image(layout.icon.Release());   // note: this clobbers the item's label()
		item->callback(ChangeLayoutCallback, &layout.apply_task);

	}

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
	tab = ut::MakeUnique<ViewportTab>(settings,
	                                  ut::Move(layouts),
	                                  x + skTabMargin,
	                                  y + skTabMargin,
	                                  w - skTabMargin * 2,
	                                  ViewportTab::skHeight);

	// tile
	const ut::uint32 vertical_offset = ViewportTab::skHeight + skTabMargin * 2;
	tile = ut::MakeUnique<Fl_Tile>(x,
	                               vertical_offset,
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
	for (size_t i = 0; i < tab->layouts.GetNum(); i++)
	{
		if (tab->layouts[i]->arrangement.GetNum() == skMaxViewports)
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
		tab->layout_choice->value(static_cast<int>(layout_id));
	}

	// calculate viewport position and size
	ViewportLayout& layout = tab->layouts[layout_id.Get()].GetRef();
	const size_t active_vp_count = layout.arrangement.GetNum();
	ut::Array< ut::Rect<int> > rects = CalculateViewportSize(layout);

	for (ut::uint32 i = 0; i < skMaxViewports; i++)
	{
		// generate unique id for the viewport
		Viewport::Id viewport_id = id_generator.Generate();

		// create dummy viewport if it is inactive
		if (i >= active_vp_count)
		{
			viewport_boxes[i] = ut::MakeUnique<ViewportBox>(settings,
		                                                    viewport_id,
		                                                    32, 32, 32, 32);
			viewport_boxes[i]->hide();
			viewport_boxes[i]->GetViewport().Deactivate();
			continue;
		}

		// create viewport
		const ut::Rect<int>& rect = rects[i];
		viewport_boxes[i] = ut::MakeUnique<ViewportBox>(settings,
		                                                viewport_id,
		                                                rect.offset.X(),
		                                                rect.offset.Y(),
		                                                rect.extent.X(),
		                                                rect.extent.Y());
	}

	tile->end();
	this->resizable(tile.Get());

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

// Changes viewport layout.
//    @param layout_id - id of the layout to be set.
void ViewportArea::ChangeLayout(size_t layout_id)
{
	UT_ASSERT(tab->layouts.GetNum() != 0);

	if (layout_id >= tab->layouts.GetNum())
	{
		ut::log.Lock() << "Warning! Invalid layout id: " << layout_id << ut::cret;
		layout_id = tab->layouts.GetNum() - 1;
	}

	ViewportLayout& layout = tab->layouts[layout_id].GetRef();
	const size_t active_vp_count = layout.arrangement.GetNum();
	ut::Array< ut::Rect<int> > rects = CalculateViewportSize(layout);
	
	// iterate all viewports
	for (ut::uint32 i = 0; i < skMaxViewports; i++)
	{
		// hide inactive viewports
		if (i >= active_vp_count)
		{
			viewport_boxes[i]->hide();
			viewport_boxes[i]->GetViewport().Deactivate();
			continue;
		}

		// resize and activate viewport
		viewport_boxes[i]->resize(rects[i].offset.X(),
		                          rects[i].offset.Y(),
		                          rects[i].extent.X(),
		                          rects[i].extent.Y());
		viewport_boxes[i]->show();
		viewport_boxes[i]->GetViewport().Activate();

		// remove viewport from the tile widget
		tile->remove(viewport_boxes[i].Get());
	}

	// insert all viewports back to the tile widget
	// to update tile pattern
	for (ut::uint32 i = 0; i < active_vp_count; i++)
	{
		tile->insert(viewport_boxes[i].GetRef(), nullptr);
	}

	// select icon
	tab->layout_choice->value(static_cast<int>(layout_id));
}

// Returns an id of the current layout.
ut::uint32 ViewportArea::GetCurrentLayoutId() const
{
	return static_cast<ut::uint32>(tab->layout_choice->value());
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
	for (size_t i = 0; i < layout_arrangements.GetNum(); i++)
	{
		auto layout_func = ut::MemberFunction<ViewportArea, void(size_t)>(this, &ViewportArea::ChangeLayout);

		ut::UniquePtr<ViewportLayout> layout = ut::MakeUnique<ViewportLayout>(ut::Move(layout_arrangements[i]),
		                                                                      ViewportTab::skElementHeight,
		                                                                      ut::Task<void(size_t)>(layout_func, i));

		layouts.Add(ut::Move(layout));
	}

	return layouts;
}

// Transforms size of the viewport from relative to absolute.
ut::Array< ut::Rect<int> > ViewportArea::CalculateViewportSize(const ViewportLayout& layout)
{
	const size_t active_vp_count = layout.arrangement.GetNum();
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
	const size_t count = rects.GetNum();
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

//----------------------------------------------------------------------------//
END_NAMESPACE(ui)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif // VE_DESKTOP
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
