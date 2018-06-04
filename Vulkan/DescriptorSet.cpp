
#include "VHeader.h"
#include "VGlobal.h"
#include "VInstance.h"



void VInstance::destroyDescriptorSetLayout(vk::DescriptorSetLayout layout){
	device.destroyDescriptorSetLayout(layout);
}
vk::DescriptorPool createStandardDescriptorSetPool(VInstance* instance){
	
	std::array<vk::DescriptorPoolSize, 3> poolSize = {
		vk::DescriptorPoolSize(vk::DescriptorType::eUniformBuffer, 10),
		vk::DescriptorPoolSize(vk::DescriptorType::eSampledImage, 10),
		vk::DescriptorPoolSize(vk::DescriptorType::eSampler, 10)
	};
	
	vk::DescriptorPoolCreateInfo createInfo(vk::DescriptorPoolCreateFlags(), 10, poolSize.size(), poolSize.data());

	
	vk::DescriptorPool descriptorPool;
	V_CHECKCALL(instance->device.createDescriptorPool(&createInfo, nullptr, &descriptorPool), printf("Creation of DescriptorSetPool failed\n"));
	return descriptorPool;
}
std::vector<vk::DescriptorSet> VInstance::createDescriptorSets(vk::DescriptorPool descriptorSetPool, std::vector<vk::DescriptorSetLayout>* descriptorSetLayouts){
	
	return device.allocateDescriptorSets(vk::DescriptorSetAllocateInfo(descriptorSetPool, descriptorSetLayouts->size(), descriptorSetLayouts->data()));
}