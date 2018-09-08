#pragma once

#include "VulkanHeader.h"
#include "VulkanBuffer.h"
#include <atomic>
#include <render/IdArray.h>

struct VulkanInstance;

struct VulkanBufferStorage {
	struct VulkanBufferSection {
		RId id;
		u32 offset, size;
	};

	VulkanBuffer buffer;
	vk::Semaphore semaphore;
	IdArray<VulkanBufferSection> sections;
	u8* mapped_transfer_buffer;
	u32 needed_size;

	VulkanBufferStorage ( VulkanInstance* instance, vk::BufferUsageFlags usageflags );

	RId allocate_chunk ( u32 size );
	void free_chunk ( RId index );
	void reorganize();
	u32 get_offset ( RId index );
	u8* get_ptr ( RId index );
	void update_data();
};

struct VulkanTransferBuffer {
	VulkanBuffer buffer;
	vk::Semaphore semaphore;
	VulkanTransferBuffer ( VulkanInstance* instance, vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags needed, vk::MemoryPropertyFlags recommended = vk::MemoryPropertyFlags() );
	~VulkanTransferBuffer();
};
struct VulkanTransferJob {
	VulkanTransferBuffer* used_transfer_buffer;
	VulkanBuffer* dst_buffer;
	u32 final_offset, byte_size;
};
struct VulkanPerFrameTransferData {
	vk::Fence fence;
	vk::CommandBuffer commandbuffer;
	DynArray<VulkanBuffer*> used_buffers;
	
	void init(VulkanInstance* v_instance, vk::CommandPool commandpool);
	void destroy(VulkanInstance* v_instance);
};
struct VulkanTransferController {
	VulkanInstance* v_instance;
	DynArray<VulkanTransferBuffer*> transfer_buffers;
	DynArray<VulkanPerFrameTransferData*> perframe_data;
	DynArray<VulkanPerFrameTransferData*> available_perframe_data;
	
	DynArray<VulkanTransferJob> jobs;
	//Array<VulkanTransferJob> transfer_jobs;
	vk::CommandPool commandpool;
	vk::CommandBuffer synchron_commandbuffer;

	VulkanTransferController (VulkanInstance* v_instance);
	~VulkanTransferController();

	std::pair<u8*, vk::Semaphore> request_transfer_memory ( VulkanBuffer* target_buffer, u32 size, u32 offset = 0 );
	void do_transfers ( );
	void check_free ( );
};
