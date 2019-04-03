//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "commie_common.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(commie)
START_NAMESPACE(resources)
//----------------------------------------------------------------------------//
// commie::resources::Image is a class for storing 2d image resources.
// Holds only raw data, so dimensions and format must be known at this point.
struct Image
{
	// Constructor, notice that there is no default constructor.
	Image(ut::uint32 img_width,
	      ut::uint32 img_height,
	      const ut::byte* img_data);

	// metrics
	const ut::uint32 width;
	const ut::uint32 height;

	// pixels
	const ut::byte* data;
};

//----------------------------------------------------------------------------//
// Images
extern const Image gkOptionsIcon;
extern const Image gkClosedLockIcon;
extern const Image gkOpenedLockIcon;

//----------------------------------------------------------------------------//
END_NAMESPACE(resources)
END_NAMESPACE(commie)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//