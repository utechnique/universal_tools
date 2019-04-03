//----------------------------------------------------------------------------//
//---------------------------------|  U  T  |---------------------------------//
//----------------------------------------------------------------------------//
#include "rc.h"
#include "rc_data.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(commie)
START_NAMESPACE(resources)
//----------------------------------------------------------------------------//
const Image gkOptionsIcon(32, 32, skOptionsIconData);
const Image gkClosedLockIcon(32, 32, skClosedLockIconData);
const Image gkOpenedLockIcon(32, 32, skOpenedLockIconData);

//----------------------------------------------------------------------------//
// Constructor, notice that there is no default constructor.
Image::Image(ut::uint32 img_width,
             ut::uint32 img_height,
             const ut::byte* img_data) : width(img_width)
                                       , height(img_height)
                                       , data(img_data)
{ }

//----------------------------------------------------------------------------//
END_NAMESPACE(resources)
END_NAMESPACE(commie)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//