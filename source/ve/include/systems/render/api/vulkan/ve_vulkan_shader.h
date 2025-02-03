//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#pragma once
//----------------------------------------------------------------------------//
#if VE_VULKAN
//----------------------------------------------------------------------------//
#include "systems/render/api/vulkan/ve_vulkan_resource.h"
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// Vulkan shader.
class PlatformShader : public VkRc<vk::Rc::shader_module>
{
	friend class Device;
	friend class Context;
public:
	// Constructor.
	PlatformShader(VkDevice device_handle,
	               VkShaderModule shader_module_handle,
	               const VkPipelineShaderStageCreateInfo& shader_stage_info);

	// Move constructor.
	PlatformShader(PlatformShader&&) noexcept;

	// Move operator.
	PlatformShader& operator =(PlatformShader&&) noexcept;

	// Copying is prohibited.
	PlatformShader(const PlatformShader&) = delete;
	PlatformShader& operator =(const PlatformShader&) = delete;

	// Converts provided shader type (see ve::Shader::Type) to appropriate
	// VkShaderStageFlagBits set of flags.
	//    @param shader_type - ve::Shader::Type value.
	//    @return - VkShaderStageFlagBits object.
	static VkShaderStageFlagBits ConvertTypeToVkStage(ut::uint32 shader_type);

private:
	VkPipelineShaderStageCreateInfo stage_info;

	// VkGraphicsPipelineCreateInfo needs a pointer to the null terminated
	// string that must remain the same.
	ut::UniquePtr<ut::String> entry_point_ptr;
};

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif // VE_VULKAN
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//