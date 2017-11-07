
#include "VHeader.h"
#include "VGlobal.h"
#include "DataWrapper.h"

VkCommandPool singleTransferCommandPool = VK_NULL_HANDLE;
VkCommandPool transferCommandBuffer = VK_NULL_HANDLE;
VkCommandBuffer singleTransferCommandBuffer = VK_NULL_HANDLE;
VkCommandBuffer singleImageTransitionBuffer = VK_NULL_HANDLE;



void createBuffer ( VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags needed, VkMemoryPropertyFlags recommended, VkBuffer* buffer, VkDeviceMemory* bufferMemory ) {
	VkBufferCreateInfo bufferInfo = {};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size;
	bufferInfo.usage = usage;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VCHECKCALL ( vkCreateBuffer ( vGlobal.deviceWrapper.device, &bufferInfo, nullptr, buffer ), printf ( "Failed To Create Buffer\n" ) );

	printf ( "Create Buffer of size %d\n", size );

	*bufferMemory = allocateBufferMemory ( *buffer, needed, recommended );

	vkBindBufferMemory ( vGlobal.deviceWrapper.device, *buffer, *bufferMemory, 0 );
}
void destroyBuffer ( VkBuffer buffer, VkDeviceMemory bufferMemory ) {
	vkDestroyBuffer ( vGlobal.deviceWrapper.device, buffer, nullptr );
	vkFreeMemory ( vGlobal.deviceWrapper.device, bufferMemory, nullptr );
}

void copyBuffer ( VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize srcOffset, VkDeviceSize dstOffset, VkDeviceSize size,
		VkPipelineStageFlags inputPipelineStageFlags, VkAccessFlags inputAccessFlag, VkPipelineStageFlags outputPipelineStageFlags, VkAccessFlags outputAccessFlag) {

	if ( singleTransferCommandPool == VK_NULL_HANDLE ) {
		VTQueue* queue = vGlobal.deviceWrapper.requestTransferQueue();
		if ( queue )
			singleTransferCommandPool = createCommandPool ( queue->transferQId, vk::CommandPoolCreateFlags(vk::CommandPoolCreateFlagBits::eTransient) );
		else
			singleTransferCommandPool = createCommandPool ( vGlobal.deviceWrapper.getPGCQueue()->graphicsQId, vk::CommandPoolCreateFlags(vk::CommandPoolCreateFlagBits::eTransient)  );
	}

	VkCommandBuffer commandBuffer = createCommandBuffer ( singleTransferCommandPool, vk::CommandBufferLevel::ePrimary );

	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer ( commandBuffer, &beginInfo );

	VkBufferCopy copyRegion = {};
	copyRegion.srcOffset = srcOffset; // Optional
	copyRegion.dstOffset = dstOffset; // Optional
	copyRegion.size = size;
	
	VkBufferMemoryBarrier bufferMemoryBarriers[2];
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

	VkSubmitInfo submitInfo = {};
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

void transferData ( const void* srcData, VkBuffer targetBuffer, VkDeviceSize offset, VkDeviceSize size, VkPipelineStageFlags usePipelineFlags, VkAccessFlags useFlags) {
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
