//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "systems/ui/ve_ui.h"
#include "systems/ui/desktop/ve_desktop_viewport.h"
#include "systems/render/api/ve_render_device.h"
#include "ve_id_generator.h"
//----------------------------------------------------------------------------//
#if VE_DESKTOP
//----------------------------------------------------------------------------//
#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(ui)
//----------------------------------------------------------------------------//
// ve::ui::DesktopCfg is a class to save/load options and
// preferences for ui widgets.
class DesktopCfg : public ut::meta::Reflective
{
public:
	// Constructor, default values are set here.
	DesktopCfg();

	// Registers data into reflection tree.
	//    @param snapshot - reference to the reflection tree
	void Reflect(ut::meta::Snapshot& snapshot);

	// Saves to file.
	ut::Optional<ut::Error> Save();

	// Loads from file.
	ut::Optional<ut::Error> Load();

	// Generates full local path to the configuration file.
	static ut::String GenerateFullPath();

	// data
	ut::uint32 position_x; // left coordinate of the window
	ut::uint32 position_y; // top coordinate of the window
	ut::uint32 width; // width of the window
	ut::uint32 height; // height of the window

	// default file path to the configuration file
	static const char* skFileName;
};

//----------------------------------------------------------------------------//
// ve::ui::DesktopFrontend class implements user interface for desktop systems.
class DesktopFrontend : public Frontend
{
public:
	// Constructor.
	DesktopFrontend(ut::SharedPtr<render::Device::Thread> in_render_thread);

	// Destructor
	~DesktopFrontend();

	// Initialization.
	ut::Optional<ut::Error> Initialize();

	// Launches user interface.
	void Run();

	// Saves current ui configuration to the file.
	void SaveCfg();

private:
	// Creates ve::render::Display for the viewport with specified id
	// in render thread.
	//    @param device - reference to render device.
	//    @param id - identifier of the viewport to register.
	void RegisterRenderDisplay(render::Device& device, Viewport::Id id);

	// Closes ve::render::Display object associated with specified
	// viewport in render thread.
	//    @param device - reference to render device.
	//    @param id - identifier of the viewport to close.
	void CloseRenderDisplay(render::Device& device, Viewport::Id id);

	// Sends a request to the render thread to resize desired display.
	//    @param id - id of the viewport to resize.
	//    @param width - new width in pixels.
	//    @param height - new height in pixels.
	void ResizeRenderDisplay(Viewport::Id id, ut::uint32 width, ut::uint32 height);

	// fltk is single threaded, thus all widgets must be created and
	// processed in a separate thread
	ut::UniquePtr<ut::Thread> fltk_thread;

	// main window
	ut::UniquePtr<Fl_Window> window;

	// viewports for 3d visualization
	ut::Map< Viewport::Id, ut::UniquePtr<Viewport> > viewports;

	// Id-generator is used to generate unique identifiers for viewports.
	IdGenerator<Viewport::Id> viewport_id_generator;

	// render thread must have access to ui viewports
	ut::SharedPtr<render::Device::Thread> render_thread;

	// Minimum width of the window
	static const ut::uint32 skMinWidth;

	// Minimum height of the window
	static const ut::uint32 skMinHeight;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(ui)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif // VE_DESKTOP
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//