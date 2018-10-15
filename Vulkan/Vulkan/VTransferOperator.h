#pragma once

#include "VHeader.h"
#include "VBuffer.h"
#include <atomic>
#include <render/IdArray.h>

struct VInstance;

struct VBufferStorage {
	struct VBufferSection {
		RId id;
		u32 offset, size;
	};

	VBuffer buffer;
	vk::Semaphore semaphore;
	IdArray<VBufferSection> sections;
	void* mapped_transfer_buffer;
	u32 needed_size;

	VBufferStorage ( VInstance* instance, vk::BufferUsageFlags usageflags );

	RId allocate_chunk ( u32 size );
	void free_chunk ( RId index );
	
	void reorganize();
	
	void* get_data_ptr ( RId index );
	void update_data();
};

struct VTransferBuffer {
	VBuffer buffer;
	vk::Semaphore semaphore;
	VTransferBuffer ( VInstance* instance, vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags needed, vk::MemoryPropertyFlags recommended = vk::MemoryPropertyFlags() );
	~VTransferBuffer();
};
struct VTransferJob {
	VTransferBuffer* used_transfer_buffer;
	VBuffer* dst_buffer;
	u32 final_offset, byte_size;
};
struct VPerFrameTransferData {
	vk::Fence fence;
	vk::CommandBuffer commandbuffer;
	DynArray<VBuffer*> used_buffers;
	
	void init(VInstance* v_instance, vk::CommandPool commandpool);
	void destroy(VInstance* v_instance);
};
struct VTransferController {
	VInstance* v_instance;
	DynArray<VTransferBuffer*> transfer_buffers;
	DynArray<VPerFrameTransferData*> perframe_data;
	DynArray<VPerFrameTransferData*> available_perframe_data;
	
	DynArray<VTransferJob> jobs;
	//Array<VTransferJob> transfer_jobs;
	vk::CommandPool commandpool;
	vk::CommandBuffer synchron_commandbuffer;

	VTransferController (VInstance* v_instance);
	~VTransferController();

	std::pair<void*, vk::Semaphore> request_transfer_memory ( VBuffer* target_buffer, u32 size, u32 offset = 0 );
	void do_transfers ( );
	void check_free ( );
};
