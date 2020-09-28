//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "systems/render/ve_render_api.h"
#include "ve_render_toolset.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// ve::render::Profiler measures rendering performance.
class Profiler
{
public:
	// Constructor.
	Profiler(Toolset &toolset_ref) noexcept;

	// Draws performance information to the backbuffer.
	void DrawInfo(Context& context, Frame& frame, ut::uint32 display_width, ut::uint32 display_height);

	// Single text string to be displayed.
	struct TextEntry
	{
		// top-left position of the text
		ut::Vector<2, int> position;

		// string with text
		ut::String data;
	};

private:
	// Creates a vertex buffer containing preallocated vertices for the text.
	// Each symbol represents a quad of two triangles, 6 vertices per symbol.
	Buffer CreateTextBuffer();

	// Creates uniform buffer for the quad fragment shader.
	Buffer CreateUniformBuffer();

	// Updates text vertex buffer with up-to-date performance data.
	ut::uint32 UpdateTextBuffer(Context& context,
	                            ut::uint32 display_width,
	                            ut::uint32 display_height,
	                            double fps);

	// Returns a number of symbols in the text vertex buffer.
	ut::uint32 GetNumSymbolsInCache();

	// helper tools
	Toolset &tools;

	// cache of separate strings to be displayed.
	ut::Array<TextEntry> text_cache;
	
	// font texture
	Image font;

	// buffer with separate character quads
	Buffer text_buffer;

	// uniform buffer for a quad shader
	Buffer text_ub;

	// timer to measure fps
	ut::time::PerformanceCounter<128> fps_counter;

	// timer to update cache with desired interval
	ut::time::Counter timer;

	// font metrics
	ut::uint32 font_width;
	ut::uint32 font_height;

	// default path to the font image
	static const char* skFontPath;

	// number of characters in a font texture
	static const ut::uint32 skFontSize;

	// maximum number of characters in a text buffer
	static const ut::uint32 skTextBufferCapacity;

	// offset of the fps entry from the right upper corner in pixels
	static const ut::Vector<2, int> skFpsOffset;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
