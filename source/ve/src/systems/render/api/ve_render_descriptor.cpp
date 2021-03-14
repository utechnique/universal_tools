//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "systems/render/api/ve_render_descriptor.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// Constructor, accepts a name of the shader resource.
Descriptor::Descriptor(const char* binding_name) : name(binding_name)
{}

// Move constructor.
Descriptor::Descriptor(Descriptor&&) noexcept = default;

// Move operator.
Descriptor& Descriptor::operator =(Descriptor&&) noexcept = default;

// Binding constructor.
Descriptor::Binding::Binding(ut::uint32 in_id,
                             Shader::Parameter::Type in_type) : id(in_id)
                                                              , type(in_type)
{}

// Initializes binding index for the provided shader stage.
void Descriptor::Connect(Shader& shader)
{
	const Shader::Info& info = shader.GetInfo();
	const size_t parameter_count = info.parameters.GetNum();
	for (size_t i = 0; i < parameter_count; i++)
	{
		const Shader::Parameter& parameter = info.parameters[i];
		if (parameter.GetName() != name)
		{
			continue;
		}

		if (binding && binding->id != parameter.GetBinding())
		{
			ut::log.Lock() << "Warning! Shader variable descriptor \"" << name <<
			                  "\" is reconnected with different id: " << parameter.GetBinding() <<
			                  " (was " << binding->id << ")" << ut::cret;
		}

		binding = Binding(parameter.GetBinding(), parameter.GetType());
	}
}

// Binds provided uniform buffer to this descriptor.
void Descriptor::BindUniformBuffer(Buffer& uniform_buffer, ut::uint32 array_id)
{
	if (!binding)
	{
		ut::log.Lock() << "Warning! Attempt to bind unconnected descriptor " << 
		                  "as an uniform buffer: \"" << name << "\"" << ut::cret;
		return;
	}

	if (uniform_buffer.GetInfo().type != Buffer::uniform)
	{
		ut::log.Lock() << "Warning! Attempt to bind non-uniform buffer " << 
		                  "as an uniform buffer: \"" << name << "\"" << ut::cret;
	}

	AssignSlot(&uniform_buffer, array_id);
}

// Binds provided image to this descriptor.
void Descriptor::BindImage(Image& image, ut::uint32 array_id)
{
	if (!binding)
	{
		ut::log.Lock() << "Warning! Attempt to bind unconnected descriptor " << 
		                  "as an image: \"" << name << "\"" << ut::cret;
		return;
	}

	AssignSlot(&image, array_id);
}

// Binds provided cube face to this descriptor.
void Descriptor::BindCubeFace(Image& cube_image, Image::Cube::Face face, ut::uint32 array_id)
{
	if (cube_image.GetInfo().type != Image::type_cube)
	{
		ut::log.Lock() << "Warning! Attempt to bind non-cubic image " <<
		                  "as a cube face image: \"" << name << "\"" << ut::cret;
		return;
	}

	if (!binding)
	{
		ut::log.Lock() << "Warning! Attempt to bind unconnected descriptor " << 
		                  "as a cube face image: \"" << name << "\"" << ut::cret;
		return;
	}

	AssignSlot(&cube_image, array_id);
	binding->slots[array_id]->cube_face = face;
}

// Binds provided sampler to this descriptor.
void Descriptor::BindSampler(Sampler& sampler, ut::uint32 array_id)
{
	if (!binding)
	{
		ut::log.Lock() << "Warning! Attempt to bind unconnected descriptor " <<
		                  "as a sampler: \"" << name << "\"" << ut::cret;
		return;
	}

	AssignSlot(&sampler, array_id);
}

// Returns current binding.
const ut::Optional<Descriptor::Binding>& Descriptor::GetBinding() const
{
	return binding;
}

// Connects shader resource that is specified by the provided pointer to
// the appropriate slot.
void Descriptor::AssignSlot(void* rc_ptr, ut::uint32 array_id)
{
	const ut::uint32 min_array_size = array_id + 1;
	if (binding->slots.GetNum() < min_array_size)
	{
		binding->slots.Resize(min_array_size);
	}
	ut::Optional<Slot>& slot = binding->slots[array_id];
	slot = Slot();
	slot->array_id = array_id;
	slot->uniform_buffer = static_cast<Buffer*>(rc_ptr);
}

//----------------------------------------------------------------------------//
// Move constructor.
DescriptorSet::DescriptorSet(DescriptorSet&&) noexcept = default;

// Move operator.
DescriptorSet& DescriptorSet::operator =(DescriptorSet&&) noexcept = default;

// Initializes binding indices for the provided shader stage.
void DescriptorSet::Connect(Shader& shader)
{
	const size_t descriptor_count = descriptors.GetNum();
	for (size_t i = 0; i < descriptor_count; i++)
	{
		GetDescriptor(i).Connect(shader);
	}
	UpdateResourceCount();
	Validate();
}

// Initializes binding indices for the provided shader stages.
void DescriptorSet::Connect(BoundShader& bound_shader)
{
	for (size_t i = 0; i < Shader::skStageCount; i++)
	{
		if (bound_shader.stages[i])
		{
			Connect(bound_shader.stages[i].Get());
		}
	}
}

// Returns a reference to the recource counter.
const DescriptorSet::ResourceCount& DescriptorSet::GetResourceCount()
{
	return resource_count;
}

// Returns a number of descriptors in this set.
size_t DescriptorSet::GetDescriptorCount() const
{
	return descriptors.GetNum();
}

// Returns a total number of slots in this set.
size_t DescriptorSet::GetSlotCount() const
{
	size_t slot_count = 0;
	const size_t descriptor_count = descriptors.GetNum();
	for (size_t i = 0; i < descriptor_count; i++)
	{
		const Descriptor& descriptor = GetDescriptor(i);
		const ut::Optional<Descriptor::Binding>& binding = descriptor.GetBinding();
		if (binding)
		{
			slot_count += binding->slots.GetNum();
		}
	}
	return slot_count;
}

// Returns a constant reference to the descriptor.
const Descriptor& DescriptorSet::GetDescriptor(size_t id) const
{
	const ut::byte* descriptor_address = reinterpret_cast<const ut::byte*>(this) + descriptors[id];
	return *reinterpret_cast<const Descriptor*>(descriptor_address);
}

// Returns a reference to the descriptor.
Descriptor& DescriptorSet::GetDescriptor(size_t id)
{
	ut::byte* descriptor_address = reinterpret_cast<ut::byte*>(this) + descriptors[id];
	return *reinterpret_cast<Descriptor*>(descriptor_address);
}

// Updates @resources_count.
void DescriptorSet::UpdateResourceCount()
{
	resource_count.uniform_buffers = 0;
	resource_count.images = 0;
	resource_count.samplers = 0;
	resource_count.storage_buffers = 0;

	const size_t descriptor_count = descriptors.GetNum();
	for (size_t i = 0; i < descriptor_count; i++)
	{
		ut::Optional<Descriptor::Binding> binding = GetDescriptor(i).GetBinding();

		if (!binding)
		{
			continue;
		}

		switch (binding->type)
		{
		case Shader::Parameter::uniform_buffer: resource_count.uniform_buffers++; break;
		case Shader::Parameter::image:          resource_count.images++;          break;
		case Shader::Parameter::sampler:        resource_count.samplers++;        break;
		case Shader::Parameter::storage_buffer: resource_count.storage_buffers++; break;
		}
	}
}

// Checks if descriptors have valid values.
void DescriptorSet::Validate()
{
	// every parameter must be unique
	const size_t descriptor_count = descriptors.GetNum();
	for (size_t i = 0; i < descriptor_count; i++)
	{
		for (size_t j = i + 1; j < descriptor_count; j++)
		{
			Descriptor& d1 = GetDescriptor(i);
			Descriptor& d2 = GetDescriptor(j);

			// check names
			if (ut::StrCmp(d1.name, d2.name))
			{
				ut::log.Lock() << "Warning! Shader variable descriptors share " <<
				                  "the same name: \"" << d1.name << "\"" << ut::cret;
			}

			const ut::Optional<Descriptor::Binding>& b1 = d1.GetBinding();
			const ut::Optional<Descriptor::Binding>& b2 = d2.GetBinding();

			if (!b1 || !b2)
			{
				continue;
			}

			// check id
			if (b1->id == b2->id && b1->type == b2->type)
			{
				ut::log.Lock() << "Warning! Shader variable descriptors \"" << d1.name <<
				                  " and \"" << d2.name << "\" share the same id: " <<
				                  b1->id  << ut::cret;
			}
		}
	}
}

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//