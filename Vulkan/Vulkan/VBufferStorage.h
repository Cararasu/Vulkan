#pragma once

#include "VHeader.h"
#include "VBuffer.h"
#include <atomic>
#include <render/IdArray.h>

struct VInstance;

struct VTransientBufferStorage {

	VBuffer buffer;
	VThinBuffer staging_buffer;

	u64 max_offset;
	u64 needed_size;

	bool changed = false;
	vk::Semaphore transfer_sem;

	VTransientBufferStorage ( VInstance* instance, vk::BufferUsageFlags usageflags );
	~VTransientBufferStorage ( );

	u64 allocate_chunk ( u64 size );
	void* allocate_transfer_buffer( );

	//updates the data to the buffer
	vk::Semaphore transfer_data ( vk::CommandBuffer buffer );

	void clear_transfer();
};

constexpr u64 UNIFORM_BUFFER_SIZE = 1024 * 16;

struct Chunk {
	u64 index, offset, size;
};

struct VUpdateableBufferStorage {
	VInstance* v_instance;
	vk::BufferUsageFlags usageflags;
	DynArray<std::pair<VBuffer*, VThinBuffer>> buffers;
	DynArray<Chunk> freelist;

	VUpdateableBufferStorage ( VInstance* instance, vk::BufferUsageFlags usageflags );
	~VUpdateableBufferStorage ( );

	Chunk allocate_chunk ( u64 size );
	VBuffer* get_buffer ( u64 index );
	void fetch_transferbuffers();
	std::pair<VBuffer*, VThinBuffer> get_buffer_pair ( u64 index );
	void free_transferbuffers(u64 frame_index);
	void free_chunk ( Chunk chunk );
};
struct VDividableMemory {
	GPUMemory memory;
	u64 offset;
	void* mapped_ptr = nullptr;
};
//store for staging buffers
constexpr u64 MAX_MEMORY_CUNK_SIZE = 16 * 1024 * 1024;
struct VDividableBufferStore {
	VInstance* v_instance;
	DynArray<VDividableMemory> memory_chunks;
	DynArray<VDividableMemory> special_memory_chunks;
	DynArray<vk::Buffer> buffers;
	vk::BufferUsageFlags usage;
	vk::MemoryPropertyFlags needed;
	vk::MemoryPropertyFlags recommended;
	u64 last_frame_index_acquired = 0;

	VDividableBufferStore ( VInstance* v_instance, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags needed, vk::MemoryPropertyFlags recommended = vk::MemoryPropertyFlags() );
	~VDividableBufferStore();

	VThinBuffer acquire_buffer(u64 size);
	void free_buffers();

	void destroy();
};