//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "systems/ui/desktop/ve_desktop_viewport.h"
#include "systems/ui/desktop/ve_desktop_ui_cfg.h"
#include "ve_id_generator.h"
//----------------------------------------------------------------------------//
#if VE_DESKTOP
//----------------------------------------------------------------------------//
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Tile.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Choice.H>
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(ui)
//----------------------------------------------------------------------------//
// ve::ui::ViewportContainer contains viewport, additional tools to interact
// with it and a frame.
class ViewportBox : public Fl_Group
{
public:
	// Constructor.
	ViewportBox(const Settings& settings,
	            Viewport::Id id,
	            ut::uint32 x,
	            ut::uint32 y,
	            ut::uint32 w,
	            ut::uint32 h);

	// Returns a reference to the viewport widget.
	DesktopViewport& GetViewport();

	// boundary for viewport resizing
	static const ut::uint32 skResizeBorder;

private:
	ut::UniquePtr<DesktopViewport> viewport;
};

//----------------------------------------------------------------------------//
// ve::ui::ViewportLayout encapsulates disposition of viewports.
struct ViewportLayout
{
	// Array of 32-bit pixels.
	typedef ut::Array< ut::Color<4, ut::byte> > IconBuffer;

	// Disposition of the viewports.
	typedef ut::Array< ut::Rect<float> > Arrangement;

	// Constructor.
	ViewportLayout(Arrangement in_arrangement,
	               ut::uint32 icon_size,
	               ut::Task<void(size_t)> task);

	// Copying/moving is prohibited.
	ViewportLayout(const ViewportLayout&) = delete;
	ViewportLayout(ViewportLayout&&) = delete;
	ViewportLayout& operator = (const ViewportLayout&) = delete;
	ViewportLayout& operator = (ViewportLayout&&) = delete;

	// Generates icon buffer.
	static IconBuffer GenerateIconData(const Arrangement& arrangement, ut::uint32 size);

	// Generates icon.
	static ut::UniquePtr<Fl_RGB_Image> GenerateIcon(const IconBuffer& buffer, ut::uint32 size);
	
	// Arrangement of the viewports.
	const ut::Array< ut::Rect<float> > arrangement;

	// Icon pixel buffer.
	const IconBuffer icon_buffer;

	// Arrangement icon.
	ut::UniquePtr<Fl_RGB_Image> icon;

	// Task that applies current layout.
	ut::Task<void(size_t)> apply_task;
};

//----------------------------------------------------------------------------//
// ve::ui::ViewportTab contains controls to operate with viewports.
class ViewportTab : public Fl_Group
{
public:
	// Array of layouts.
	typedef ut::Array< ut::UniquePtr<ViewportLayout> > LayoutArray;

	// Constructor.
	ViewportTab(const Settings& settings,
	            LayoutArray in_layouts,
	            ut::uint32 x,
	            ut::uint32 y,
	            ut::uint32 w,
	            ut::uint32 h);

	// Copying/moving is prohibited.
	ViewportTab(const ViewportTab&) = delete;
	ViewportTab(ViewportTab&&) = delete;
	ViewportTab& operator = (const ViewportTab&) = delete;
	ViewportTab& operator = (ViewportTab&&) = delete;

	// Creates choice widget.
	static ut::UniquePtr<Fl_Choice> CreateLayoutChoice(LayoutArray& layouts, int x, int y);

	// Callback that is called when a layout is changed.
	static void ChangeLayoutCallback(Fl_Widget* widget, void* data);

	// Height of the elements in pixels
	static const ut::uint32 skElementHeight;

	// Element margin in pixels.
	static const ut::uint32 skElementMargin;

	// Height of the this tab in pixels
	static const ut::uint32 skHeight;

	// Background box.
	ut::UniquePtr<Fl_Box> background;

	// Array of possible layouts.
	LayoutArray layouts;

	ut::UniquePtr<Fl_Group> controls_group;

	// Combobox widget to choose a layout.
	ut::UniquePtr<Fl_Choice> layout_choice;
};

//----------------------------------------------------------------------------//
// ve::ui::ViewportArea contains viewports arranged in a special layout.
class ViewportArea : public Fl_Group
{
	static constexpr ut::uint32 skMaxViewports = 4;
public:
	// Constructor.
	ViewportArea(const Settings& settings,
	             ut::uint32 x,
	             ut::uint32 y,
	             ut::uint32 w,
	             ut::uint32 h);

	// Copying is prohibited.
	ViewportArea(const ViewportArea&) = delete;
	ViewportArea& operator = (ViewportArea&&) = delete;

	// Generates and returns an array of references to the viewports.
	ut::Array< ut::Ref<Viewport> > GetViewports();

	// Changes viewport layout.
	//    @param layout_id - id of the layout to be set.
	void ChangeLayout(size_t layout_id);

	// Returns an id of the current layout.
	ut::uint32 GetCurrentLayoutId() const;

private:
	// Generates array of different layouts.
	ut::Array< ut::UniquePtr<ViewportLayout> > GenerateDefaultLayouts();

	// Transforms size of the viewport from relative to absolute.
	ut::Array< ut::Rect<int> > CalculateViewportSize(const ViewportLayout& layout);

	// Modifies provided rects so that they had zero margin.
	void AdjustViewportEdges(ut::Array< ut::Rect<int> >& layout_rects);

	// Tab widget.
	ut::UniquePtr<ViewportTab> tab;

	// Tile widget to conveniently resize viewports.
	ut::UniquePtr<Fl_Tile> tile;

	// Box to set resizing range of viewports.
	ut::UniquePtr<Fl_Box> border_box;

	// Viewports are placed in special 'box' widgets.
	ut::UniquePtr<ViewportBox> viewport_boxes[skMaxViewports];

	// Id-generator is used to generate unique identifiers for viewports.
	IdGenerator<Viewport::Id> id_generator;

	// Offset to the tab in pixels.
	static const ut::uint32 skTabMargin;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(ui)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif // VE_DESKTOP
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
