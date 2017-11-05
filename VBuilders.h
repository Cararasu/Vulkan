#ifndef VFACTORIES_H
#define VFACTORIES_H

#include "VHeader.h"
#include <vector>
#include "VGlobal.h"

class ShaderBuilder {
	VkShaderModuleCreateInfo createInfo;
public:
	ShaderBuilder() : createInfo() {
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.pNext = nullptr;
		createInfo.flags = 0;
	}

	ShaderBuilder& setShaderCode (std::vector<char>& data) {
		createInfo.codeSize = data.size();
		createInfo.pCode = reinterpret_cast<const uint32_t*> (data.data());
		return *this;
	}
	ShaderBuilder& setShaderCode (const char* data, size_t size) {
		createInfo.codeSize = size;
		createInfo.pCode = reinterpret_cast<const uint32_t*> (data);
		return *this;
	}

	VkShaderModule build () {
		VkShaderModule shadermodule;
		VCHECKCALL (vkCreateShaderModule (vGlobal.deviceWrapper.device, &createInfo, nullptr, &shadermodule), {
			printf ("Creation of Shadermodule failed\n");
			return VK_NULL_HANDLE;
		});
		return shadermodule;
	}
};

struct DescriptorSetLayoutBuilder {
	VkDescriptorSetLayoutCreateInfo info;

	std::vector<VkDescriptorSetLayoutBinding> bindings;

	DescriptorSetLayoutBuilder() {
		info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		info.pNext = nullptr;
		info.flags = 0;
	}

	DescriptorSetLayoutBuilder& reset(){
		bindings.clear();
		return *this;
	}
	
	DescriptorSetLayoutBuilder& pushBinding(uint32_t binding, VkDescriptorType descriptorType, uint32_t descriptorCount, VkShaderStageFlags stageFlags, const VkSampler* pImmutableSamplers = nullptr) {
		VkDescriptorSetLayoutBinding bind;
		bind.binding = binding;
		bind.descriptorType = descriptorType;
		bind.descriptorCount = descriptorCount;
		bind.stageFlags = stageFlags;
		bind.pImmutableSamplers = pImmutableSamplers;
		bindings.push_back(bind);
		return *this;
	}

	VkDescriptorSetLayout build(VkDevice device) {
		VkDescriptorSetLayout layout;
		info.bindingCount = bindings.size();
		info.pBindings = bindings.data();

		vkCreateDescriptorSetLayout(device, &info, nullptr, &layout);
		return layout;
	}
};
struct PipelineLayoutBuilder {
	VkPipelineLayoutCreateInfo info;

	std::vector<VkDescriptorSetLayout> setLayouts;
	std::vector<VkPushConstantRange> pushConstantRange;

	PipelineLayoutBuilder() {
		info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		info.pNext = nullptr;
		info.flags = 0;
	}

	PipelineLayoutBuilder& reset(){
		setLayouts.clear();
		pushConstantRange.clear();
		return *this;
	}
	
	PipelineLayoutBuilder& pushBinding(VkDescriptorSetLayout descriptorSetLayout) {
		setLayouts.push_back(descriptorSetLayout);
		return *this;
	}
	PipelineLayoutBuilder& pushPushConstant(VkPushConstantRange pushConstRange) {
		pushConstantRange.push_back(pushConstRange);
		return *this;
	}

	VkPipelineLayout build(VkDevice device) {
		VkPipelineLayout layout;
		info.setLayoutCount = setLayouts.size();
		info.pSetLayouts = setLayouts.data();
		info.pushConstantRangeCount = pushConstantRange.size();
		info.pPushConstantRanges = pushConstantRange.data();

		VCHECKCALL(vkCreatePipelineLayout(device, &info, nullptr, &layout), printf("Creation of PipelineLayout failed\n"));
		return layout;
	}
};


#endif // VFACTORIES_H
