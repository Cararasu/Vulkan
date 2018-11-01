#pragma once

#include <vulkan/vulkan.hpp>
#include <render/Resources.h>

struct VShaderModule : public ShaderModule {
	ShaderType type;
	String name;
	vk::ShaderModule shadermodule;
	
	VShaderModule(ShaderType type, String name, vk::ShaderModule shadermodule) : type(type), name(name), shadermodule(shadermodule) { }
};