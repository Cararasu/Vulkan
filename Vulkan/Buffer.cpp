
#include "VHeader.h"
#include "VGlobal.h"
#include "DataWrapper.h"


void VInstance::createBuffer ( vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags needed, vk::MemoryPropertyFlags recommended, vk::Buffer* buffer, vk::DeviceMemory* bufferMemory ) {
	vk::BufferCreateInfo bufferInfo(vk::BufferCreateFlags(), size, usage, vk::SharingMode::eExclusive);

	V_CHECKCALL ( device.createBuffer(&bufferInfo, nullptr, buffer), printf ( "Failed To Create Buffer\n" ) );

	printf ( "Create Buffer of size %llu\n", size );
	
	vk::MemoryRequirements memRequirements;
	device.getBufferMemoryRequirements(*buffer, &memRequirements);
	*bufferMemory = allocateMemory(memRequirements, needed | recommended);
	if(!*bufferMemory)
		*bufferMemory = allocateMemory(memRequirements, needed);

	vkBindBufferMemory ( device, *buffer, *bufferMemory, 0 );
}
void VInstance::destroyBuffer ( vk::Buffer buffer, vk::DeviceMemory bufferMemory ) {
	vkDestroyBuffer ( device, buffer, nullptr );
	vkFreeMemory ( device, bufferMemory, nullptr );
}

void copyBuffer (  BufferWrapper* srcBuffer, BufferWrapper* dstBuffer, vk::DeviceSize srcOffset, vk::DeviceSize dstOffset, vk::DeviceSize size,
		vk::PipelineStageFlags inputPipelineStageFlags, vk::AccessFlags inputAccessFlag, vk::PipelineStageFlags outputPipelineStageFlags, vk::AccessFlags outputAccessFlag,
		vk::CommandPool commandPool, vk::Queue submitQueue) {
	assert(srcBuffer->instance == dstBuffer->instance);
	
	vk::CommandBuffer commandBuffer =  srcBuffer->instance->createCommandBuffer ( commandPool, vk::CommandBufferLevel::ePrimary );
	
	commandBuffer.begin(vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit)));
	
	copyBuffer(srcBuffer, dstBuffer, srcOffset, dstOffset, size, inputPipelineStageFlags, inputAccessFlag, outputPipelineStageFlags, outputAccessFlag, commandBuffer);

	commandBuffer.end();

	vk::SubmitInfo submitInfo(
		0, nullptr,//waitsemaphores
		nullptr,//pWaitDstStageMask
		1, &commandBuffer,
		0, nullptr//signalsemaphores
		);
	submitQueue.submit({submitInfo}, vk::Fence());
}
void copyBuffer(BufferWrapper* srcBuffer, BufferWrapper* dstBuffer, vk::DeviceSize srcOffset, vk::DeviceSize dstOffset, vk::DeviceSize size,
	vk::PipelineStageFlags inputPipelineStageFlags, vk::AccessFlags inputAccessFlag, vk::PipelineStageFlags outputPipelineStageFlags, vk::AccessFlags outputAccessFlag,
	vk::CommandBuffer commandBuffer){
	assert(srcBuffer->instance == dstBuffer->instance);
	
	VInstance* instance = srcBuffer->instance;
	
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
					instance->transfQId, instance->transfQId,
					srcBuffer->buffer,
					srcOffset, size
				)
			},
			{}
		);
	}
	commandBuffer.copyBuffer(srcBuffer->buffer, dstBuffer->buffer, 1, &copyRegion);

	if(outputAccessFlag && outputPipelineStageFlags) {
		commandBuffer.pipelineBarrier(
			vk::PipelineStageFlagBits::eTransfer, outputPipelineStageFlags,
			vk::DependencyFlagBits::eByRegion,
			{},
			{
				vk::BufferMemoryBarrier(
					vk::AccessFlagBits::eTransferWrite, outputAccessFlag,
					instance->transfQId, instance->transfQId,
					srcBuffer->buffer,
					srcOffset, size
				)
			},
			{}
		);
	}
}
void transferData ( const void* srcData, BufferWrapper* targetBuffer, vk::DeviceSize offset, vk::DeviceSize size, vk::PipelineStageFlags usePipelineFlags, vk::AccessFlags useFlags, vk::CommandBuffer commandBuffer) {
	MappedBufferWrapper* transferBuffer;
	if ( V_MAX_STAGINGBUFFER_SIZE < size ) {
		printf ( "Stagingbuffer not big enough -> create template\n" );
		transferBuffer = new MappedBufferWrapper ( targetBuffer->instance, size, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlags(vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent) );
	}else{	
		if ( !stagingBuffer ) {
			printf ( "Creating Staging-Buffer\n" );
			stagingBuffer = new MappedBufferWrapper ( targetBuffer->instance, V_MAX_STAGINGBUFFER_SIZE, vk::BufferUsageFlagBits::eTransferSrc, 
				vk::MemoryPropertyFlags(vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eDeviceLocal) );
		}
		transferBuffer = stagingBuffer;
	}
	memcpy ( transferBuffer->data, srcData, size );
	copyBuffer ( transferBuffer, targetBuffer, 0, offset, size,
		vk::PipelineStageFlagBits::eHost, vk::AccessFlagBits::eHostWrite, usePipelineFlags, useFlags, commandBuffer);

	if ( V_MAX_STAGINGBUFFER_SIZE < size ) {
		delete transferBuffer;
	}
}

void transferData ( const void* srcData, BufferWrapper* targetBuffer, vk::DeviceSize offset, vk::DeviceSize size, vk::PipelineStageFlags usePipelineFlags, vk::AccessFlags useFlags,
		vk::CommandPool commandPool, vk::Queue submitQueue) {

	vk::CommandBuffer commandBuffer =  targetBuffer->instance->createCommandBuffer ( commandPool, vk::CommandBufferLevel::ePrimary );
	
	commandBuffer.begin(vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit)));
	
	transferData(srcData, targetBuffer, offset, size, usePipelineFlags, useFlags, commandBuffer);

	commandBuffer.end();

	vk::SubmitInfo submitInfo(
		0, nullptr,//waitsemaphores
		nullptr,//pWaitDstStageMask
		1, &commandBuffer,
		0, nullptr//signalsemaphores
		);
	submitQueue.submit({submitInfo}, vk::Fence());
}
