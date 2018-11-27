#pragma once

#include "VHeader.h"
#include "VBuffer.h"
#include <atomic>
#include <render/IdArray.h>

struct VInstance;

struct VTransientBufferStorage {

	VBuffer buffer;
	VBuffer* staging_buffer = nullptr;

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
	DynArray<std::pair<VBuffer*, VBuffer*>> buffers;
	DynArray<Chunk> freelist;

	VUpdateableBufferStorage ( VInstance* instance, vk::BufferUsageFlags usageflags );
	~VUpdateableBufferStorage ( );

	Chunk allocate_chunk ( u64 size );
	VBuffer* get_buffer ( u64 index );
	void fetch_transferbuffers();
	std::pair<VBuffer*, VBuffer*> get_buffer_pair ( u64 index );
	void free_transferbuffers(u64 frame_index);
	void free_chunk ( Chunk chunk );
};