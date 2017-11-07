
#include "VHeader.h"
#include "VGlobal.h"



std::vector<vk::DescriptorSetLayout> createStandardDescriptorSetLayouts(){
	
	vk::DescriptorSetLayoutBinding bindings[] = {
		vk::DescriptorSetLayoutBinding(0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex, nullptr)
	};
	vk::DescriptorSetLayoutCreateInfo createInfo(vk::DescriptorSetLayoutCreateFlags(), 1, bindings);
	
	std::vector<vk::DescriptorSetLayout> descriptorSetLayouts;
	descriptorSetLayouts.resize(1);
	V_CHECKCALL(vGlobal.deviceWrapper.device.createDescriptorSetLayout(&createInfo, nullptr, descriptorSetLayouts.data()), printf("Creation of DescriptorSetLayout failed\n"));
	
	return descriptorSetLayouts;
}
void destroyDescriptorSetLayout(vk::DescriptorSetLayout layout){
	vGlobal.deviceWrapper.device.destroyDescriptorSetLayout(layout);
}
vk::DescriptorPool createStandardDescriptorSetPool(){
	
	std::array<vk::DescriptorPoolSize, 1> poolSize = {vk::DescriptorPoolSize(vk::DescriptorType::eUniformBuffer, 10)};
	
	vk::DescriptorPoolCreateInfo createInfo(vk::DescriptorPoolCreateFlags(), 10, poolSize.size(), poolSize.data());

	
	vk::DescriptorPool descriptorPool;
	V_CHECKCALL(vGlobal.deviceWrapper.device.createDescriptorPool(&createInfo, nullptr, &descriptorPool), printf("Creation of DescriptorSetPool failed\n"));
	return descriptorPool;
}
void createStandardDescriptorSet(vk::DescriptorPool descriptorSetPool, std::vector<vk::DescriptorSetLayout>* descriptorSetLayouts, std::vector<vk::DescriptorSet>* descriptorSets){
	
	vk::DescriptorSetAllocateInfo createInfo(descriptorSetPool, descriptorSetLayouts->size(), descriptorSetLayouts->data());
	
	descriptorSets->resize(descriptorSetLayouts->size());
	
	V_CHECKCALL(vGlobal.deviceWrapper.device.allocateDescriptorSets(&createInfo, descriptorSets->data()), printf("Creation of DescriptorSetPool failed\n"));
}