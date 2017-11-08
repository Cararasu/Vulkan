
#include "VHeader.h"
#include "VGlobal.h"



std::vector<vk::DescriptorSetLayout> createStandardDescriptorSetLayouts(){
	
	vk::DescriptorSetLayoutBinding bindings[] = {
		vk::DescriptorSetLayoutBinding(0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex, nullptr)
	};
	
	std::vector<vk::DescriptorSetLayout> descriptorSetLayouts = {
		global.deviceWrapper.device.createDescriptorSetLayout(vk::DescriptorSetLayoutCreateInfo(vk::DescriptorSetLayoutCreateFlags(), 1, bindings), nullptr)
	};
	return descriptorSetLayouts;
}
void destroyDescriptorSetLayout(vk::DescriptorSetLayout layout){
	global.deviceWrapper.device.destroyDescriptorSetLayout(layout);
}
vk::DescriptorPool createStandardDescriptorSetPool(){
	
	std::array<vk::DescriptorPoolSize, 1> poolSize = {vk::DescriptorPoolSize(vk::DescriptorType::eUniformBuffer, 10)};
	
	vk::DescriptorPoolCreateInfo createInfo(vk::DescriptorPoolCreateFlags(), 10, poolSize.size(), poolSize.data());

	
	vk::DescriptorPool descriptorPool;
	V_CHECKCALL(global.deviceWrapper.device.createDescriptorPool(&createInfo, nullptr, &descriptorPool), printf("Creation of DescriptorSetPool failed\n"));
	return descriptorPool;
}
std::vector<vk::DescriptorSet> createDescriptorSets(vk::DescriptorPool descriptorSetPool, std::vector<vk::DescriptorSetLayout>* descriptorSetLayouts){
	
	return global.deviceWrapper.device.allocateDescriptorSets(vk::DescriptorSetAllocateInfo(descriptorSetPool, descriptorSetLayouts->size(), descriptorSetLayouts->data()));
}