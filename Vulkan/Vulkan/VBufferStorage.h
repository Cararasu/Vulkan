#pragma once

#include "VHeader.h"
#include "VBuffer.h"
#include <atomic>
#include <render/IdArray.h>

struct VInstance;

struct VTransientBufferStorage {

	VBuffer buffer;
	VBuffer* staging_buffer;
	
	u64 max_offset;
	u64 needed_size;
	
	bool changed = false;
	vk::Semaphore transfer_sem;

	VTransientBufferStorage ( VInstance* instance, vk::BufferUsageFlags usageflags );
	~VTransientBufferStorage ( );

	u64 allocate_chunk ( u64 size );
	
	//will allocate a transfer buffer
	void* allocate_transferbuffer();
	
	//updates the data to the buffer
	void transfer_data();
	
	void clear_transfer();
};

struct VUpdateableBufferStorage {
	VBuffer buffer;
	VBuffer* staging_buffer;
	
	u64 max_offset;
	u64 needed_size;
	

	VUpdateableBufferStorage ( VInstance* instance, vk::BufferUsageFlags usageflags );
	~VUpdateableBufferStorage ( );
	
	u64 allocate_chunk ( u64 size );
	void free_chunk ( u64 size );
	
	//will allocate a transfer buffer
	void* allocate_transferbuffer();
	
	//updates the data to the buffer
	void transfer_data();
	
	void clear_transfer();
};
