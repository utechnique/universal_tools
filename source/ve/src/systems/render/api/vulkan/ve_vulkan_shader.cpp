//----------------------------------------------------------------------------//
//---------------------------------|  V  E  |---------------------------------//
//----------------------------------------------------------------------------//
#include "systems/render/api/ve_render_shader.h"
//----------------------------------------------------------------------------//
#if VE_VULKAN
//----------------------------------------------------------------------------//
START_NAMESPACE(ve)
START_NAMESPACE(render)
//----------------------------------------------------------------------------//
// Constructor.
PlatformShader::PlatformShader(VkDevice device_handle,
                               VkShaderModule shader_module_handle,
                               const VkPipelineShaderStageCreateInfo& shader_stage_info) : VkRc<vk::shader_module>(shader_module_handle,
                                                                                                                   device_handle)
                                                                                         , stage_info(shader_stage_info)
                                                                                         , entry_point_ptr(ut::MakeUnique<ut::String>(shader_stage_info.pName))
{
	stage_info.pName = entry_point_ptr->GetAddress();
}

// Move constructor.
PlatformShader::PlatformShader(PlatformShader&&) noexcept = default;

// Move operator.
PlatformShader& PlatformShader::operator =(PlatformShader&&) noexcept = default;

// Converts provided shader type (see ve::Shader::Type) to appropriate
// VkShaderStageFlagBits set of flags.
//    @param shader_type - ve::Shader::Type value.
//    @return - VkShaderStageFlagBits object.
VkShaderStageFlagBits PlatformShader::ConvertTypeToVkStage(ut::uint32 shader_type)
{
	switch (shader_type)
	{
	case Shader::vertex: return VK_SHADER_STAGE_VERTEX_BIT;
	case Shader::hull: return VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
	case Shader::domain: return VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
	case Shader::geometry: return VK_SHADER_STAGE_GEOMETRY_BIT;
	case Shader::pixel: return VK_SHADER_STAGE_FRAGMENT_BIT;
	case Shader::compute: return VK_SHADER_STAGE_COMPUTE_BIT;
	}
	return VK_SHADER_STAGE_ALL;
}

//----------------------------------------------------------------------------//
END_NAMESPACE(render)
END_NAMESPACE(ve)
//----------------------------------------------------------------------------//
#endif // VE_VULKAN
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//
//----------------------------------------------------------------------------//