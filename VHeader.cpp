
#include "VHeader.h"
#include <stdio.h>
#include <cstdlib>
#include <GLFW/glfw3.h>
#include "VGlobal.h"


VkDeviceMemory allocateMemory(VkMemoryRequirements memoryRequirement, VkMemoryPropertyFlags needed, VkMemoryPropertyFlags recommended){
	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memoryRequirement.size;
	allocInfo.memoryTypeIndex = findMemoryType(memoryRequirement.memoryTypeBits, needed | recommended);
	if(allocInfo.memoryTypeIndex == -1){
		allocInfo.memoryTypeIndex = findMemoryType(memoryRequirement.memoryTypeBits, needed);
	}
	printf("Allocate %d Bytes\n", memoryRequirement.size);

	VkDeviceMemory memory;
	VCHECKCALL(vkAllocateMemory(vGlobal.deviceWrapper.device, &allocInfo, nullptr, &memory), printf("Failed To Create Image Memory\n"));
	return memory;
}
VkDeviceMemory allocateImageMemory(VkImage image, VkMemoryPropertyFlags needed, VkMemoryPropertyFlags recommended){
	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(vGlobal.deviceWrapper.device, image, &memRequirements);
	return allocateMemory(memRequirements, needed, recommended);
}
VkDeviceMemory allocateBufferMemory(VkBuffer buffer, VkMemoryPropertyFlags needed, VkMemoryPropertyFlags recommended){
	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(vGlobal.deviceWrapper.device, buffer, &memRequirements);
	return allocateMemory(memRequirements, needed, recommended);
}

vk::CommandPool createCommandPool(uint32_t queueId, vk::CommandPoolCreateFlags createFlags){
	vk::CommandPoolCreateInfo createInfo(vk::CommandPoolCreateFlags(createFlags), queueId);
	
	vk::CommandPool commandPool;
	V_CHECKCALL(vGlobal.deviceWrapper.device.createCommandPool(&createInfo, nullptr, &commandPool), printf("Creation of transfer CommandPool Failed\n"));
	return commandPool;
}
void destroyCommandPool(VkCommandPool commandPool){
	vkDestroyCommandPool(vGlobal.deviceWrapper.device, commandPool, nullptr);
}
vk::CommandBuffer createCommandBuffer(vk::CommandPool commandPool, vk::CommandBufferLevel bufferLevel){
	vk::CommandBufferAllocateInfo allocateInfo(commandPool, bufferLevel, 1);
	
    vk::CommandBuffer commandBuffer;
	vGlobal.deviceWrapper.device.allocateCommandBuffers(&allocateInfo, &commandBuffer);
    return commandBuffer;
}
void deleteCommandBuffer(VkCommandPool commandPool, VkCommandBuffer commandBuffer){
	vkFreeCommandBuffers(vGlobal.deviceWrapper.device, commandPool, 1, &commandBuffer);
}

void copyData(const void* srcData, VkDeviceMemory dstMemory, VkDeviceSize offset, VkDeviceSize size) {
	void* data;
	vkMapMemory(vGlobal.deviceWrapper.device, dstMemory, offset, size, 0, &data);
		memcpy(data, srcData, size);
	vkUnmapMemory(vGlobal.deviceWrapper.device, dstMemory);
}

uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(vGlobal.physicalDevice, &memProperties);
	
	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
		if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
			return i;
		}
	}
	return -1;
}

VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) {
	for (VkFormat format : candidates) {
		VkFormatProperties props;
		vkGetPhysicalDeviceFormatProperties(vGlobal.physicalDevice, format, &props);
		if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
			return format;
		} else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
			return format;
		}
	}
	assert(false);
	return VK_FORMAT_UNDEFINED;
}

bool hasStencilComponent(VkFormat format) {
	return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}

VkFormat findDepthFormat() {
    return findSupportedFormat(
        {VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT, VK_FORMAT_D32_SFLOAT},
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
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
