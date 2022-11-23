//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#include "systems/render/api/ve_render_platform.h"
#include "systems/render/api/ve_render_shader.h"
#include "systems/render/api/ve_render_buffer.h"
#include "systems/render/api/ve_render_image.h"
#include "systems/render/api/ve_render_sampler.h"
#include "templates/ut_each_is.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// ve::render::Descriptor represents a shader resource binding by name.
class Descriptor
{
public:
	// Represents an element in array of shader resources.
	struct Slot
	{
		// Id of the cube face in case if bound resource is an image,
		// and if this image is a cubemap.
		ut::Optional<Image::Cube::Face> cube_face;

		// Array element id (if bound resource is an array).
		ut::uint32 array_id;

		// Union of resources that can be bound to the pipeline.
		union
		{
			Buffer* uniform_buffer;
			Image* image;
			Sampler* sampler;
		};
	};

	// Contains an id of the shader resource and it's type.
	struct Binding
	{
		// Constructor, type and id must be provided.
		Binding(ut::uint32 in_id, Shader::Parameter::Type in_type);

		// Id of the resource how it's defined in a shader code.
		ut::uint32 id;

		// Type of resource how it's defined in a shader code.
		Shader::Parameter::Type type;

		// Resources to be bound to the pipeline.
		ut::Array< ut::Optional<Slot> > slots;
	};

	// Constructor, accepts a name of the shader resource.
	Descriptor(const char* binding_name);

	// Move constructor.
	Descriptor(Descriptor&&) noexcept;

	// Move operator.
	Descriptor& operator =(Descriptor&&) noexcept;

	// Copying is prohibited.
	Descriptor(const Descriptor&) = delete;
	Descriptor& operator =(const Descriptor&) = delete;

	// Name of the shader resource.
	const char* name;

	// Initializes binding index for the provided shader stage.
	void Connect(Shader& shader);

	// Binds provided uniform buffer to this descriptor.
	void BindUniformBuffer(Buffer& uniform_buffer, ut::uint32 array_id = 0);

	// Binds provided image to this descriptor.
	void BindImage(Image& image, ut::uint32 array_id = 0);

	// Binds provided cube face to this descriptor.
	void BindCubeFace(Image& cube_image, Image::Cube::Face face, ut::uint32 array_id = 0);

	// Binds provided sampler to this descriptor.
	void BindSampler(Sampler& sampler, ut::uint32 array_id = 0);

	// Returns current binding.
	const ut::Optional<Binding>& GetBinding() const;

private:
	// Connects shader resource that is specified by the provided pointer to
	// the appropriate slot.
	void AssignSlot(void* rc_ptr, ut::uint32 array_id);

	// Binding index (must be the same for all shader stages).
	ut::Optional<Binding> binding;
};

//----------------------------------------------------------------------------//
// ve::render::DescriptorSet represents a set of descriptors bound to the
// rendering pipeline.
class DescriptorSet
{
	friend class Device;
	friend class Context;
	friend class PipelineState;
public:
	// Represents a counter of resources connected to this set.
	struct ResourceCount
	{
		ut::uint32 uniform_buffers = 0;
		ut::uint32 images = 0;
		ut::uint32 samplers = 0;
		ut::uint32 storage_buffers = 0;
	};

	// Constructor, accepts variable number of descriptor references.
	template<typename... Elements, typename Sfinae = typename ut::EnableIf<ut::EachIs<Descriptor, Elements...>::value>::Type>
	DescriptorSet(Elements&... elements)
	{
		ut::byte* start = reinterpret_cast<ut::byte*>(this);
		Descriptor* args[]{ &elements... };
		const size_t n = sizeof(args) / sizeof(Descriptor*);
		for (size_t i = 0; i < n; i++)
		{
			ut::byte* descriptor_address = reinterpret_cast<ut::byte*>(args[i]);
			descriptors.Add(static_cast<ut::uint32>(descriptor_address - start));
		}
	}

	// Move constructor.
	DescriptorSet(DescriptorSet&&) noexcept;

	// Move operator.
	DescriptorSet& operator =(DescriptorSet&&) noexcept;

	// Copying is prohibited.
	DescriptorSet(const DescriptorSet&) = delete;
	DescriptorSet& operator =(const DescriptorSet&) = delete;

	// Initializes binding indices for the provided shader stage.
	void Connect(Shader& shader);

	// Initializes binding indices for the provided shader stages.
	void Connect(BoundShader& bound_shader);

	// Returns a reference to the recource counter.
	const ResourceCount& GetResourceCount();

	// Returns a number of descriptors in this set.
	size_t GetDescriptorCount() const;

	// Returns a total number of slots in this set.
	size_t GetSlotCount() const;

	// Returns a constant reference to the descriptor.
	const Descriptor& GetDescriptor(size_t id) const;

	// Returns a reference to the descriptor.
	Descriptor& GetDescriptor(size_t id);

private:
	// Updates @resources_count.
	void UpdateResourceCount();

	// Checks if descriptors have valid values.
	void Validate();

	// each descriptor here is an offset from the DescriptorSet class in bytes.
	ut::Array<ut::uint32> descriptors;

	// counts bound resources
	ResourceCount resource_count;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//