#pragma once

#include "VulkanBuffer.h"
#include <atomic>


struct VulkanTransferBuffer {
	VulkanBuffer buffer;
	vk::DeviceSize used_bytes;
	std::atomic<u32> uses;

	VulkanTransferBuffer ( VulkanInstance* instance, vk::DeviceSize buffersize );
	~VulkanTransferBuffer ( );
};
/*struct VulkanTransferJob {
	VulkanTransferBuffer* used_transfer_buffer;
	VulkanBuffer* dst_buffer;
	u32 transfer_offset, final_offset, byte_size;
	bool is_running;
	vk::Fence finished_fence;
};*/
struct VulkanTransferController {
	VulkanInstance* v_instance;
	Array<VulkanTransferBuffer*> transfer_buffers;
	//Array<VulkanTransferJob> transfer_jobs;
	vk::CommandPool commandpool;
	vk::CommandBuffer synchron_commandbuffer;

	VulkanTransferController ( VulkanInstance* v_instance );
	~VulkanTransferController();

	void transfer_data ( u8 * data, u32 size, VulkanBuffer* dstbuffer, vk::DeviceSize offset );
	void transfer_data ( u8 * data, u32 size, VulkanBuffer* dstbuffer, vk::DeviceSize offset, vk::CommandBuffer commandbuffer );
};