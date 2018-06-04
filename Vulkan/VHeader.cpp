
#include "VHeader.h"
#include <stdio.h>
#include <cstdlib>
#include <GLFW/glfw3.h>
#include "VGlobal.h"
#include "VInstance.h"


vk::DeviceMemory VInstance::allocateMemory(vk::MemoryRequirements memoryRequirement, vk::MemoryPropertyFlags properties){
	u32 memoryType = findMemoryType(memoryRequirement.memoryTypeBits, properties);
	if(memoryType == -1)
		return vk::DeviceMemory();
		
	vk::MemoryAllocateInfo allocInfo(memoryRequirement.size, memoryType);
	printf("Allocate %d Bytes\n", memoryRequirement.size);

	vk::DeviceMemory memory;
	V_CHECKCALL(device.allocateMemory(&allocInfo, nullptr, &memory), printf("Failed To Create Image Memory\n"));
	return memory;
}

vk::CommandPool VInstance::createTransferCommandPool(vk::CommandPoolCreateFlags createFlags){
	u32 qId;
	if(transfQId)
		qId = transfQId;
	else
		qId = graphQId;
	vk::CommandPoolCreateInfo createInfo(vk::CommandPoolCreateFlags(createFlags), qId);
	
	vk::CommandPool commandPool;
	V_CHECKCALL(device.createCommandPool(&createInfo, nullptr, &commandPool), printf("Creation of transfer CommandPool Failed\n"));
	return commandPool;
}
vk::CommandPool VInstance::createGraphicsCommandPool(vk::CommandPoolCreateFlags createFlags){
	vk::CommandPoolCreateInfo createInfo(vk::CommandPoolCreateFlags(createFlags), graphQId);
	
	vk::CommandPool commandPool;
	V_CHECKCALL(device.createCommandPool(&createInfo, nullptr, &commandPool), printf("Creation of transfer CommandPool Failed\n"));
	return commandPool;
}
void VInstance::destroyCommandPool(vk::CommandPool commandPool){
	vkDestroyCommandPool(device, commandPool, nullptr);
}
vk::CommandBuffer VInstance::createCommandBuffer(vk::CommandPool commandPool, vk::CommandBufferLevel bufferLevel){
	vk::CommandBufferAllocateInfo allocateInfo(commandPool, bufferLevel, 1);
	
    vk::CommandBuffer commandBuffer;
	device.allocateCommandBuffers(&allocateInfo, &commandBuffer);
    return commandBuffer;
}
void VInstance::deleteCommandBuffer(vk::CommandPool commandPool, vk::CommandBuffer commandBuffer){
	device.freeCommandBuffers(commandPool, 1, &commandBuffer);
}

void VInstance::copyData(const void* srcData, vk::DeviceMemory dstMemory, vk::DeviceSize offset, vk::DeviceSize size) {
	void* data;
	vkMapMemory(device, dstMemory, offset, size, 0, &data);
		memcpy(data, srcData, size);
	vkUnmapMemory(device, dstMemory);
}

u32 VInstance::findMemoryType(u32 typeFilter, vk::MemoryPropertyFlags properties) {
	vk::PhysicalDeviceMemoryProperties memProperties;
	physicalDevice.getMemoryProperties(&memProperties);
	
	for (u32 i = 0; i < memProperties.memoryTypeCount; i++) {
		if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
			return i;
		}
	}
	return -1;
}

vk::Format VInstance::findSupportedFormat(const std::vector<vk::Format>& candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features) {
	for (vk::Format format : candidates) {
		vk::FormatProperties props;
		physicalDevice.getFormatProperties(format, &props);
		if (tiling == vk::ImageTiling::eLinear && (props.linearTilingFeatures & features) == features) {
			return format;
		} else if (tiling == vk::ImageTiling::eOptimal && (props.optimalTilingFeatures & features) == features) {
			return format;
		}
	}
	assert(false);
	return vk::Format::eUndefined;
}

bool hasStencilComponent(vk::Format format) {
	return format == vk::Format::eD32SfloatS8Uint || format == vk::Format::eD24UnormS8Uint;
}

vk::Format VInstance::findDepthFormat() {
    return findSupportedFormat(
        {vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint, vk::Format::eD32Sfloat},
        vk::ImageTiling::eOptimal,
        vk::FormatFeatureFlagBits::eDepthStencilAttachment
    );
}

vk::Semaphore VInstance::createSemaphore(){
	return device.createSemaphore(vk::SemaphoreCreateInfo(), nullptr);
}

void VInstance::destroySemaphore(vk::Semaphore semaphore){
	device.destroySemaphore(semaphore, nullptr);
}

void printError(VkResult res){
	switch(res){
	case VK_SUCCESS:
		puts("Success!\n");
		break;
	case VK_NOT_READY:
		puts("Not Ready!\n");
		break;
	case VK_TIMEOUT:
		puts("Timeout!\n");
		break;
	case VK_EVENT_SET:
		puts("Event Set!\n");
		break;
	case VK_EVENT_RESET:
		puts("Event Reset!\n");
		break;
	case VK_RESULT_END_RANGE:
		puts("End Range / Incomplete!\n");
		break;
	case VK_SUBOPTIMAL_KHR:
		puts("Suboptimal!\n");
		break;
	case VK_RESULT_RANGE_SIZE:
		puts("Range Size!\n");
		break;
	case VK_RESULT_MAX_ENUM:
		puts("Max Enum!\n");
		break;
	case VK_ERROR_OUT_OF_HOST_MEMORY:
		puts("OOM Host!\n");
		break;
	case VK_ERROR_OUT_OF_DEVICE_MEMORY:
		puts("OOM Device!\n");
		break;
	case VK_ERROR_INITIALIZATION_FAILED:
		puts("Init failed!\n");
		break;
	case VK_ERROR_DEVICE_LOST:
		puts("Device lost!\n");
		break;
	case VK_ERROR_MEMORY_MAP_FAILED:
		puts("Memory map failed!\n");
		break;
	case VK_ERROR_LAYER_NOT_PRESENT:
		puts("Layer not Ppresent!\n");
		break;
	case VK_ERROR_EXTENSION_NOT_PRESENT:
		puts("Extension not present!\n");
		break;
	case VK_ERROR_FEATURE_NOT_PRESENT:
		puts("Feature not present!\n");
		break;
	case VK_ERROR_INCOMPATIBLE_DRIVER:
		puts("Incompatible Driver!\n");
		break;
	case VK_ERROR_TOO_MANY_OBJECTS:
		puts("Too many Objects!\n");
		break;
	case VK_ERROR_FORMAT_NOT_SUPPORTED:
		puts("Format not supported!\n");
		break;
	case VK_ERROR_FRAGMENTED_POOL:
		puts("Fragmented pool/Begin Range!\n");
		break;
	case VK_ERROR_SURFACE_LOST_KHR:
		puts("Surface lost!\n");
		break;
	case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
		puts("Native window in use!\n");
		break;
	case VK_ERROR_OUT_OF_DATE_KHR:
		puts("Incomplete!\n");
		break;
	case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR:
		puts("Incompatible display!\n");
		break;
	case VK_ERROR_VALIDATION_FAILED_EXT:
		puts("Validation failed!\n");
		break;
	case VK_ERROR_INVALID_SHADER_NV:
		puts("Invalid Shader!\n");
		break;
	case VK_ERROR_OUT_OF_POOL_MEMORY_KHR:
		puts("OOM pool!\n");
		break;
	case VK_ERROR_INVALID_EXTERNAL_HANDLE_KHR:
		puts("Invalid External Handle!\n");
		break;
	default:
		printf("Other Error 0x%x\n", res);
	}
	if(res < 0){
		global.terminate();
		glfwTerminate();
		exit(res);
	}
}
