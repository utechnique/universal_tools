//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "systems/ui/desktop/ve_icon.h"
//----------------------------------------------------------------------------//
#if VE_DESKTOP
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(ui)
//----------------------------------------------------------------------------//
// Constructor, initializes fltk image structure.
Icon::Icon(ut::uint32 image_width,
           ut::uint32 image_height,
           ut::Array< ut::Color<4, ut::byte> > pixels) : width(image_width)
                                                       , height(image_height)
                                                       , data(ut::Move(pixels))
                                                       , image(ut::MakeUnique<Fl_RGB_Image>(data.GetFirst().GetData(),
                                                                                            width, height, 4))
{}

// Moving is allowed.
Icon::Icon(Icon&&) = default;
Icon& Icon::operator = (Icon&&) = default;

// Creates icon data for the collapse icon.
Icon Icon::CreateCollapse(ut::uint32 width,
                          ut::uint32 height,
                          const ut::Color<4, ut::byte>& color,
                          bool expanded)
{
	ut::Array< ut::Color<4, ut::byte> > icon_data(width * height);

	const int size = ut::Min<ut::uint32>(width, height);
	const int half_size = size / 2;
	const int margin = size / 4;
	const int inv_margin = size - margin - 1;
	const bool odd = size % 2 == 0 ? true : false;

	for (int y = 0; y < size; y++)
	{
		for (int x = 0; x < size; x++)
		{
			ut::Color<4, ut::byte>& pixel = icon_data[y*size + x];
			pixel = color;
			pixel.A() = 0;

			const bool inside = x >= margin &&
			                    x <= inv_margin &&
			                    y >= margin &&
			                    y <= inv_margin;
			if (!inside)
			{
				continue;
			}

			const int oy = y + size / (expanded ? 6 : -6);

			if (!expanded && oy > half_size)
			{
				continue;
			}
			else if (!expanded && oy == margin - 1)
			{
				pixel.A() = color.A();
				continue;
			}
			else if (expanded && oy < half_size)
			{
				continue;
			}

			const int line_width = 1;
			const int d0 = ut::Abs(oy - x);
			const int d1 = ut::Abs(oy - size + x + 1);

			if (!expanded && odd && oy == half_size)
			{
				const int ix = x - half_size;
				if (ix == 0 || ix == -1)
				{
					pixel.A() = color.A() / 2;
				}
				continue;
			}

			if (d0 < line_width || d1 < line_width)
			{
				pixel.A() = color.A();
			}
			else if (d0 < line_width + 1 || d1 < line_width + 1)
			{
				pixel.A() = color.A() / 2;
			}
		}
	}

	return Icon(width, height, ut::Move(icon_data));
}

// Creates plus icon.
Icon Icon::CreatePlus(ut::uint32 width,
                      ut::uint32 height,
                      const ut::Color<4, ut::byte>& color,
                      ut::uint32 thickness,
                      ut::uint32 margin)
{
	ut::Array< ut::Color<4, ut::byte> > icon_data(width * height);

	const int size = ut::Min<ut::uint32>(width, height);
	const int half_width = width / 2;
	const int half_height = height / 2;
	const int half_thickness = thickness / 2;
	const int inv_x_margin = width - margin - 1;
	const int inv_y_margin = height - margin - 1;
	const int signed_height = static_cast<int>(height);
	const int signed_width = static_cast<int>(width);

	for (int y = 0; y < signed_height; y++)
	{
		for (int x = 0; x < signed_width; x++)
		{
			ut::Color<4, ut::byte>& pixel = icon_data[y*width + x];
			pixel = color;
			pixel.A() = 0;

			const bool inside = x >= static_cast<int>(margin) &&
			                    x <= inv_x_margin &&
			                    y >= static_cast<int>(margin) &&
			                    y <= inv_y_margin;
			if (!inside)
			{
				continue;
			}

			const int ox = x >= half_width ? 0 : 1;
			const int oy = y >= half_height ? 0 : 1;

			const int dx = ut::Abs<int>(x + ox - half_width);
			const int dy = ut::Abs<int>(y + oy - half_height);

			if (dx >= half_thickness && dy >= half_thickness)
			{
				continue;
			}

			pixel.A() = color.A();
		}
	}

	return Icon(width, height, ut::Move(icon_data));
}

// Creates cross icon (thickness is one pixel).
Icon Icon::CreateCross(ut::uint32 width,
                       ut::uint32 height,
                       const ut::Color<4, ut::byte>& color,
                       ut::uint32 margin)
{
	ut::Array< ut::Color<4, ut::byte> > icon_data(width * height);

	const int size = ut::Min<ut::uint32>(width, height);
	const int odd_fixer = size % 2 == 0 ? 1 : 0;
	const int signed_margin = static_cast<int>(margin);
	const int inv_margin = size - signed_margin - odd_fixer;
	const int signed_height = static_cast<int>(height);
	const int signed_width = static_cast<int>(width);

	for (int y = 0; y < signed_height; y++)
	{
		for (int x = 0; x < signed_width; x++)
		{
			ut::Color<4, ut::byte>& pixel = icon_data[y*size + x];
			pixel = color;
			pixel.A() = 0;

			bool inside = x >= signed_margin &&
			              x <= inv_margin &&
			              y >= signed_margin &&
			              y <= inv_margin;
			if (!inside)
			{
				continue;
			}

			const int line_width = 1;

			int d0 = ut::Abs(y - x);
			int d1 = ut::Abs(y - size + x + odd_fixer);

			if ((x == signed_margin || x == inv_margin) &&
				(y == signed_margin || y == inv_margin))
			{
				pixel.A() = color.A() - color.A() / 4;
			}
			else if (d0 < line_width || d1 < line_width)
			{
				pixel.A() = color.A();
			}
			else if (d0 < line_width + 1 || d1 < line_width + 1)
			{
				pixel.A() = color.A() / 2;
			}
		}
	}

	return Icon(width, height, ut::Move(icon_data));
}

// Creates an icon with two arrows in different directions (thickness is one pixel).
Icon Icon::CreateChange(ut::uint32 width,
                        ut::uint32 height,
                        const ut::Color<4, ut::byte>& color,
                        ut::uint32 margin)
{
	ut::Array< ut::Color<4, ut::byte> > icon_data(width * height);

	const int imargin = static_cast<int>(margin);
	const int size = ut::Min<ut::uint32>(width, height);
	const int half_size = size / 2;
	const int quad_size = size / 4;
	const int half_size_x = size / 2 - imargin;
	const int odd = size % 2 == 0 ? 1 : 0;

	for (int y = 0; y < size; y++)
	{
		for (int x = 0; x < size; x++)
		{
			ut::Color<4, ut::byte>& pixel = icon_data[y*size + x];
			pixel = color;
			pixel.A() = 0;

			if (x < imargin || x >= (size - imargin))
			{
				continue;
			}

			if (odd == 0 && y == 0)
			{
				continue;
			}

			const bool up = y < half_size;
			const int ox = x + (up ? half_size_x : -half_size_x);
			const int oy = y + (up ? quad_size : -quad_size);

			const int line_width = 1;
			const int d0 = ut::Abs(ox - oy);
			const int d1 = ut::Abs(ox - size + oy + 1);

			if (y == quad_size || y == quad_size * 3 ||
			    y == quad_size - odd || y == quad_size * 3 - odd)
			{
				pixel.A() = color.A();
			}
			else if (d0 < line_width || d1 < line_width)
			{
				pixel.A() = color.A();
			}
			else if (d0 < line_width + 1 || d1 < line_width + 1)
			{
				pixel.A() = color.A() / 2;
			}
		}
	}

	return Icon(width, height, ut::Move(icon_data));
}

// Creates an icon with a trash bin.
Icon Icon::CreateTrashBin(ut::uint32 width,
                          ut::uint32 height,
                          const ut::Color<4, ut::byte>& color,
                          ut::uint32 margin,
                          ut::uint32 thickness)
{
	ut::Array< ut::Color<4, ut::byte> > icon_data(width * height);
	const int iw = static_cast<int>(width);
	const int ih = static_cast<int>(height);
	const int it = static_cast<int>(thickness);
	const int im = static_cast<int>(margin);

	const int ef_width = iw - im;
	const int ef_height = ih - im;
	const int bin_width = ef_width / 2 + ef_width / 4;
	const int bin_half_width = bin_width / 2;
	const int bin_quad_height = ef_height / 4;
	const int half_width = iw / 2;
	const int half_height = ih / 2;
	const int cap_height = im + bin_quad_height;
	const int cap_width = bin_width / 2 + bin_width / 4;
	const int cap_half_width = cap_width / 2;
	const int cap_lower_width = ef_width;
	const int cap_lower_half_width = cap_lower_width / 2;
	const int rib_offset_x = bin_width / 6 + ut::Max(1, it / 2);
	const int rib_offset_y = (ef_height) / 6;

	for (int y = 0; y < ih; y++)
	{
		for (int x = 0; x < iw; x++)
		{
			ut::Color<4, ut::byte>& pixel = icon_data[y*width + x];
			pixel = color;
			pixel.A() = 0;

			// bin lines
			if (y >= cap_height && y < ih - im && x >= half_width - bin_half_width && x < half_width + bin_half_width &&
			   (x < half_width - bin_half_width + it || x >= half_width + bin_half_width - it ||
			    y < cap_height + it || y >= ih - im - it))
			{
				pixel.A() = 255;
			}

			// cap lines
			if (y >= im && y < cap_height && x >= half_width - cap_half_width && x < half_width + cap_half_width &&
			   (x < half_width - cap_half_width + it || x >= half_width + cap_half_width - it || y < im + it))
			{
				pixel.A() = 255;
			}

			// cap lower line
			if (x >= half_width - cap_lower_half_width && x < half_width + cap_lower_half_width &&
			    y >= cap_height && y < cap_height + it)
			{
				pixel.A() = 255;
			}
			
			// bin ribs
			if (y >= cap_height + rib_offset_y && y < ih - im - rib_offset_y &&          
			   ((x >= half_width - rib_offset_x && x < half_width - rib_offset_x + it) ||
			     x < half_width + rib_offset_x && x >= half_width + rib_offset_x - it))
			{
				pixel.A() = 195;
			}

		}
	}

	return Icon(width, height, ut::Move(icon_data));
}

//----------------------------------------------------------------------------//
END_NAMESPACE(ui)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif // VE_DESKTOP
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
