
#include "VHeader.h"
#include "VGlobal.h"
#include "DataWrapper.h"


void createBuffer ( vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags needed, vk::MemoryPropertyFlags recommended, vk::Buffer* buffer, vk::DeviceMemory* bufferMemory ) {
	vk::BufferCreateInfo bufferInfo(vk::BufferCreateFlags(), size, usage, vk::SharingMode::eExclusive);

	V_CHECKCALL ( vGlobal.deviceWrapper.device.createBuffer(&bufferInfo, nullptr, buffer), printf ( "Failed To Create Buffer\n" ) );

	printf ( "Create Buffer of size %d\n", size );

	*bufferMemory = allocateBufferMemory ( *buffer, needed, recommended );

	vkBindBufferMemory ( vGlobal.deviceWrapper.device, *buffer, *bufferMemory, 0 );
}
void destroyBuffer ( vk::Buffer buffer, vk::DeviceMemory bufferMemory ) {
	vkDestroyBuffer ( vGlobal.deviceWrapper.device, buffer, nullptr );
	vkFreeMemory ( vGlobal.deviceWrapper.device, bufferMemory, nullptr );
}

void copyBuffer ( vk::CommandPool commandPool, vk::Buffer srcBuffer, vk::Buffer dstBuffer, vk::DeviceSize srcOffset, vk::DeviceSize dstOffset, vk::DeviceSize size,
		vk::PipelineStageFlags inputPipelineStageFlags, vk::AccessFlags inputAccessFlag, vk::PipelineStageFlags outputPipelineStageFlags, vk::AccessFlags outputAccessFlag) {
	
	vk::CommandBuffer commandBuffer =  createCommandBuffer ( commandPool, vk::CommandBufferLevel::ePrimary );
	
	commandBuffer.begin(vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit)));
	
	vk::BufferCopy copyRegion = {};
	copyRegion.srcOffset = srcOffset; // Optional
	copyRegion.dstOffset = dstOffset; // Optional
	copyRegion.size = size;
	
	if(inputAccessFlag && inputPipelineStageFlags) {
		commandBuffer.pipelineBarrier(
			inputPipelineStageFlags, vk::PipelineStageFlagBits::eTransfer,
			vk::DependencyFlagBits::eByRegion,
			{},
			{
				vk::BufferMemoryBarrier(
					inputAccessFlag, vk::AccessFlagBits::eTransferRead,
					vGlobal.deviceWrapper.transfQId, vGlobal.deviceWrapper.transfQId,
					srcBuffer,
					srcOffset, size
				)
			},
			{}
		);
	}
	commandBuffer.copyBuffer(srcBuffer, dstBuffer, 1, &copyRegion);

	if(outputAccessFlag && outputPipelineStageFlags) {
		commandBuffer.pipelineBarrier(
			vk::PipelineStageFlagBits::eTransfer, outputPipelineStageFlags,
			vk::DependencyFlagBits::eByRegion,
			{},
			{
				vk::BufferMemoryBarrier(
					vk::AccessFlagBits::eTransferWrite, outputAccessFlag,
					vGlobal.deviceWrapper.transfQId, vGlobal.deviceWrapper.transfQId,
					srcBuffer,
					srcOffset, size
				)
			},
			{}
		);
	}

	commandBuffer.end();

	vk::SubmitInfo submitInfo(
		0, nullptr,//waitsemaphores
		nullptr,//pWaitDstStageMask
		1, &commandBuffer,
		0, nullptr//signalsemaphores
		);
	VTQueue* vtQueue = vGlobal.deviceWrapper.requestTransferQueue();
	if ( vtQueue ) {
		vtQueue->submitTransfer ( 1, &submitInfo );
		//vtQueue->waitForFinish();
	} else {
		vGlobal.deviceWrapper.getPGCQueue()->submitGraphics ( 1, &submitInfo );
		//vGlobal.deviceWrapper.getPGCQueue()->waitForFinish();
	}
}

void transferData ( vk::CommandPool transferCommandPool, const void* srcData, vk::Buffer targetBuffer, vk::DeviceSize offset, vk::DeviceSize size, vk::PipelineStageFlags usePipelineFlags, vk::AccessFlags useFlags) {
	if ( !stagingBuffer ) {
		printf ( "Creating Staging-Buffer\n" );
		stagingBuffer = new MappedBufferWrapper ( V_MAX_STAGINGBUFFER_SIZE, vk::BufferUsageFlagBits::eTransferSrc, 
			vk::MemoryPropertyFlags(vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent) );
	}
	MappedBufferWrapper* transferBuffer;
	if ( V_MAX_STAGINGBUFFER_SIZE < size ) {
		printf ( "Stagingbuffer not big enough -> create template\n" );
		transferBuffer = new MappedBufferWrapper ( size, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlags(vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent) );
	}else{
		transferBuffer = stagingBuffer;
	}
	memcpy ( transferBuffer->data, srcData, size );
	copyBuffer ( transferCommandPool, transferBuffer->buffer, targetBuffer, 0, offset, size,
		vk::PipelineStageFlagBits::eHost, vk::AccessFlagBits::eHostWrite, usePipelineFlags, useFlags);

	if ( V_MAX_STAGINGBUFFER_SIZE < size ) {
		delete transferBuffer;
	}

}
