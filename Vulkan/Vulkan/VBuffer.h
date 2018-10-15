#pragma once

#include "VHeader.h"

struct VInstance;

struct VBuffer {
	VInstance* v_instance;
	GPUMemory memory;
	vk::Buffer buffer;
	vk::DeviceSize size;
	vk::BufferUsageFlags usage;
	vk::MemoryPropertyFlags needed;
	vk::MemoryPropertyFlags recommended;
	
	void* mapped_ptr = nullptr;

	VBuffer ( VInstance* instance );
	VBuffer ( VInstance* instance, vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags needed, vk::MemoryPropertyFlags recommended = vk::MemoryPropertyFlags() );
	~VBuffer();
	
	RendResult init(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags needed, vk::MemoryPropertyFlags recommended = vk::MemoryPropertyFlags());
	RendResult init();

	RendResult map_mem();
	RendResult unmap_mem();
	
	RendResult transfer_to(VBuffer* dst, vk::DeviceSize offset, vk::DeviceSize size, vk::CommandBuffer commandBuffer);
	
	inline RendResult transfer_to(VBuffer* dst, vk::CommandBuffer commandBuffer){
		return transfer_to(dst, 0, std::min(memory.size, dst->memory.size), commandBuffer);
	}

	void destroy();
};
