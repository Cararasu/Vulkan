#include "VulkanTransferOperator.h"
#include "VulkanInstance.h"


VulkanTransferController::VulkanTransferController ( VulkanInstance* instance ) : v_instance ( instance ), transfer_buffers(), commandpool(), synchron_commandbuffer() {

}
VulkanTransferController::~VulkanTransferController() {
	if ( synchron_commandbuffer ) {
		vulkan_device ( v_instance ).freeCommandBuffers ( commandpool, 1, &synchron_commandbuffer );
	}
	if ( commandpool ) {
		vulkan_device ( v_instance ).destroyCommandPool ( commandpool );
	}
}
void VulkanTransferController::transfer_data ( u8 * data, u32 size, VulkanBuffer* dstbuffer, vk::DeviceSize offset ) {
	if ( !commandpool ) {
		vk::CommandPoolCreateInfo createInfo ( vk::CommandPoolCreateFlagBits::eResetCommandBuffer, vulkan_pgc_queue_wrapper ( v_instance, 0 )->graphics_queue_id );
		vulkan_device ( v_instance ).createCommandPool ( &createInfo, nullptr, &commandpool );
	}
	if ( synchron_commandbuffer ) {
		synchron_commandbuffer.reset ( vk::CommandBufferResetFlags() );
	} else {
		vk::CommandBufferAllocateInfo allocateInfo ( commandpool, vk::CommandBufferLevel::ePrimary, 1 );
		vulkan_device ( v_instance ).allocateCommandBuffers ( &allocateInfo, &synchron_commandbuffer );
	}
	//should be transfered to here:
	//transfer_data(data, size, dstbuffer, offset, synchron_commandbuffer);
	VulkanBuffer stagingbuffer ( v_instance, size,
	                             vk::BufferUsageFlags() | vk::BufferUsageFlagBits::eTransferSrc,
	                             vk::MemoryPropertyFlags() | vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostVisible );
	stagingbuffer.map_mem();
	memcpy ( stagingbuffer.mapped_ptr, data, size );

	synchron_commandbuffer.begin ( vk::CommandBufferBeginInfo ( vk::CommandBufferUsageFlagBits::eOneTimeSubmit ) );
	vk::BufferCopy copyRegion ( 0, offset, size );
	synchron_commandbuffer.copyBuffer ( stagingbuffer.buffer, dstbuffer->buffer, 1, &copyRegion );
	synchron_commandbuffer.end();
	vk::PipelineStageFlags waitflags = vk::PipelineStageFlagBits::eHost;
	vk::SubmitInfo submitInfo ( 0, nullptr, &waitflags,
	                            1, &synchron_commandbuffer,
	                            0, nullptr );
	//should be transfer queue
	v_instance->queues.pgc[0].graphics_queue.submit ( 1, &submitInfo, vk::Fence() );
	v_instance->queues.pgc[0].graphics_queue.waitIdle();
}
void VulkanTransferController::transfer_data ( u8 * data, u32 size, VulkanBuffer* dstbuffer, vk::DeviceSize offset, vk::CommandBuffer commandbuffer ) {

}

VulkanTransferBuffer::VulkanTransferBuffer ( VulkanInstance* instance, vk::DeviceSize buffersize ) :
	buffer ( instance, buffersize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlags() | vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostVisible ),
	used_bytes ( 0 ), uses ( 0 ) {
	buffer.map_mem();
}
VulkanTransferBuffer::~VulkanTransferBuffer ( ) {
	
}