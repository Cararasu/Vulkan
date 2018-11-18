#pragma once

#include "VHeader.h"

struct VInstance;
struct VSimpleTransferJob;

struct VBuffer {
	VInstance* v_instance;
	GPUMemory memory;
	vk::Buffer buffer;
	vk::DeviceSize size;
	vk::BufferUsageFlags usage;
	vk::MemoryPropertyFlags needed;
	vk::MemoryPropertyFlags recommended;
	
	void* mapped_ptr = nullptr;
	u64 last_build_frame_index = 0;

	VBuffer ( VInstance* instance );
	VBuffer ( VInstance* instance, vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags needed, vk::MemoryPropertyFlags recommended = vk::MemoryPropertyFlags() );
	~VBuffer();
	
	RendResult init(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags needed, vk::MemoryPropertyFlags recommended = vk::MemoryPropertyFlags());
	RendResult init();

	RendResult map_mem();
	RendResult unmap_mem();

	void destroy();
};

RendResult transfer_buffer_data(VSimpleTransferJob& job, vk::CommandBuffer commandBuffer);
