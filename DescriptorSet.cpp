
#include "VHeader.h"
#include "VGlobal.h"



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