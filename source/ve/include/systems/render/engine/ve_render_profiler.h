//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "systems/render/ve_render_api.h"
#include "ve_render_cfg.h"
#include "ve_render_frame.h"
#include "ve_render_rc_mgr.h"
#include "ve_render_image_loader.h"
#include "ve_sampler_cache.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// ve::render::Profiler measures rendering performance.
class Profiler
{
public:
	// Available performance stat measurements.
	enum class Stat
	{
		unit_initialization,
		wait_gpu,
		wait_sync,
		frame,
		batching,
		ibl,
		gbuffer_bake,
		deferred_shading,
		forward_rendering,
		postprocess,
		submit,
		count
	};

	// Each stat can be accompanied with one of these subgroups,
	// that is, the measured stat is a part of a larger group.
	enum class StatSubgroup
	{
		ibl
	};

	// Two queries are needed to measure GPU performace: one before
	// the desired commands and one after.
	struct GpuQuery
	{
		ut::uint32 start;
		ut::uint32 end;
	};

	// Contains the results of measuring the time in milliseconds
	// that is needed for CPU (and optionally GPU) to execute the
	// desired section of code.
	struct Measurement
	{
		// Statistics group this measurement belongs to.
		Stat id;

		// CPU time in milliseconds.
		double cpu = 0.0;

		// GPU time in milliseconds.
		double gpu = 0.0;

		// GPU query indices.
		ut::Optional<GpuQuery> query;

		// A viewport index the measurement belongs to.
		ut::Optional<ui::Viewport::Id> viewport_id;

		// Indicates if this measurement is a part of bigger measurement.
		ut::Optional<StatSubgroup> subgroup;
	};

	// Contains a set of measurements that were
	// taken within a particular frame.
	struct FrameData
	{
		ut::Array<Measurement> measurements;
		QueryBuffer query_buffer;
		ut::uint32 query_count;
	};

	// Single text string to be displayed.
	struct TextEntry
	{
		// Top-left position of the text.
		ut::Vector<2, int> position;

		// A string with a text.
		ut::String data;
	};

	// Contains a vertex and uniform buffers for the
	// profiler data to be displayed in a particular viewport.
	struct ViewportTextBuffer
	{
		Buffer vertex_buffer;
		Buffer uniform_buffer;
		ut::uint32 symbol_count;
		ui::Viewport::Id viewport_id;
	};

	// Stops a measurement counter automatically in destructor. This is
	// intended to be a more convenient alternative to the
	// Profiler::Start/StopCounter call pair.
	class ScopeCounter
	{
	public:
		ScopeCounter(Profiler& in_profiler,
		             ut::uint32 in_stat_index,
		             ut::Optional<Context&> in_context) : profiler(in_profiler)
		                                                , stat_index(in_stat_index)
		                                                , render_context(ut::Move(in_context))
		{}

		ScopeCounter(ScopeCounter&& other) noexcept : profiler(other.profiler)
		                                            , stat_index(other.stat_index)
		                                            , render_context(ut::Move(other.render_context))
		{
			other.stat_index = skInvalidStatId;
		}

		~ScopeCounter()
		{
			if (stat_index == skInvalidStatId)
			{
				return;
			}

			profiler.StopCounter(stat_index, render_context);
		}

		// Copying/assignment is prohibited.
		ScopeCounter& operator =(ScopeCounter&& other) noexcept = delete;
		ScopeCounter(const ScopeCounter&) noexcept = delete;
		ScopeCounter& operator =(const ScopeCounter&) noexcept = delete;

	private:
		Profiler& profiler;
		ut::uint32 stat_index;
		ut::Optional<Context&> render_context;
		static constexpr ut::uint32 skInvalidStatId = -1;
	};

	// Constructor.
	Profiler(Device& in_device,
	         ImageLoader& in_img_loader,
	         Config<Settings>& in_config,
	         ResourceManager& in_rc_mgr,
	         SamplerCache& in_sampler_cache,
	         FrameManager& in_frame_mgr) noexcept;

	// Draws performance statistics information to the viewport's backbuffer.
	void DrawStats(Context& context,
	               Frame& frame,
	               ui::Viewport::Id viewport_id,
	               ut::uint32 display_width,
	               ut::uint32 display_height);

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
	ut::Optional<ScopeCounter> CreateScopeCounter(Stat stat,
	                                              ut::Optional<Context&> render_context = ut::Optional<Context&>(),
	                                              ut::Optional<ui::Viewport::Id> viewport_id = ut::Optional<ui::Viewport::Id>(),
	                                              ut::Optional<StatSubgroup> subgroup = ut::Optional<StatSubgroup>());

	// Starts a counter.
	//    @param stat - a statistics ID of the measurement.
	//    @param render_context - an optional reference to the render context,
	//                            used for measuring GPU time.
	//    @param viewport_id - an optional index of the viewport in which the
	//                         measurement occurs.
	//    @param subgroup - an optional measurement subgroup.
	//    @return - an ID of this measurement.
	ut::uint32 StartCounter(Stat stat,
	                        ut::Optional<Context&> render_context = ut::Optional<Context&>(),
	                        ut::Optional<ui::Viewport::Id> viewport_id = ut::Optional<ui::Viewport::Id>(),
	                        ut::Optional<StatSubgroup> subgroup = ut::Optional<StatSubgroup>());

	// Stops a counter.
	//    @param measurement_id - an ID of the measurement to finish, must be
	//                            previously obtained from the
	//                            Profiler::StartCounter() call.
	//    @param render_context - an optional reference to the render context,
	//                            used for measuring GPU time.
	void StopCounter(ut::uint32 measurement_id,
	                 ut::Optional<Context&> render_context = ut::Optional<Context&>());

	// Resets all counters and updates cur_frame_id member.
	// Must be called once per frame before writing GPU queries.
	void ResetCounterAndUpdateFrameId();

	// Collects GPU queries from the previous cycle and calculates
	// the average results both for GPU and CPU measurements.
	void CollectQueriesAndSaveResults(Context& context);

private:
	// Returns a null-terminated string with the name of the given stat group.
	static char* GetStatName(Stat stat);

	// Creates a vertex buffer containing preallocated vertices for the text.
	// Each symbol represents a quad of two triangles, 6 vertices per symbol.
	Buffer CreateTextBuffer();

	// Creates a uniform buffer for the quad fragment shader.
	Buffer CreateUniformBuffer();

	// Updates text vertex buffer with up-to-date performance data.
	void UpdateTextBuffer(Context& context,
	                      ViewportTextBuffer& viewport_buffer,
	                      ut::uint32 display_width,
	                      ut::uint32 display_height);

	// Returns a number of symbols in the text vertex buffer.
	ut::uint32 GetNumSymbolsInCache(const ut::Array<TextEntry>& text_cache);

	// The default path to the font image
	static const char* skFontPath;

	// The offset from the borders to a displayed text.
	static const ut::Vector<2, int> skBorderMargin;

	// The number of characters in a font texture
	static constexpr ut::uint32 skFontSize = 95;

	// The maximum number of characters in a text buffer
	static constexpr ut::uint32 skTextBufferCapacity = 4096;

	// The maximum number of GPU timestamp queries used by this profiler.
	static constexpr ut::uint32 skMaxGpuTimestampQueries = 256;

	// The number of frames (cycles) to take into account for
	// averaging performance measurements.
	static constexpr ut::uint32 skCycleHistory = 64;

	// The length in symbols of the displayed stat row.
	static constexpr ut::uint32 skStatRowLen = 14;

	// Helper tools.
	Device& device;
	ImageLoader& img_loader;
	Config<Settings>& config;
	ResourceManager& rc_mgr;
	SamplerCache& sampler_cache;
	FrameManager& frame_mgr;

	// Precise time counter to measure cpu performance.
	ut::time::Counter counter;
	
	// An image with the profiler font.
	Image font;

	// The input assembly state that is used to draw primitives
	// directly to a display
	InputAssemblyState display_input_assembly;

	// A counter to measure fps.
	ut::time::PerformanceCounter<skCycleHistory> fps_counter;

	// Current FPS rate.
	double fps = 0.0;

	// Stat history, size=Profiler::skCycleHistory.
	ut::Array<ut::Array<Measurement>> stat_history;

	// Average stats for the current frame.
	ut::Array<Measurement> avg_frame_stats;

	// A set of cached vertex buffers per viewport.
	ut::HashMap<ui::Viewport::Id, ViewportTextBuffer> viewport_cache;

	// Indicates if cached vertex buffers must be updated in
	// current frame for all vieports.
	bool update_viewport_cache = false;

	// A set of GPU queries per frame-in-flight.
	ut::Array<FrameData> frame_data;

	// The current frame-in-flight index
	ut::uint32 cur_frame_id = 0;

	// font metrics
	ut::uint32 font_width;
	ut::uint32 font_height;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
