
#include "VHeader.h"
#include "VGlobal.h"
#include "DataWrapper.h"

vk::CommandPool singleTransferCommandPool = vk::CommandPool();
vk::CommandPool transferCommandBuffer = vk::CommandPool();
vk::CommandBuffer singleTransferCommandBuffer = vk::CommandBuffer();
vk::CommandBuffer singleImageTransitionBuffer = vk::CommandBuffer();



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

void copyBuffer ( vk::Buffer srcBuffer, vk::Buffer dstBuffer, vk::DeviceSize srcOffset, vk::DeviceSize dstOffset, vk::DeviceSize size,
		vk::PipelineStageFlags inputPipelineStageFlags, vk::AccessFlags inputAccessFlag, vk::PipelineStageFlags outputPipelineStageFlags, vk::AccessFlags outputAccessFlag) {

	if ( !singleTransferCommandPool ) {
		VTQueue* queue = vGlobal.deviceWrapper.requestTransferQueue();
		if ( queue )
			singleTransferCommandPool = createCommandPool ( queue->transferQId, vk::CommandPoolCreateFlags(vk::CommandPoolCreateFlagBits::eTransient) );
		else
			singleTransferCommandPool = createCommandPool ( vGlobal.deviceWrapper.getPGCQueue()->graphicsQId, vk::CommandPoolCreateFlags(vk::CommandPoolCreateFlagBits::eTransient)  );
	}

	vk::CommandBuffer commandBuffer = createCommandBuffer ( singleTransferCommandPool, vk::CommandBufferLevel::ePrimary );

	vk::CommandBufferBeginInfo beginInfo(vk::CommandBufferUsageFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit));

	commandBuffer.begin(beginInfo);

	vk::BufferCopy copyRegion = {};
	copyRegion.srcOffset = srcOffset; // Optional
	copyRegion.dstOffset = dstOffset; // Optional
	copyRegion.size = size;
	
	vk::BufferMemoryBarrier bufferMemoryBarriers[] = {
		vk::BufferMemoryBarrier(
			inputAccessFlag, vk::AccessFlagBits::eTransferRead,
			vGlobal.deviceWrapper.transfQId, vGlobal.deviceWrapper.transfQId,
			srcBuffer,
			srcOffset, size
		)
	};

	commandBuffer.pipelineBarrier(
		inputPipelineStageFlags, vk::PipelineStageFlagBits::eTransfer,
		vk::DependencyFlagBits::eByRegion,
		0, nullptr,
		1, bufferMemoryBarriers,
		0, nullptr
	);
	commandBuffer.copyBuffer(srcBuffer, dstBuffer, 1, &copyRegion);

	
	bufferMemoryBarriers[0] = vk::BufferMemoryBarrier(
			vk::AccessFlagBits::eTransferWrite, outputAccessFlag,
			vGlobal.deviceWrapper.transfQId, vGlobal.deviceWrapper.transfQId,
			srcBuffer,
			srcOffset, size
		);

	commandBuffer.pipelineBarrier(
		vk::PipelineStageFlagBits::eTransfer, outputPipelineStageFlags,
		vk::DependencyFlagBits::eByRegion,
		0, nullptr,
		1, bufferMemoryBarriers,
		0, nullptr
	);

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
	deleteCommandBuffer ( singleTransferCommandPool, commandBuffer );
}

void transferData ( const void* srcData, vk::Buffer targetBuffer, vk::DeviceSize offset, vk::DeviceSize size, vk::PipelineStageFlags usePipelineFlags, vk::AccessFlags useFlags) {
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
	copyBuffer ( transferBuffer->buffer, targetBuffer, 0, offset, size,
		vk::PipelineStageFlagBits::eHost, vk::AccessFlagBits::eHostWrite, usePipelineFlags, useFlags);

	if ( V_MAX_STAGINGBUFFER_SIZE < size ) {
		delete transferBuffer;
	}

}
