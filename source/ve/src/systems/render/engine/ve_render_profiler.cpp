//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "systems/render/engine/ve_render_profiler.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// Default path to the font image.
const char* Profiler::skFontPath = "maps/fonts/profiler.png";

// Number of characters in a font texture.
const ut::uint32 Profiler::skFontSize = 95;

// Maximum number of characters in a text buffer.
const ut::uint32 Profiler::skTextBufferCapacity = 4096;

// Offset of the fps entry from the right upper corner in pixels.
const ut::Vector<2, int> Profiler::skFpsOffset = ut::Vector<2, int>(25, 5);

//----------------------------------------------------------------------------//
// Constructor.
Profiler::Profiler(Toolset &toolset_ref) noexcept : tools(toolset_ref)
                                                  , font(tools.img_loader.Load(skFontPath).MoveOrThrow())
                                                  , display_input_assembly(Frame::CreateInputAssemblyState())
                                                  , text_buffer(CreateTextBuffer())
                                                  , text_ub(CreateUniformBuffer())
                                                  , font_width(font.GetInfo().width / skFontSize)
                                                  , font_height(font.GetInfo().height)
{
	fps_counter.Start();
	timer.Start();
}

//----------------------------------------------------------------------------->
// Draws performance information to the backbuffer.
void Profiler::DrawInfo(Context& context, Frame& frame, ut::uint32 display_width, ut::uint32 display_height)
{
	// calculate fps
	fps_counter.Stop();
	double frequency = fps_counter.GetTime<ut::time::seconds, double>();
	double fps = 1.0 / frequency;
	fps_counter.Start();
	
	// update happens 2 times per second
	ut::uint32 symbols_to_draw = 0;
	bool needs_update = timer.GetTime<ut::time::seconds, double>() > 0.5;
	if (needs_update)
	{
		// restart timer
		timer.Start();

		symbols_to_draw = UpdateTextBuffer(context,
		                                   display_width,
		                                   display_height,
		                                   fps);
	}
	else
	{
		symbols_to_draw = GetNumSymbolsInCache();
	}

	// draw text
	if (symbols_to_draw > 0)
	{
		// update uniform buffer
		Frame::DisplayUB display_ub;
		display_ub.color = ut::Color<4>(0.8, 0.9f, 1.0f, 0.85f);
		ut::Optional<ut::Error> update_ub_error = tools.rc_mgr.UpdateBuffer(context, text_ub, &display_ub);
		if (update_ub_error)
		{
			throw update_ub_error.Move();
		}

		// set font texture
		frame.quad_desc_set.tex2d.BindImage(font);

		// set font color
		frame.quad_desc_set.ub.BindUniformBuffer(text_ub);

		// set sampler
		frame.quad_desc_set.sampler.BindSampler(tools.sampler_cache.linear_clamp);

		// draw symbols
		context.BindDescriptorSet(frame.quad_desc_set);
		context.BindVertexBuffer(text_buffer, 0);
		context.Draw(symbols_to_draw * 6, 0);
	}
}

//----------------------------------------------------------------------------->
// Creates a vertex buffer containing preallocated vertices for the text.
// Each symbol represents a quad of two triangles, 6 vertices per symbol.
Buffer Profiler::CreateTextBuffer()
{
	const ut::uint32 stride = display_input_assembly.vertex_stride;

	Buffer::Info buffer_info;
	buffer_info.type = Buffer::vertex;
	buffer_info.usage = render::memory::gpu_read_cpu_write;
	buffer_info.size = skTextBufferCapacity * 6 * stride;
	buffer_info.stride = stride;
	ut::Result<Buffer, ut::Error> buffer_result = tools.device.CreateBuffer(ut::Move(buffer_info));
	return buffer_result.MoveOrThrow();
}

//----------------------------------------------------------------------------->
// Creates uniform buffer for the quad fragment shader.
Buffer Profiler::CreateUniformBuffer()
{
	Buffer::Info buffer_info;
	buffer_info.type = Buffer::uniform;
	buffer_info.usage = render::memory::gpu_read_cpu_write;
	buffer_info.size = sizeof(Frame::DisplayUB);
	ut::Result<Buffer, ut::Error> buffer_result = tools.device.CreateBuffer(ut::Move(buffer_info));
	return buffer_result.MoveOrThrow();
}

//----------------------------------------------------------------------------->
// Updates text vertex buffer with up-to-date performance data.
ut::uint32 Profiler::UpdateTextBuffer(Context& context,
                                      ut::uint32 display_width,
                                      ut::uint32 display_height,
                                      double fps)
{
	// reset cache
	text_cache.Empty();

	// how many symbols to display
	ut::uint32 symbols_to_draw = 0;

	// add fps entry
	TextEntry fps_entry;
	fps_entry.data.Print("%.1lf", fps);
	const ut::uint32 fps_width = static_cast<ut::uint32>(fps_entry.data.Length()) * font_width;
	fps_entry.position.X() = display_width - fps_width - skFpsOffset.X();
	fps_entry.position.Y() = skFpsOffset.Y();
	if (!text_cache.Add(ut::Move(fps_entry)))
	{
		throw ut::Error(ut::error::out_of_memory);
	}

	// check cache size
	if (GetNumSymbolsInCache() > skTextBufferCapacity)
	{
		ut::log.Lock() << "Render profiler: Warning! Exceeded maximum"
		               << "number of characters for output buffer." << ut::cret;
		text_cache.Empty();
		return 0;
	}

	// calculate eye-space metrics of a single character
	const float char_width = 2.0f * static_cast<float>(font_width) /
	                         static_cast<float>(display_width);
	const float char_height = 2.0f * static_cast<float>(font_height) /
	                          static_cast<float>(display_height);

	// calculate width of a single character in texture space
	const float tex_width = 1.0f / static_cast<float>(skFontSize);

	// map text vertex buffer
	ut::Result<void*, ut::Error> map_result = context.MapBuffer(text_buffer, ut::access_write);
	if (!map_result)
	{
		throw map_result.MoveAlt();
	}

	// get pointer to the first vertice
	VertexReflector vertices(display_input_assembly, map_result.Get());

	// update vertices
	const ut::uint32 text_count = static_cast<ut::uint32>(text_cache.GetNum());
	for (ut::uint32 i = 0; i < text_count; i++)
	{
		// scale upper-left position to 0..1 size
		float x = static_cast<float>(text_cache[i].position.X());
		float y = static_cast<float>(text_cache[i].position.Y());
		x /= static_cast<float>(display_width);
		y /= static_cast<float>(display_height);

		// transform position to eye-space
		x = -1.0f + x * 2.0f;
		y = 1.0f - y * 2.0f;

		const ut::String& text = text_cache[i].data;
		const ut::uint32 length = static_cast<ut::uint32>(text.Length());
		for (ut::uint32 j = 0; j < length; j++)
		{
			ut::uint32 offset = j * 6;

			// calculate character id in the font texture
			char char_id = text[j] - 32;
			if (char_id < 0)
			{
				char_id = 0;
			}

			// calculate texcoord offset to the desired character
			float tx = char_id * tex_width;

			// 4 vertices
			ut::Vector<2> pos_lu(x, y),
			              pos_ru(x + char_width, y),
			              pos_lb(x, y - char_height),
			              pos_rb(x + char_width, y - char_height);
			ut::Vector<2> tex_lu(tx, 0),
			              tex_ru(tx + tex_width, 0),
			              tex_lb(tx, 1),
			              tex_rb(tx + tex_width, 1);

			// first triangle
			vertices.Get<vertex_traits::position>(offset + 0).Write(pos_lu);
			vertices.Get<vertex_traits::texcoord>(offset + 0).Write(tex_lu);
			vertices.Get<vertex_traits::position>(offset + 1).Write(pos_ru);
			vertices.Get<vertex_traits::texcoord>(offset + 1).Write(tex_ru);
			vertices.Get<vertex_traits::position>(offset + 2).Write(pos_lb);
			vertices.Get<vertex_traits::texcoord>(offset + 2).Write(tex_lb);

			// second triangle
			vertices.Get<vertex_traits::position>(offset + 3).Write(pos_ru);
			vertices.Get<vertex_traits::texcoord>(offset + 3).Write(tex_ru);
			vertices.Get<vertex_traits::position>(offset + 4).Write(pos_lb);
			vertices.Get<vertex_traits::texcoord>(offset + 4).Write(tex_lb);
			vertices.Get<vertex_traits::position>(offset + 5).Write(pos_rb);
			vertices.Get<vertex_traits::texcoord>(offset + 5).Write(tex_rb);

			// one more symbol to draw
			symbols_to_draw++;

			// update horizontal position
			x += char_width;
		}
	}

	// finish vertex buffer
	context.UnmapBuffer(text_buffer);

	// success
	return symbols_to_draw;
}

//----------------------------------------------------------------------------->
// Returns a number of symbols in the text vertex buffer.
ut::uint32 Profiler::GetNumSymbolsInCache()
{
	ut::uint32 count = 0;
	const size_t text_count = text_cache.GetNum();
	for (size_t i = 0; i < text_count; i++)
	{
		count += static_cast<ut::uint32>(text_cache[i].data.Length());
	}
	return count;
}

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//