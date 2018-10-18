#pragma once

#include "VHeader.h"
#include "VBuffer.h"
#include <atomic>
#include <render/IdArray.h>

struct VInstance;

struct VBufferStorage {

	VBuffer buffer;
	VBuffer* staging_buffer;
	
	u64 max_offset;
	u64 needed_size;

	VBufferStorage ( VInstance* instance, vk::BufferUsageFlags usageflags );
	~VBufferStorage ( );

	u64 allocate_chunk ( u64 size );
	
	//will allocate a transfer buffer
	void* allocate_transferbuffer();
	
	//updates the data to the buffer
	void transfer_data();
	
	void clear_transfer();
};
