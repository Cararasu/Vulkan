
#include "VHeader.h"
#include "VGlobal.h"



std::vector<vk::DescriptorSetLayout> createStandardDescriptorSetLayouts(){
	
	vk::DescriptorSetLayoutBinding bindings1[] = {
		vk::DescriptorSetLayoutBinding(0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex, nullptr),
	};
	vk::DescriptorSetLayoutBinding bindings2[] = {
		vk::DescriptorSetLayoutBinding(0, vk::DescriptorType::eSampler, 1, vk::ShaderStageFlagBits::eFragment, nullptr),
		vk::DescriptorSetLayoutBinding(1, vk::DescriptorType::eSampledImage, 3, vk::ShaderStageFlagBits::eFragment, nullptr)
	};
	
	std::vector<vk::DescriptorSetLayout> descriptorSetLayouts = {
		global.deviceWrapper.device.createDescriptorSetLayout(vk::DescriptorSetLayoutCreateInfo(vk::DescriptorSetLayoutCreateFlags(), 1, bindings1), nullptr),
		global.deviceWrapper.device.createDescriptorSetLayout(vk::DescriptorSetLayoutCreateInfo(vk::DescriptorSetLayoutCreateFlags(), 2, bindings2), nullptr)
	};
	return descriptorSetLayouts;
}
void destroyDescriptorSetLayout(vk::DescriptorSetLayout layout){
	global.deviceWrapper.device.destroyDescriptorSetLayout(layout);
}
vk::DescriptorPool createStandardDescriptorSetPool(){
	
	std::array<vk::DescriptorPoolSize, 3> poolSize = {
		vk::DescriptorPoolSize(vk::DescriptorType::eUniformBuffer, 10),
		vk::DescriptorPoolSize(vk::DescriptorType::eSampledImage, 10),
		vk::DescriptorPoolSize(vk::DescriptorType::eSampler, 10)
	};
	
	vk::DescriptorPoolCreateInfo createInfo(vk::DescriptorPoolCreateFlags(), 10, poolSize.size(), poolSize.data());

	
	vk::DescriptorPool descriptorPool;
	V_CHECKCALL(global.deviceWrapper.device.createDescriptorPool(&createInfo, nullptr, &descriptorPool), printf("Creation of DescriptorSetPool failed\n"));
	return descriptorPool;
}
std::vector<vk::DescriptorSet> createDescriptorSets(vk::DescriptorPool descriptorSetPool, std::vector<vk::DescriptorSetLayout>* descriptorSetLayouts){
	
	return global.deviceWrapper.device.allocateDescriptorSets(vk::DescriptorSetAllocateInfo(descriptorSetPool, descriptorSetLayouts->size(), descriptorSetLayouts->data()));
}