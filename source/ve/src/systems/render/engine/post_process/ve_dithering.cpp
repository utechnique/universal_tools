//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "systems/render/engine/post_process/ve_dithering.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
START_NAMESPACE(postprocess)
//----------------------------------------------------------------------------//
// Cache name and path.
const char* Dithering::Cache::skMetaName = "dithering";
const char* Dithering::Cache::skFileName = Dithering::Cache::skMetaName;

//----------------------------------------------------------------------------//
// Gradient dithering (per-view) data constructor.
Dithering::ViewData::ViewData(Buffer in_gradient_dithering_buffer) :
	gradient_dithering_buffer(ut::Move(in_gradient_dithering_buffer))
{}

// Dithering constructor.
Dithering::Dithering(Toolset& toolset,
                     RenderPass& in_color_only_pass) : tools(toolset)
                                                     , fullscreen_quad(tools.rc_mgr.fullscreen_quad->subsets.GetFirst())
                                                     , color_only_pass(in_color_only_pass)
                                                     , gradient_dithering_shader(LoadGradientDitheringShader())
                                                     , gradient_dithering_pipeline(CreateGradientDitheringPipelineState())
                                                     , lookup(GenLookupTable())
{}

// Creates dithering (per-view) data.
//    @return - a new Dithering::ViewData object or error if failed.
ut::Result<Dithering::ViewData, ut::Error> Dithering::CreateViewData()
{
	// create uniform buffer
	Buffer::Info buffer_info;
	buffer_info.type = Buffer::Type::uniform;
	buffer_info.usage = render::memory::Usage::gpu_read_cpu_write;
	buffer_info.size = sizeof(ViewData::GradientDitheringUB);
	ut::Result<Buffer, ut::Error> gradient_dithering_buffer = tools.device.CreateBuffer(ut::Move(buffer_info));
	if (!gradient_dithering_buffer)
	{
		throw ut::Error(gradient_dithering_buffer.MoveAlt());
	}

	// create final data object
	ViewData data(gradient_dithering_buffer.Move());

	// connect descriptors
	data.gradient_dithering_desc_set.Connect(gradient_dithering_shader);

	// success
	return data;
}

// Performs gradient dithering, makes smooth color transition for LDR targets.
//    @param swap_mgr - reference to the post-process swap manager.
//    @param context - reference to the rendering context.
//    @param data - reference to the Dithering::ViewData object.
//    @param source - reference to the source image.
// 	  @param parameters - reference to the Dithering::Parameters object
//                        containing parameters for the dithering effect.
//    @param time_ms - total accumulated time in milliseconds.
//    @return - optional reference to the postprocess slot used for dithering.
ut::Optional<SwapSlot&> Dithering::ApplyGradientDithering(SwapManager& swap_mgr,
                                                          Context& context,
                                                          ViewData& data,
                                                          Image& source,
                                                          const Parameters& parameters,
                                                          float time_ms)
{
	if (!parameters.gradient_dithering_enabled)
	{
		return ut::Optional<SwapSlot&>();
	}

	// get post-process render target
	ut::Optional<SwapSlot&> slot = swap_mgr.Swap();
	UT_ASSERT(slot.HasValue());
	const Framebuffer::Info& fb_info = slot->color_only_framebuffer.GetInfo();

	// get source and lookup image info
	const Image::Info& src_img_info = source.GetInfo();
	const Image::Info& lookup_info = lookup->GetInfo();

	// adjust dithering radius according to the image resolution
	const ut::uint32 src_size = ut::Min(src_img_info.width, src_img_info.height);
	ut::uint32 lookup_size = lookup_info.width / 2;
	ut::uint32 dithering_mip_id = 1;
	if (src_size <= 2160)
	{
		lookup_size /= 2;
		dithering_mip_id += 1;
	}
	else if (src_size <= 1080)
	{
		lookup_size /= 4;
		dithering_mip_id += 2;
	}
	else if (src_size <= 640)
	{
		lookup_size /= 16;
		dithering_mip_id += 3;
	}
	else if (src_size <= 320)
	{
		lookup_size /= 32;
		dithering_mip_id += 4;
	}

	// update uniform buffer
	ViewData::GradientDitheringUB gradient_dithering_ub;
	gradient_dithering_ub.parameters.X() = static_cast<float>(src_img_info.width);
	gradient_dithering_ub.parameters.Y() = static_cast<float>(src_img_info.height);
	gradient_dithering_ub.parameters.Z() = static_cast<float>(lookup_size);
	gradient_dithering_ub.parameters.W() = static_cast<float>(dithering_mip_id);
	ut::Optional<ut::Error> update_ub_error = tools.rc_mgr.UpdateBuffer(context,
	                                                                    data.gradient_dithering_buffer,
	                                                                    &gradient_dithering_ub);
	if (update_ub_error)
	{
		throw update_ub_error.Move();
	}

	// set shader resources
	data.gradient_dithering_desc_set.ub.BindUniformBuffer(data.gradient_dithering_buffer);
	data.gradient_dithering_desc_set.tex2d.BindImage(source);
	data.gradient_dithering_desc_set.tex2d_lut.BindImage(lookup.Get());
	data.gradient_dithering_desc_set.sampler.BindSampler(tools.sampler_cache.linear_clamp);
	data.gradient_dithering_desc_set.sampler_lut.BindSampler(tools.sampler_cache.point_wrap);

	// draw quad
	ut::Rect<ut::uint32> render_area(0, 0, fb_info.width, fb_info.height);
	context.BeginRenderPass(color_only_pass,
	                        slot->color_only_framebuffer,
	                        render_area,
	                        ut::Color<4>(0), 1.0f);
	context.BindPipelineState(gradient_dithering_pipeline);
	context.BindDescriptorSet(data.gradient_dithering_desc_set);
	context.BindVertexBuffer(tools.rc_mgr.fullscreen_quad->subsets.GetFirst().vertex_buffer.GetRef(), 0);
	context.Draw(6, 0);
	context.EndRenderPass();

	return slot;
}

// Returns compiled pixel shader performing the gradient dithering effect.
Shader Dithering::LoadGradientDitheringShader()
{
	ut::Result<Shader, ut::Error> shader = tools.shader_loader.Load(Shader::Stage::pixel,
	                                                                "gradient_dithering_ps",
	                                                                "PS",
	                                                                "gradient_dithering.hlsl");
	return shader.MoveOrThrow();
}

// Creates a pipeline state for the gradient dithering effect.
PipelineState Dithering::CreateGradientDitheringPipelineState()
{
	PipelineState::Info info;
	info.SetShader(Shader::Stage::vertex, tools.quad.vs);
	info.SetShader(Shader::Stage::pixel, gradient_dithering_shader);
	info.input_assembly_state = fullscreen_quad.CreateIaState();
	info.depth_stencil_state.depth_test_enable = false;
	info.depth_stencil_state.depth_write_enable = false;
	info.depth_stencil_state.depth_compare_op = compare::Operation::never;
	info.rasterization_state.polygon_mode = RasterizationState::PolygonMode::fill;
	info.rasterization_state.cull_mode = RasterizationState::CullMode::off;
	info.blend_state.attachments.Add(BlendState::CreateNoBlending());
	return tools.device.CreatePipelineState(ut::Move(info), color_only_pass).MoveOrThrow();
}

// Generates dithering lookup table image.
RcRef<Map> Dithering::GenLookupTable()
{	
	constexpr ut::uint32 size = 128;

	tools.random.Seed(0);

	ResourceCreator<Map>::InitInfo init_info;
	init_info.img_type = Image::Type::planar;
	init_info.format = pixel::Format::r16g16_unorm;
	init_info.width = size;
	init_info.height = size;
	init_info.depth = 1;
	init_info.mip_count = 6;
	init_info.generate_mips = false;
	init_info.name = "dithering_lut";

	// calculate total number of pixels
	ut::uint32 pixel_count = 0;
	ut::uint32 mip_size = size;
	for (ut::uint32 mip_id = 0; mip_id < init_info.mip_count; mip_id++)
	{
		pixel_count += mip_size * mip_size;
		mip_size /= 2;
	}

	// use map creator to conveniently create blue-noise texture
	ResourceCreator<Map>& map_creator = tools.rc_mgr.GetCreator<Map>();

	// load the cache
	Cache cache;
	ut::Optional<ut::Error> load_cache_error = cache.Load(pixel_count);
	if (!load_cache_error)
	{
		return map_creator.CreateFromData<2, ut::uint16>(init_info, cache.lookup).MoveOrThrow();
	}

	// calculates a distance between two [0,1] points,
	// this method takes into account wrapping
	static auto calc_distance = [](const ut::Vector<2, float>& p0,
	                               const ut::Vector<2, float>& p1)
	{
		ut::Vector<2, float> offset = p1 - p0;

		if (offset.X() > 0.5f)
			offset.X() = 1.0f - offset.X();

		if (offset.Y() > 0.5f)
			offset.Y() = 1.0f - offset.Y();

		return offset.Length();
	};

	// calculates an 'energy' sum of the 9x9 pixel area, then the central pixel
	// can be modified and an energy difference will show how the distribution
	// of color has changed
	static auto energy = [&](const ut::Array<ut::Color<2, float>>& mip_data,
	                        const ut::Vector<2, int>& a, int mip_size)
	{
		int ia = a.Y() * mip_size + a.X();

		float total_energy = 0.0f;
		for (int dy = -4; dy <= 4; dy++)
		{
			for (int dx = -4; dx <= 4; dx++)
			{
				if (dx == 0 && dy == 0)
				{
					continue;
				}

				const ut::Vector<2, int> b((a.X() + dx + mip_size) % mip_size,
				                           (a.Y() + dy + mip_size) % mip_size);
				const int ib = b.Y() * mip_size + b.X();

				const ut::Color<2, float> pia = mip_data[ia];
				const ut::Color<2, float> pib = mip_data[ib];

				float length2 = static_cast<float>(dx * dx + dy * dy);
				total_energy += ut::Exp(-length2 / (2.1f * 2.1f)
				                        -calc_distance(pia, pib));
			}
		}

		return total_energy;
	};

	// converts white noise into blue noise
	auto filter_blue_noise = [&](ut::Array<ut::Color<2, float>>& mip_data,
	                            int iteration_count, int mip_size)
	{
		for (int i = 0; i < iteration_count; i++)
		{
			// select two random pixels
			ut::Vector<2, ut::int32> a(tools.random() % mip_size, tools.random() % mip_size);
			ut::Vector<2, ut::int32> b(tools.random() % mip_size, tools.random() % mip_size);
			if (a == b)
			{
				continue;
			}

			// find pixel indices in buffer.
			const int ia = a.Y() * mip_size + a.X();
			const int ib = b.Y() * mip_size + b.X();

			// find energy before and after swapping pixels
			const float start_energy = energy(mip_data, a, mip_size) +
			                           energy(mip_data, b, mip_size);
			ut::Vector<2, float> tmp = mip_data[ia];
			mip_data[ia] = mip_data[ib];
			mip_data[ib] = tmp;
			float endEnergy = energy(mip_data, a, mip_size) +
			                  energy(mip_data, b, mip_size);

			// if the energy was lower before the swap, then swap back
			if (start_energy < endEnergy)
			{
				tmp = mip_data[ia];
				mip_data[ia] = mip_data[ib];
				mip_data[ib] = tmp;
			}
		}
	};

	// initialize mips
	ut::uint32 mip_offset = 0;
	mip_size = size;
	cache.lookup.Resize(pixel_count);
	for (ut::uint32 mip_id = 0; mip_id < init_info.mip_count; mip_id++)
	{
		ut::uint32 mip_pixel_count = mip_size * mip_size;

		// first initialize all pixel indices sequentially from the first to the last
		ut::Array<ut::uint32> pixel_indices_ordered(mip_pixel_count);
		for (ut::uint32 pixel_id = 0; pixel_id < pixel_indices_ordered.Count(); pixel_id++)
		{
			pixel_indices_ordered[pixel_id] = pixel_id;
		}

		// then shuffle pixel indices and initialize pixels with [0,1) data,
		// each pixel must be different and the entire color space must be covered
		ut::Array<ut::Color<2, float>> normalized_data(mip_pixel_count);
		for (ut::uint32 pixel_id = 0; pixel_id < mip_pixel_count; pixel_id++)
		{
			const ut::uint32 unordered_id = tools.random() % pixel_indices_ordered.Count();
			const ut::uint32 ordered_id = pixel_indices_ordered[unordered_id];

			const ut::uint32 x = ordered_id % mip_size;
			const ut::uint32 y = ordered_id / mip_size;

			const float value_x = static_cast<float>(x) / mip_size;
			const float value_y = static_cast<float>(y) / mip_size;

			normalized_data[pixel_id] = ut::Color<2, float>(value_x, value_y);

			pixel_indices_ordered.Remove(unordered_id);
		}

		// filter white noise -> blue noise
		const ut::uint32 num_iterations = ut::Pow(12u, init_info.mip_count - mip_id);
		filter_blue_noise(normalized_data, num_iterations, mip_size);

		// encode the final result into unorm_16 values
		for (ut::uint32 pixel_id = 0; pixel_id < mip_pixel_count; pixel_id++)
		{
			cache.lookup[mip_offset + pixel_id].X() = static_cast<ut::uint16>(normalized_data[pixel_id].X() * 65535.0f);
			cache.lookup[mip_offset + pixel_id].Y() = static_cast<ut::uint16>(normalized_data[pixel_id].Y() * 65535.0f);
		}

		// go to the next mip
		mip_offset += mip_pixel_count;
		mip_size /= 2;
	}	
	
	// save the cache and create gpu data
	cache.Save();
	return map_creator.CreateFromData<2, ut::uint16>(init_info, cache.lookup).MoveOrThrow();
}

// Registers @lookup data into reflection tree.
//    @param snapshot - reference to the reflection tree
void Dithering::Cache::Reflect(ut::meta::Snapshot& snapshot)
{
	snapshot.Add(ut::meta::Binary(lookup, sizeof(ut::uint16)), "lookup");
}

// Loads @lookup data from the @skCacheFileName file.
ut::Optional<ut::Error> Dithering::Cache::Load(size_t expected_lookup_size)
{
	// capture meta snapshot
	ut::meta::Snapshot snapshot = ut::meta::Snapshot::Capture(*this, skMetaName,
	                                                          ut::meta::Info::CreatePure());

	// open file for reading
	ut::File file;
	ut::Optional<ut::Error> open_error = file.Open(path, ut::File::Access::read);
	if (open_error)
	{
		ut::log.Lock() << "Dithering cache: failed to open file " << path << ut::cret;
		return open_error;
	}

	// load from file
	ut::Optional<ut::Error> load_error = snapshot.Load(file);
	if (load_error)
	{
		ut::log.Lock() << "Dithering cache: failed to load from cache, error = "
		               << load_error->GetCode() << ut::cret;
		return load_error;
	}

	// check lookup table size
	if (lookup.Count() != expected_lookup_size)
	{
		ut::log.Lock() << "Dithering cache: invalid lookup size ("
		               << lookup.Count() << " bytes)" << ut::cret;
		return ut::Error(ut::error::out_of_bounds);
	}

	return ut::Optional<ut::Error>();
}

// Saves @lookup data to the @skCacheFileName file.
ut::Optional<ut::Error> Dithering::Cache::Save()
{
	// capture meta snapshot
	ut::meta::Snapshot snapshot = ut::meta::Snapshot::Capture(*this, skMetaName,
	                                                          ut::meta::Info::CreatePure());

	// create directory if it doesn't exist
	ut::CreateDirectories(path.GetIsolatedLocation(false));

	// open file for writing
	ut::File file;
	ut::Optional<ut::Error> open_error = file.Open(path, ut::File::Access::write);
	if (open_error)
	{
		ut::log.Lock() << "Dithering cache: failed to save file " << path << ut::cret;
		return open_error;
	}

	// write the snapshot to the file
	return snapshot.Save(file);
}

//----------------------------------------------------------------------------//
END_NAMESPACE(postprocess)
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//