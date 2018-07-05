#pragma once

#include "VulkanInstance.h"

struct VulkanBuffer {

	VulkanInstance* v_instance;
	GPUMemory memory;
	vk::Buffer buffer;

	VulkanBuffer ( VulkanInstance* instance, vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags needed, vk::MemoryPropertyFlags recommended = vk::MemoryPropertyFlags() );
	~VulkanBuffer();

	void destroy();
};
