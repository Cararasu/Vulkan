
#include "VHeader.h"
#include "VGlobal.h"
#include "DataWrapper.h"

vk::CommandPool singleTransferCommandPool = vk::CommandPool();
vk::CommandPool transferCommandBuffer = vk::CommandPool();
vk::CommandBuffer singleTransferCommandBuffer = vk::CommandBuffer();
vk::CommandBuffer singleImageTransitionBuffer = vk::CommandBuffer();



void createBuffer ( vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags needed, vk::MemoryPropertyFlags recommended, vk::Buffer* buffer, vk::DeviceMemory* bufferMemory ) {
	vk::BufferCreateInfo bufferInfo(size, usage, vk::SharingMode::eExclusive);

	VCHECKCALL ( vGlobal.deviceWrapper.device.createBuffer(&bufferInfo, nullptr, buffer), printf ( "Failed To Create Buffer\n" ) );

	printf ( "Create Buffer of size %d\n", size );

	*bufferMemory = allocateBufferMemory ( *buffer, needed, recommended );

	vkBindBufferMemory ( vGlobal.deviceWrapper.device, *buffer, *bufferMemory, 0 );
}
void destroyBuffer ( vk::Buffer buffer, vk::DeviceMemory bufferMemory ) {
	vkDestroyBuffer ( vGlobal.deviceWrapper.device, buffer, nullptr );
	vkFreeMemory ( vGlobal.deviceWrapper.device, bufferMemory, nullptr );
}

void copyBuffer ( vk::Buffer srcBuffer, vk::Buffer dstBuffer, vk::DeviceSize srcOffset, vk::DeviceSize dstOffset, vk::DeviceSize size,
		vk::PipelineStageFlags inputPipelineStageFlags, vk::AccessFlags inputAccessFlag, vk::PipelineStageFlags outputPipelineStageFlags, vk::AccessFlags outputAccessFlag) {

	if ( singleTransferCommandPool == VK_NULL_HANDLE ) {
		VTQueue* queue = vGlobal.deviceWrapper.requestTransferQueue();
		if ( queue )
			singleTransferCommandPool = createCommandPool ( queue->transferQId, vk::CommandPoolCreateFlags(vk::CommandPoolCreateFlagBits::eTransient) );
		else
			singleTransferCommandPool = createCommandPool ( vGlobal.deviceWrapper.getPGCQueue()->graphicsQId, vk::CommandPoolCreateFlags(vk::CommandPoolCreateFlagBits::eTransient)  );
	}

	vk::CommandBuffer commandBuffer = createCommandBuffer ( singleTransferCommandPool, vk::CommandBufferLevel::ePrimary );

	vk::CommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer ( commandBuffer, &beginInfo );

	vk::BufferCopy copyRegion = {};
	copyRegion.srcOffset = srcOffset; // Optional
	copyRegion.dstOffset = dstOffset; // Optional
	copyRegion.size = size;
	
	vk::BufferMemoryBarrier bufferMemoryBarriers[2];
	bufferMemoryBarriers[0].sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
	bufferMemoryBarriers[0].pNext = nullptr;
	bufferMemoryBarriers[0].srcAccessMask = inputAccessFlag;
	bufferMemoryBarriers[0].dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
	bufferMemoryBarriers[0].srcQueueFamilyIndex = vGlobal.deviceWrapper.transfQId;
	bufferMemoryBarriers[0].dstQueueFamilyIndex = vGlobal.deviceWrapper.transfQId;
	bufferMemoryBarriers[0].buffer = srcBuffer;
	bufferMemoryBarriers[0].offset = srcOffset;
	bufferMemoryBarriers[0].size = size;

	printf("First Barrier\n");
	vkCmdPipelineBarrier ( commandBuffer,
	                       inputPipelineStageFlags, VK_PIPELINE_STAGE_TRANSFER_BIT,
	                       VK_DEPENDENCY_BY_REGION_BIT,
	                       0, nullptr,
	                       1, bufferMemoryBarriers,
	                       0, nullptr );

	vkCmdCopyBuffer ( commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion );

	
	bufferMemoryBarriers[0].srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	bufferMemoryBarriers[0].dstAccessMask = outputAccessFlag;
	bufferMemoryBarriers[0].srcQueueFamilyIndex = vGlobal.deviceWrapper.transfQId;
	bufferMemoryBarriers[0].dstQueueFamilyIndex = vGlobal.deviceWrapper.transfQId;
	bufferMemoryBarriers[0].buffer = srcBuffer;
	bufferMemoryBarriers[0].offset = srcOffset;
	bufferMemoryBarriers[0].size = size;

	printf("Second Barrier\n");
	vkCmdPipelineBarrier ( commandBuffer,
	                       VK_PIPELINE_STAGE_TRANSFER_BIT, outputPipelineStageFlags,
	                       VK_DEPENDENCY_BY_REGION_BIT,
	                       0, nullptr,
	                       1, bufferMemoryBarriers,
	                       0, nullptr );

	vkEndCommandBuffer ( commandBuffer );

	vk::SubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	VTQueue* vtQueue = vGlobal.deviceWrapper.requestTransferQueue();
	if ( vtQueue ) {
		vtQueue->submitTransfer ( 1, &submitInfo, VK_NULL_HANDLE );
		//vtQueue->waitForFinish();
	} else {
		vGlobal.deviceWrapper.getPGCQueue()->submitGraphics ( 1, &submitInfo, VK_NULL_HANDLE );
		//vGlobal.deviceWrapper.getPGCQueue()->waitForFinish();
	}
	deleteCommandBuffer ( singleTransferCommandPool, commandBuffer );
}

void transferData ( const void* srcData, vk::Buffer targetBuffer, vk::DeviceSize offset, vk::DeviceSize size, vk::PipelineStageFlags usePipelineFlags, vk::AccessFlags useFlags) {
	if ( !stagingBuffer ) {
		printf ( "Creating Staging-Buffer\n" );
		stagingBuffer = new MappedBufferWrapper ( V_MAX_STAGINGBUFFER_SIZE, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT );
	}
	MappedBufferWrapper* transferBuffer;
	if ( V_MAX_STAGINGBUFFER_SIZE < size ) {
		printf ( "Stagingbuffer not big enough -> create template\n" );
		transferBuffer = new MappedBufferWrapper ( size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT );
	}else{
		transferBuffer = stagingBuffer;
	}
	memcpy ( transferBuffer->data, srcData, size );
	copyBuffer ( transferBuffer->buffer, targetBuffer, 0, offset, size,
		VK_PIPELINE_STAGE_HOST_BIT, VK_ACCESS_HOST_WRITE_BIT, usePipelineFlags, useFlags);

	if ( V_MAX_STAGINGBUFFER_SIZE < size ) {
		delete transferBuffer;
	}

}
