//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "systems/render/engine/ve_render_profiler.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// The default path to the font image.
const char* Profiler::skFontPath = "maps/fonts/profiler.png";

// The offset from the borders to a displayed text.
const ut::Vector<2, int> Profiler::skBorderMargin = ut::Vector<2, int>(25, 5);

//----------------------------------------------------------------------------//
// Constructor.
Profiler::Profiler(Device& in_device,
                   ImageLoader& in_img_loader,
                   Config<Settings>& in_config,
                   ResourceManager& in_rc_mgr,
                   SamplerCache& in_sampler_cache,
                   FrameManager& in_frame_mgr) noexcept : device(in_device)
                                                        , img_loader(in_img_loader)
                                                        , config(in_config)
                                                        , rc_mgr(in_rc_mgr)
                                                        , sampler_cache(in_sampler_cache)
                                                        , frame_mgr(in_frame_mgr)
                                                        , font(img_loader.LoadFromFile(skFontPath).MoveOrThrow())
                                                        , display_input_assembly(Frame::CreateInputAssemblyState())
                                                        , font_width(font.GetInfo().width / skFontSize)
                                                        , font_height(font.GetInfo().height)
{
	// create frame data
	for (size_t frame_it = 0; frame_it < config.frames_in_flight; ++frame_it)
	{
		QueryBuffer::Info query_info;
		query_info.type = QueryBuffer::Type::timestamp;
		query_info.count = skMaxGpuTimestampQueries;
		ut::Result<QueryBuffer, ut::Error> query_buffer = device.CreateQueryBuffer(query_info);
		frame_data.Add({ ut::Array<Measurement>(), query_buffer.MoveOrThrow(), 0 });
	}

	// start counters
	fps_counter.Start();
	counter.Start();
}

// Draws performance statistics information to the viewport's backbuffer.
void Profiler::DrawStats(Context& context,
                         Frame& frame,
                         ui::Viewport::Id viewport_id,
                         ut::uint32 display_width,
                         ut::uint32 display_height)
{
	// check if such viewport buffer exists
	ut::Optional<ViewportTextBuffer&> viewport_buffer = viewport_cache.Find(viewport_id);
	if (!viewport_buffer)
	{
		ViewportTextBuffer new_buffer =
		{
			CreateTextBuffer(),
			CreateUniformBuffer(),
			0,
			viewport_id
		};

		viewport_cache.Insert(viewport_id, ut::Move(new_buffer));

		viewport_buffer = viewport_cache.Find(viewport_id);
		UT_ASSERT(viewport_buffer.HasValue());

		update_viewport_cache = true;
	}

	// update vertex buffer
	if (update_viewport_cache)
	{
		UpdateTextBuffer(context,
		                 viewport_buffer.Get(),
		                 display_width,
		                 display_height);
	}

	// draw text
	if (viewport_buffer->symbol_count)
	{
		// update uniform buffer
		Frame::DisplayUB display_ub;
		display_ub.color = ut::Color<4>(0.8, 0.9f, 1.0f, 0.85f);
		ut::Optional<ut::Error> update_ub_error = rc_mgr.UpdateBuffer(context,
		                                                              viewport_buffer->uniform_buffer,
		                                                              &display_ub);
		if (update_ub_error)
		{
			throw update_ub_error.Move();
		}

		// set font texture
		frame.quad_desc_set.tex2d.BindImage(font);

		// set font color
		frame.quad_desc_set.ub.BindUniformBuffer(viewport_buffer->uniform_buffer);

		// set sampler
		frame.quad_desc_set.sampler.BindSampler(sampler_cache.linear_clamp);

		// draw symbols
		context.BindDescriptorSet(frame.quad_desc_set);
		context.BindVertexBuffer(viewport_buffer->vertex_buffer, 0);
		context.Draw(viewport_buffer->symbol_count * 6, 0);
	}
}

// Starts a counter and returns a scope object finishing
// the measurement in the end of a scope.
//    @param stat - a statistics ID of the measurement.
//    @param render_context - an optional reference to the render context,
//                            used for measuring GPU time.
//    @param viewport_id - an optional index of the viewport in which the
//                         measurement occurs.
//    @param subgroup - an optional measurement subgroup.
//    @return - an optional new ve::render::Profiler::ScopeCounter object
//              or nothing if profiling is turned off.
ut::Optional<Profiler::ScopeCounter> Profiler::CreateScopeCounter(Stat stat,
                                                                  ut::Optional<Context&> render_context,
                                                                  ut::Optional<ui::Viewport::Id> viewport_id,
                                                                  ut::Optional<StatSubgroup> subgroup)
{
	if (!config.show_render_stat)
	{
		return ut::Optional<Profiler::ScopeCounter>();
	}

	const ut::uint32 counter_id = StartCounter(stat,
	                                           render_context,
	                                           ut::Move(viewport_id),
	                                           ut::Move(subgroup));

	return Profiler::ScopeCounter(*this, counter_id, ut::Move(render_context));
}

// Starts a counter.
//    @param stat - a statistics ID of the measurement.
//    @param render_context - an optional reference to the render context,
//                            used for measuring GPU time.
//    @param viewport_id - an optional index of the viewport in which the
//                         measurement occurs.
//    @param subgroup - an optional measurement subgroup.
//    @return - an ID of this measurement.
ut::uint32 Profiler::StartCounter(Stat stat,
                                  ut::Optional<Context&> render_context,
                                  ut::Optional<ui::Viewport::Id> viewport_id,
                                  ut::Optional<StatSubgroup> subgroup)
{
	FrameData& frame = frame_data[cur_frame_id];

	Measurement measurement;
	measurement.id = stat;
	measurement.viewport_id = ut::Move(viewport_id);
	measurement.subgroup = ut::Move(subgroup);
	measurement.cpu = counter.GetTime<ut::time::Unit::millisecond, double>();

	if (render_context)
	{
		UT_ASSERT(frame.query_count + 2 <= skMaxGpuTimestampQueries);

		GpuQuery query;
		query.start = frame.query_count;
		query.end = frame.query_count + 1;
		frame.query_count += 2;

		render_context->WriteQuery(frame.query_buffer,
		                           query.start,
		                           QueryBuffer::PipelineStage::top_of_pipe);

		measurement.query = ut::Move(query);
	}

	frame.measurements.Add(ut::Move(measurement));

	return static_cast<ut::uint32>(frame.measurements.Count()) - 1;
}

// Stops a counter.
//    @param measurement_id - an ID of the measurement to finish, must be
//                            previously obtained from the
//                            Profiler::StartCounter() call.
//    @param render_context - an optional reference to the render context,
//                            used for measuring GPU time.
void Profiler::StopCounter(ut::uint32 measurement_id,
                           ut::Optional<Context&> render_context)
{
	FrameData& frame = frame_data[cur_frame_id];

	Measurement& measurement = frame.measurements[measurement_id];
	measurement.cpu = counter.GetTime<ut::time::Unit::millisecond, double>() - measurement.cpu;

	if (render_context)
	{
		const GpuQuery& query = measurement.query.Get();

		render_context->WriteQuery(frame.query_buffer,
		                           query.end,
		                           QueryBuffer::PipelineStage::bottom_of_pipe);
	}
}

// Resets all counters and updates cur_frame_id member.
// Must be called once per frame before writing GPU queries.
void Profiler::ResetCounterAndUpdateFrameId()
{
	// update 2 times per second
	update_viewport_cache = counter.GetTime<ut::time::Unit::second, double>() > 0.5;
	if (update_viewport_cache)
	{
		counter.Start();
	}

	// calculate fps
	fps_counter.Stop();
	double frequency = fps_counter.GetTime<ut::time::Unit::second, double>();
	fps = 1.0 / frequency;
	fps_counter.Start();

	// get current frame index
	cur_frame_id = frame_mgr.GetCurrentFrameId();
}

// Collects GPU queries from the previous cycle and calculates
// the average results both for GPU and CPU measurements.
void Profiler::CollectQueriesAndSaveResults(Context& context)
{
	if (!config.show_render_stat)
	{
		return;
	}

	FrameData& frame = frame_data[cur_frame_id];

	// read query results
	ut::Array<ut::uint64> query_results = context.ReadAndResetQueryBuffer(frame.query_buffer,
		                                                                  0,
		                                                                  frame.query_count);

	for (Measurement& measurement : frame.measurements)
	{
		if (!measurement.query)
		{
			continue;
		}

		UT_ASSERT(measurement.query->start < frame.query_count);
		const ut::uint64 start = query_results[measurement.query->start];
			
		UT_ASSERT(measurement.query->end < frame.query_count);
		const ut::uint64 end = query_results[measurement.query->end];

		const double nanoseconds = static_cast<double>(end - start);

		measurement.gpu = ut::time::Convert<ut::time::Unit::nanosecond,
			                                ut::time::Unit::millisecond,
			                                double>(nanoseconds);
	}

	frame.query_count = 0;

	// update stat history
	const size_t history_count = stat_history.Count();
	if (history_count >= skCycleHistory)
	{
		stat_history.Remove(0);
	}
	stat_history.Add(frame.measurements);

	// calculate average stats
	avg_frame_stats = ut::Move(frame.measurements);
	for (Measurement& measurement : avg_frame_stats)
	{
		const bool has_viewport_id = measurement.viewport_id.HasValue();
		const bool in_subgroup = measurement.subgroup.HasValue();

		auto cmp_measurement = [&](const Measurement& s)
		{

			bool viewport_match = s.viewport_id.HasValue() == has_viewport_id;
			if (viewport_match && has_viewport_id)
			{
				viewport_match = s.viewport_id.Get() == measurement.viewport_id.Get();
			}

			bool subgroup_match = s.subgroup.HasValue() == in_subgroup;
			if (subgroup_match && in_subgroup)
			{
				subgroup_match = s.subgroup.Get() == measurement.subgroup.Get();
			}

			return s.id == measurement.id && viewport_match && subgroup_match;
		};

		measurement.cpu = 0.0;
		measurement.gpu = 0.0;

		double sample_count = 0.0;

		for (const ut::Array<Measurement>& history_frame : stat_history)
		{
			auto find_measurement = ut::FindIf(history_frame.Begin(),
			                                   history_frame.End(),
			                                   cmp_measurement);
			if (!find_measurement)
			{
				continue;
			}

			const Measurement& old_measurement = *find_measurement.Get();

			measurement.cpu += old_measurement.cpu;
			measurement.gpu += old_measurement.gpu;

			sample_count += 1.0;
		}

		measurement.cpu /= sample_count;
		measurement.gpu /= sample_count;
	}
}

// Returns a null-terminated string with the name of the given stat group.
char* Profiler::GetStatName(Stat stat)
{
	switch (stat)
	{
		case Stat::unit_initialization: return "init units";
		case Stat::wait_gpu: return "wait gpu";
		case Stat::wait_sync: return "wait sync";
		case Stat::frame: return "record frame";
		case Stat::batching: return "batching";
		case Stat::ibl: return "ibl";
		case Stat::gbuffer_bake: return "g-buffer";
		case Stat::deferred_shading: return "deferred";
		case Stat::forward_rendering: return "forward";
		case Stat::postprocess: return "postprocess";
		case Stat::submit: return "submit";
	}

	return "unknown";
}

// Creates a vertex buffer containing preallocated vertices for the text.
// Each symbol represents a quad of two triangles, 6 vertices per symbol.
Buffer Profiler::CreateTextBuffer()
{
	const ut::uint32 stride = display_input_assembly.vertex_stride;

	Buffer::Info buffer_info;
	buffer_info.type = Buffer::Type::vertex;
	buffer_info.usage = render::memory::Usage::gpu_read_cpu_write;
	buffer_info.size = skTextBufferCapacity * 6 * stride;
	buffer_info.stride = stride;
	ut::Result<Buffer, ut::Error> buffer_result = device.CreateBuffer(ut::Move(buffer_info));
	return buffer_result.MoveOrThrow();
}

// Creates a uniform buffer for the quad fragment shader.
Buffer Profiler::CreateUniformBuffer()
{
	Buffer::Info buffer_info;
	buffer_info.type = Buffer::Type::uniform;
	buffer_info.usage = render::memory::Usage::gpu_read_cpu_write;
	buffer_info.size = sizeof(Frame::DisplayUB);
	ut::Result<Buffer, ut::Error> buffer_result = device.CreateBuffer(ut::Move(buffer_info));
	return buffer_result.MoveOrThrow();
}

// Updates text vertex buffer with up-to-date performance data.
void Profiler::UpdateTextBuffer(Context& context,
                                ViewportTextBuffer& viewport_buffer,
                                ut::uint32 display_width,
                                ut::uint32 display_height)
{
	// reset cache
	ut::Array<TextEntry> text_cache;

	// how many symbols to display
	ut::uint32 symbols_to_draw = 0;

	// add fps entry
	if (config.show_fps)
	{
		TextEntry fps_entry;
		fps_entry.data.Print("%.1lf", fps);
		const ut::uint32 fps_width = static_cast<ut::uint32>(fps_entry.data.Length()) * font_width;
		fps_entry.position.X() = display_width - fps_width - skBorderMargin.X();
		fps_entry.position.Y() = skBorderMargin.Y();
		if (!text_cache.Add(ut::Move(fps_entry)))
		{
			throw ut::Error(ut::error::out_of_memory);
		}
	}

	// skip if no statistics needs to be displayed
	if (config.show_render_stat)
	{
		// add caption
		TextEntry caption_row;
		const ut::String caption_format = ut::String("%s  %") +
		                                  ut::Print(skStatRowLen) +
			                              ut::String("s  %s");
		caption_row.data.Print(caption_format.GetAddress(), "CPU", " ", "GPU");
		caption_row.position.X() = skBorderMargin.X();
		caption_row.position.Y() = skBorderMargin.Y();
		if (!text_cache.Add(ut::Move(caption_row)))
		{
			throw ut::Error(ut::error::out_of_memory);
		}

		// add measurements
		int y_offset = font_height;
		for (const Measurement& measurement : avg_frame_stats)
		{
			// skip other viewports
			if (measurement.viewport_id &&
				measurement.viewport_id.Get() != viewport_buffer.viewport_id)
			{
				continue;
			}

			// skip subgroups
			if (measurement.subgroup)
			{
				continue;
			}

			// place a stat name in the center
			const ut::String stat_name = GetStatName(measurement.id);
			const int first_char_index = static_cast<int>(skStatRowLen) / 2 -
			                             static_cast<int>(stat_name.Length()) / 2;
			ut::String entry_line;
			for (int i = 0; i <= skStatRowLen; ++i)
			{
				int id = i - first_char_index;
				const char c = (id < 0 || id > stat_name.Length()) ?
				               '-' : stat_name[id];
				entry_line.Append(c);
			}

			// add a new row
			TextEntry stat_row;
			stat_row.data.Print("%05.2lf%s%05.2lf",
			                    measurement.cpu,
			                    entry_line.GetAddress(),
			                    measurement.gpu);
			stat_row.position.X() = skBorderMargin.X();
			stat_row.position.Y() = skBorderMargin.Y() + y_offset;
			if (!text_cache.Add(ut::Move(stat_row)))
			{
				throw ut::Error(ut::error::out_of_memory);
			}

			// update vertical offset
			y_offset += font_height;
		}
	}

	// check cache size
	if (GetNumSymbolsInCache(text_cache) > skTextBufferCapacity)
	{
		ut::log.Lock() << "Render profiler: Warning! Exceeded maximum"
		               << "number of characters for output buffer." << ut::cret;
		return;
	}

	// calculate eye-space metrics of a single character
	const float char_width = 2.0f * static_cast<float>(font_width) /
	                         static_cast<float>(display_width);
	const float char_height = 2.0f * static_cast<float>(font_height) /
	                          static_cast<float>(display_height);

	// calculate width of a single character in texture space
	const float tex_width = 1.0f / static_cast<float>(skFontSize);

	// map text vertex buffer
	ut::Result<void*, ut::Error> map_result = context.MapBuffer(viewport_buffer.vertex_buffer, memory::CpuAccess::write);
	if (!map_result)
	{
		throw map_result.MoveAlt();
	}

	// get pointer to the first vertice
	VertexReflector vertices(display_input_assembly, map_result.Get());

	// update vertices
	ut::uint32 offset = 0;
	const ut::uint32 text_count = static_cast<ut::uint32>(text_cache.Count());
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

			// move to the next quad
			offset += 6;
		}
	}

	// finish vertex buffer
	context.UnmapBuffer(viewport_buffer.vertex_buffer);
	viewport_buffer.symbol_count = symbols_to_draw;
}

// Returns a number of symbols in the text vertex buffer.
ut::uint32 Profiler::GetNumSymbolsInCache(const ut::Array<TextEntry>& text_cache)
{
	ut::uint32 count = 0;
	const size_t text_count = text_cache.Count();
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