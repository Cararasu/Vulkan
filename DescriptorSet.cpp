
#include "VHeader.h"
#include "VGlobal.h"



std::vector<vk::DescriptorSetLayout> createStandardDescriptorSetLayouts(){
	vk::DescriptorSetLayoutCreateInfo createInfo = {};
	
	createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	createInfo.pNext = nullptr;
	createInfo.flags = 0;
	
	vk::DescriptorSetLayoutBinding bindings[1];
	bindings[0].binding = 0;
	bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	bindings[0].descriptorCount = 1;
	bindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	bindings[0].pImmutableSamplers = nullptr;
	
	createInfo.bindingCount = 1;
	createInfo.pBindings = bindings;
	
	std::vector<vk::DescriptorSetLayout> descriptorSetLayouts;
	descriptorSetLayouts.resize(1);
	VCHECKCALL(vkCreateDescriptorSetLayout(vGlobal.deviceWrapper.device, &createInfo, nullptr, descriptorSetLayouts.data()),printf("Creation of DescriptorSetLayout failed\n"));
	return descriptorSetLayouts;
}
void destroyDescriptorSetLayout(vk::DescriptorSetLayout layout){
	vkDestroyDescriptorSetLayout(vGlobal.deviceWrapper.device, layout, nullptr);
}
vk::DescriptorPool createStandardDescriptorSetPool(){
	vk::DescriptorPoolCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    createInfo.maxSets = 10;
    createInfo.poolSizeCount = 1;
	vk::DescriptorPoolSize poolSize[1];
	poolSize[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSize[0].descriptorCount = 10;
    createInfo.pPoolSizes = poolSize;
	
	vk::DescriptorPool descriptorPool;;
	VCHECKCALL(vkCreateDescriptorPool(vGlobal.deviceWrapper.device, &createInfo, nullptr, &descriptorPool),printf("Creation of DescriptorSetPool failed\n"));
	return descriptorPool;
}
void createStandardDescriptorSet(vk::DescriptorPool descriptorSetPool, std::vector<vk::DescriptorSetLayout>* descriptorSetLayouts, std::vector<vk::DescriptorSet>* descriptorSets){
	
	vk::DescriptorSetAllocateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.descriptorPool = descriptorSetPool;
    createInfo.descriptorSetCount = descriptorSetLayouts->size();
    createInfo.pSetLayouts = descriptorSetLayouts->data();
	
	descriptorSets->resize(descriptorSetLayouts->size());
	
	VCHECKCALL(vkAllocateDescriptorSets(vGlobal.deviceWrapper.device, &createInfo, descriptorSets->data()),printf("Creation of DescriptorSetPool failed\n"));
}