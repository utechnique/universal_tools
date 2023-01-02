//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "systems/ui/ve_ui_platform.h"
//----------------------------------------------------------------------------//
#include <FL/Fl_RGB_Image.H>
//----------------------------------------------------------------------------//
#if VE_DESKTOP
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(ui)
//----------------------------------------------------------------------------//
// Class containing icon image data.
class Icon
{
public:
	// Constructor, initializes fltk image structure.
	Icon(ut::uint32 image_width,
	     ut::uint32 image_height,
	     ut::Array< ut::Color<4, ut::byte> > pixels);

	// Moving is allowed.
	Icon(Icon&&);
	Icon& operator = (Icon&&);

	// Copying is prohibited.
	Icon(const Icon&) = delete;
	Icon& operator = (const Icon&) = delete;

	// Metrics.
	ut::uint32 GetWidth() const { return width; }
	ut::uint32 GetHeight() const { return height; }

	// FLTK image resource.
	Fl_RGB_Image* GetImage() { return image.Get(); }

	// Creates expand/collapse icon.
	static Icon CreateCollapse(ut::uint32 width,
	                           ut::uint32 height,
	                           const ut::Color<4, ut::byte>& color,
	                           bool expanded);

	// Creates plus icon.
	static Icon CreatePlus(ut::uint32 width,
	                       ut::uint32 height,
	                       const ut::Color<4, ut::byte>& color,
	                       ut::uint32 thickness,
	                       ut::uint32 margin);

	// Creates cross icon (thickness is one pixel).
	static Icon CreateCross(ut::uint32 width,
	                        ut::uint32 height,
	                        const ut::Color<4, ut::byte>& color,
	                        ut::uint32 margin);

	// Creates an icon with two arrows in different directions (thickness is one pixel).
	static Icon CreateChange(ut::uint32 width,
	                         ut::uint32 height,
	                         const ut::Color<4, ut::byte>& color,
	                         ut::uint32 margin);

	// Creates an icon with a trash bin.
	static Icon CreateTrashBin(ut::uint32 width,
	                           ut::uint32 height,
	                           const ut::Color<4, ut::byte>& color,
	                           ut::uint32 margin,
	                           ut::uint32 thickness);

	// Creates an arrow.
	static Icon CreateArrow(ut::uint32 width,
	                        ut::uint32 height,
	                        const ut::Color<4, ut::byte>& color,
	                        ut::uint32 margin,
	                        bool left,
	                        bool double_arrow);

private:
	ut::uint32 width; // in pixels
	ut::uint32 height; // in pixels
	ut::Array< ut::Color<4, ut::byte> > data;
	ut::UniquePtr<Fl_RGB_Image> image;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(ui)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif // VE_DESKTOP
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//