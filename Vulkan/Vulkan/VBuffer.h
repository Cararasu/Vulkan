#pragma once

#include "VHeader.h"
#include <render/Resources.h>

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

	RendResult init ( vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags needed, vk::MemoryPropertyFlags recommended = vk::MemoryPropertyFlags() );
	RendResult init();

	RendResult map_mem();
	RendResult unmap_mem();

	void destroy();
};
struct VThinBuffer {
	VInstance* v_instance = nullptr;
	vk::Buffer buffer;
	vk::DeviceSize size = 0;

	void* mapped_ptr = nullptr;

	VThinBuffer ( ) {}
	VThinBuffer ( VInstance* instance, vk::Buffer buffer, vk::DeviceSize size, void* mapped_ptr);
	~VThinBuffer();
};
struct VDividableMemory {
	GPUMemory memory;
	u64 offset;
	void* mapped_ptr = nullptr;
};
constexpr u64 MAX_MEMORY_CUNK_SIZE = 128 * 1024;
struct VDividableBufferStore {
	VInstance* v_instance;
	DynArray<VDividableMemory> memory_chunks;
	DynArray<vk::Buffer> buffers;
	vk::BufferUsageFlags usage;
	vk::MemoryPropertyFlags needed;
	vk::MemoryPropertyFlags recommended;

	VDividableBufferStore ( VInstance* v_instance, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags needed, vk::MemoryPropertyFlags recommended = vk::MemoryPropertyFlags() );
	~VDividableBufferStore();

	VThinBuffer acquire_buffer(u64 size);
	void free_buffers();

	void destroy();
};