#pragma once

#include "VulkanInstance.h"

struct VulkanBuffer {

	VulkanInstance* v_instance;
	GPUMemory memory;
	vk::Buffer buffer;
	void* mapped_ptr = nullptr;

	VulkanBuffer ( VulkanInstance* instance, vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags needed, vk::MemoryPropertyFlags recommended = vk::MemoryPropertyFlags() );
	~VulkanBuffer();

	RendResult map_mem();
	RendResult unmap_mem();
	
	RendResult transfer_to(VulkanBuffer* dst, vk::DeviceSize offset, vk::DeviceSize size, vk::CommandBuffer commandBuffer);
	
	inline RendResult transfer_to(VulkanBuffer* dst, vk::CommandBuffer commandBuffer){
		return transfer_to(dst, 0, std::min(memory.size, dst->memory.size), commandBuffer);
	}

	void destroy();
};
