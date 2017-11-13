
#include "VGlobal.h"
#include "VHeader.h"

vk::CommandPool singleImageTransitionCommandPool = vk::CommandPool();

vk::ImageView createImageView2D(vk::Image image, vk::Format format, vk::ImageAspectFlags aspectFlags) {
   
	vk::ImageViewCreateInfo imageViewCreateInfo(
		vk::ImageViewCreateFlags(),
		image, 
		vk::ImageViewType::e2D, format, 
		vk::ComponentMapping(vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eIdentity),
		vk::ImageSubresourceRange(aspectFlags, 0, 1, 0, 1)
		);
	
	vk::ImageView imageView;
	
	V_CHECKCALL(global.deviceWrapper.device.createImageView(&imageViewCreateInfo, nullptr, &imageView), printf("Creation of ImageView failed\n"));
	
	return imageView;
}
vk::ImageView createImageView2DArray(vk::Image image, uint32_t offset, uint32_t size, vk::Format format, vk::ImageAspectFlags aspectFlags) {
	
	vk::ImageViewCreateInfo imageViewCreateInfo(
		vk::ImageViewCreateFlags(),
		image, 
		vk::ImageViewType::e2DArray, format, 
		vk::ComponentMapping(vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eIdentity),
		vk::ImageSubresourceRange(aspectFlags, 0, 1, offset, size)
		);
	
	vk::ImageView imageView;
	
	V_CHECKCALL(global.deviceWrapper.device.createImageView(&imageViewCreateInfo, nullptr, &imageView), printf("Creation of ImageView failed\n"));
	
	return imageView;
}
void copyBufferToImage(vk::Buffer srcBuffer, vk::Image dstImage, vk::DeviceSize srcOffset, vk::Offset3D dstOffset, vk::Extent3D extent, uint32_t index,
	vk::PipelineStageFlags inputPipelineStageFlags, vk::AccessFlags inputAccessFlag, vk::PipelineStageFlags outputPipelineStageFlags, vk::AccessFlags outputAccessFlag,
	vk::CommandPool commandPool, vk::Queue submitQueue){
	
	
    vk::CommandBuffer commandBuffer = createCommandBuffer(commandPool, vk::CommandBufferLevel::ePrimary);

    vk::CommandBufferBeginInfo beginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
	commandBuffer.begin(beginInfo);
	
	if(inputAccessFlag && inputPipelineStageFlags) {
		commandBuffer.pipelineBarrier(
			inputPipelineStageFlags, vk::PipelineStageFlagBits::eTransfer,
			vk::DependencyFlags(),
			{},
			{
				vk::BufferMemoryBarrier(
					inputAccessFlag, vk::AccessFlagBits::eTransferRead,
					global.deviceWrapper.transfQId, global.deviceWrapper.transfQId,
					srcBuffer,
					0, 0
				)
			},
			{}
		);
	}
	vk::BufferImageCopy region(
		srcOffset,0,0,//buffer -Offset, -RowLength, -ImageHeight
		vk::ImageSubresourceLayers(vk::ImageAspectFlagBits::eColor, 0, index, 1),
		dstOffset, extent);
		
	commandBuffer.copyBufferToImage(srcBuffer, dstImage, vk::ImageLayout::eTransferDstOptimal, 1, &region);
	
	if(outputAccessFlag && outputPipelineStageFlags) {
		commandBuffer.pipelineBarrier(
			vk::PipelineStageFlagBits::eTransfer, outputPipelineStageFlags,
			vk::DependencyFlags(),
			{},
			{
				vk::BufferMemoryBarrier(
					vk::AccessFlagBits::eTransferWrite, outputAccessFlag,
					global.deviceWrapper.transfQId, global.deviceWrapper.transfQId,
					srcBuffer,
					0, 0
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

	submitQueue.submit({submitInfo}, vk::Fence());
}
void transitionImageLayout(vk::Image image, vk::Format format, vk::ImageLayout oldLayout, vk::ImageLayout newLayout, vk::ImageAspectFlags aspectMask,
		vk::CommandPool commandPool, vk::Queue submitQueue) {

    vk::CommandBuffer commandBuffer = createCommandBuffer(commandPool, vk::CommandBufferLevel::ePrimary);
	
	commandBuffer.begin(vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit));
	
	vk::AccessFlags srcAccessMask, dstAccessMask;
	vk::PipelineStageFlags sourceStage, destinationStage;
	
	if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eTransferDstOptimal) {
		srcAccessMask = vk::AccessFlags();
		dstAccessMask = vk::AccessFlagBits::eTransferWrite;

		sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
		destinationStage = vk::PipelineStageFlagBits::eTransfer;
	} else if (oldLayout == vk::ImageLayout::eTransferDstOptimal && newLayout == vk::ImageLayout::eShaderReadOnlyOptimal) {
		srcAccessMask = vk::AccessFlagBits::eTransferWrite;
		dstAccessMask = vk::AccessFlagBits::eShaderRead;

		sourceStage = vk::PipelineStageFlagBits::eTransfer;
		destinationStage = vk::PipelineStageFlagBits::eFragmentShader;
	} else if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eDepthStencilAttachmentOptimal) {
		srcAccessMask = vk::AccessFlags();
		dstAccessMask = vk::AccessFlags(vk::AccessFlagBits::eDepthStencilAttachmentRead | vk::AccessFlagBits::eDepthStencilAttachmentWrite);
		
		sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
		destinationStage = vk::PipelineStageFlagBits::eEarlyFragmentTests;
	} else {
		assert(false);
	}
	
	commandBuffer.pipelineBarrier(
		sourceStage, destinationStage,
		vk::DependencyFlags(),
		{},//memoryBarriers
		{},//bufferBarriers
		{
			vk::ImageMemoryBarrier(
				srcAccessMask, dstAccessMask,
				oldLayout, newLayout,
				global.deviceWrapper.graphQId, global.deviceWrapper.graphQId,
				image,
				vk::ImageSubresourceRange(aspectMask, 0, 1, 0, 1)
			)
		}//imageBarriers
	);
	
	commandBuffer.end();
	vk::SubmitInfo submitInfo(
		0, nullptr,//waitSemaphores
		nullptr,//pWaitDstStageMask
		1, &commandBuffer,
		0, nullptr//signalsemaphores
		);
	submitQueue.submit({submitInfo}, vk::Fence());
}
void transferData(const void* srcData, vk::Image targetImage, vk::Offset3D offset, vk::Extent3D extent, uint32_t index, vk::DeviceSize size, vk::PipelineStageFlags usePipelineFlags, vk::AccessFlags useFlag,
	vk::CommandPool commandPool, vk::Queue submitQueue){
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
	
	memcpy(transferBuffer->data, srcData, size);
	copyBufferToImage(transferBuffer->buffer, targetImage, 0, offset, extent, index,
		vk::PipelineStageFlagBits::eHost, vk::AccessFlagBits::eHostWrite, usePipelineFlags, useFlag, commandPool, submitQueue);
	
	if ( V_MAX_STAGINGBUFFER_SIZE < size ) {
		submitQueue.waitIdle();
		delete transferBuffer;
	}
}