
#include "VHeader.h"
#include <stdio.h>
#include <cstdlib>
#include <GLFW/glfw3.h>
#include "VGlobal.h"


vk::DeviceMemory allocateMemory(vk::MemoryRequirements memoryRequirement, vk::MemoryPropertyFlags needed, vk::MemoryPropertyFlags recommended){
	vk::MemoryAllocateInfo allocInfo(memoryRequirement.size, findMemoryType(memoryRequirement.memoryTypeBits, needed | recommended));

	if(allocInfo.memoryTypeIndex == -1){
		allocInfo.memoryTypeIndex = findMemoryType(memoryRequirement.memoryTypeBits, needed);
	}
	printf("Allocate %d Bytes\n", memoryRequirement.size);

	vk::DeviceMemory memory;
	V_CHECKCALL(vGlobal.deviceWrapper.device.allocateMemory(&allocInfo, nullptr, &memory), printf("Failed To Create Image Memory\n"));
	return memory;
}
vk::DeviceMemory allocateImageMemory(vk::Image image, vk::MemoryPropertyFlags needed, vk::MemoryPropertyFlags recommended){
	vk::MemoryRequirements memRequirements;
	vGlobal.deviceWrapper.device.getImageMemoryRequirements(image, &memRequirements);
	return allocateMemory(memRequirements, needed, recommended);
}
vk::DeviceMemory allocateBufferMemory(vk::Buffer buffer, vk::MemoryPropertyFlags needed, vk::MemoryPropertyFlags recommended){
	vk::MemoryRequirements memRequirements;
	vGlobal.deviceWrapper.device.getBufferMemoryRequirements(buffer, &memRequirements);
	return allocateMemory(memRequirements, needed, recommended);
}

vk::CommandPool createTransferCommandPool(vk::CommandPoolCreateFlags createFlags){
	uint32_t qId;
	if(vGlobal.deviceWrapper.transfQId)
		qId = vGlobal.deviceWrapper.transfQId;
	else
		qId = vGlobal.deviceWrapper.graphQId;
	vk::CommandPoolCreateInfo createInfo(vk::CommandPoolCreateFlags(createFlags), qId);
	
	vk::CommandPool commandPool;
	V_CHECKCALL(vGlobal.deviceWrapper.device.createCommandPool(&createInfo, nullptr, &commandPool), printf("Creation of transfer CommandPool Failed\n"));
	return commandPool;
}
vk::CommandPool createGraphicsCommandPool(vk::CommandPoolCreateFlags createFlags){
	vk::CommandPoolCreateInfo createInfo(vk::CommandPoolCreateFlags(createFlags), vGlobal.deviceWrapper.graphQId);
	
	vk::CommandPool commandPool;
	V_CHECKCALL(vGlobal.deviceWrapper.device.createCommandPool(&createInfo, nullptr, &commandPool), printf("Creation of transfer CommandPool Failed\n"));
	return commandPool;
}
void destroyCommandPool(vk::CommandPool commandPool){
	vkDestroyCommandPool(vGlobal.deviceWrapper.device, commandPool, nullptr);
}
vk::CommandBuffer createCommandBuffer(vk::CommandPool commandPool, vk::CommandBufferLevel bufferLevel){
	vk::CommandBufferAllocateInfo allocateInfo(commandPool, bufferLevel, 1);
	
    vk::CommandBuffer commandBuffer;
	vGlobal.deviceWrapper.device.allocateCommandBuffers(&allocateInfo, &commandBuffer);
    return commandBuffer;
}
void deleteCommandBuffer(vk::CommandPool commandPool, vk::CommandBuffer commandBuffer){
	vGlobal.deviceWrapper.device.freeCommandBuffers(commandPool, 1, &commandBuffer);
}

void copyData(const void* srcData, vk::DeviceMemory dstMemory, vk::DeviceSize offset, vk::DeviceSize size) {
	void* data;
	vkMapMemory(vGlobal.deviceWrapper.device, dstMemory, offset, size, 0, &data);
		memcpy(data, srcData, size);
	vkUnmapMemory(vGlobal.deviceWrapper.device, dstMemory);
}

uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties) {
	vk::PhysicalDeviceMemoryProperties memProperties;
	vGlobal.physicalDevice.getMemoryProperties(&memProperties);
	
	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
		if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
			return i;
		}
	}
	return -1;
}

vk::Format findSupportedFormat(const std::vector<vk::Format>& candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features) {
	for (vk::Format format : candidates) {
		vk::FormatProperties props;
		vGlobal.physicalDevice.getFormatProperties(format, &props);
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

vk::Format findDepthFormat() {
    return findSupportedFormat(
        {vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint, vk::Format::eD32Sfloat},
        vk::ImageTiling::eOptimal,
        vk::FormatFeatureFlagBits::eDepthStencilAttachment
    );
}

vk::Semaphore createSemaphore(){
	vk::SemaphoreCreateInfo semaphoreCreateInfo;
	vk::Semaphore sem;
	V_CHECKCALL(vGlobal.deviceWrapper.device.createSemaphore(&semaphoreCreateInfo, nullptr, &sem), printf("Creation of Semaphore failed\n"));
	return sem;
}

void destroySemaphore(vk::Semaphore semaphore){
	vGlobal.deviceWrapper.device.destroySemaphore(semaphore, nullptr);
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
		vGlobal.terminate();
		glfwTerminate();
		exit(res);
	}
}
