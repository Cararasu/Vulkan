
#include "VGlobal.h"
#include "VHeader.h"

vk::CommandPool singleImageTransitionCommandPool = vk::CommandPool();


void createImage(vk::Extent3D size, vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags usage, vk::MemoryPropertyFlags needed, vk::MemoryPropertyFlags preferred, vk::Image* image, vk::DeviceMemory* imageMemory){
	
	vk::ImageCreateInfo imageInfo;
	
	if(size.height != 1)
		if(size.depth != 1)
			imageInfo.imageType = vk::ImageType::e3D;
		else
			imageInfo.imageType = vk::ImageType::e2D;
	else
		imageInfo.imageType = vk::ImageType::e1D;
	
	imageInfo.extent.width = size.width;
	imageInfo.extent.height = size.height;
	imageInfo.extent.depth = size.depth;
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;
	imageInfo.format = format;
	imageInfo.tiling = tiling;
	imageInfo.initialLayout = vk::ImageLayout::eUndefined;
	imageInfo.usage = usage;
	imageInfo.samples = vk::SampleCountFlagBits::e1;
	imageInfo.sharingMode = vk::SharingMode::eExclusive;

	V_CHECKCALL(vGlobal.deviceWrapper.device.createImage(&imageInfo, nullptr, image), printf("Failed To Create Image\n"));

	printf("Create Image of dimensions %dx%dx%d\n", size.width, size.height, size.depth);
	
	*imageMemory = allocateImageMemory(*image, needed, preferred);
	
	vkBindImageMemory(vGlobal.deviceWrapper.device, *image, *imageMemory, 0);
}
void destroyImage(vk::Image image, vk::DeviceMemory imageMemory){
	vkDestroyImage(vGlobal.deviceWrapper.device, image, nullptr);
	vkFreeMemory(vGlobal.deviceWrapper.device, imageMemory, nullptr);
}
vk::ImageView createImageView2D(vk::Image image, vk::Format format, vk::ImageAspectFlags aspectFlags) {
   
	vk::ImageViewCreateInfo imageViewCreateInfo(
		vk::ImageViewCreateFlags(),
		image, 
		vk::ImageViewType::e2D, format, 
		vk::ComponentMapping(vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eIdentity),
		vk::ImageSubresourceRange(aspectFlags, 0, 1, 0, 1)
		);
	
	vk::ImageView imageView;
	
	V_CHECKCALL(vGlobal.deviceWrapper.device.createImageView(&imageViewCreateInfo, nullptr, &imageView), printf("Creation of ImageView failed\n"));
	
	return imageView;
}
void copyBufferToImage(vk::Buffer buffer, vk::Image image, vk::Offset3D offset, vk::Extent3D extent) {
	if(!singleTransferCommandPool){
		VTQueue* queue = vGlobal.deviceWrapper.requestTransferQueue();
		if(queue)
			singleTransferCommandPool = createCommandPool(queue->transferQId, vk::CommandPoolCreateFlags(vk::CommandPoolCreateFlagBits::eTransient) );
		else
			singleTransferCommandPool = createCommandPool(vGlobal.deviceWrapper.getPGCQueue()->graphicsQId, vk::CommandPoolCreateFlags(vk::CommandPoolCreateFlagBits::eTransient) );
	}
	
    vk::CommandBuffer commandBuffer = createCommandBuffer(singleTransferCommandPool, vk::CommandBufferLevel::ePrimary);

    vk::CommandBufferBeginInfo beginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
	commandBuffer.begin(beginInfo);
	
	vk::BufferImageCopy region(
		0,0,0,//buffer- Offset, -RowLength, -ImageHeight
		vk::ImageSubresourceLayers(vk::ImageAspectFlagBits::eColor, 0, 0, 1),
		offset, extent
	);
	commandBuffer.copyBufferToImage(buffer, image, vk::ImageLayout::eTransferDstOptimal, 1, &region);
	
	commandBuffer.end();
	
	vk::SubmitInfo submitInfo(
		0, nullptr,//waitsemaphores
		nullptr,//pWaitDstStageMask
		1, &commandBuffer,
		0, nullptr//signalsemaphores
		);

	VTQueue* vtQueue = vGlobal.deviceWrapper.requestTransferQueue();
	if(vtQueue){
		vtQueue->submitTransfer(1,&submitInfo);
		vtQueue->waitForFinish();
	}else{
		vGlobal.deviceWrapper.getPGCQueue()->submitGraphics(1,&submitInfo);
		vGlobal.deviceWrapper.getPGCQueue()->waitForFinish();
	}
	deleteCommandBuffer(singleTransferCommandPool, commandBuffer);
}
void transitionImageLayout(vk::Image image, vk::Format format, vk::ImageLayout oldLayout, vk::ImageLayout newLayout, vk::ImageAspectFlags aspectMask) {
	if(!singleImageTransitionCommandPool){
		singleImageTransitionCommandPool = createCommandPool(vGlobal.deviceWrapper.getPGCQueue()->graphicsQId, vk::CommandPoolCreateFlags(vk::CommandPoolCreateFlagBits::eTransient) );
	}
	
    vk::CommandBuffer commandBuffer = createCommandBuffer(singleImageTransitionCommandPool, vk::CommandBufferLevel::ePrimary);
	
	commandBuffer.begin(vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit));
	
	vk::PipelineStageFlags sourceStage;
	vk::PipelineStageFlags destinationStage;

	vk::ImageMemoryBarrier barrier(
		vk::AccessFlags(), vk::AccessFlags(),
		oldLayout, newLayout,
		vGlobal.deviceWrapper.graphQId, vGlobal.deviceWrapper.graphQId,
		image,
		vk::ImageSubresourceRange(aspectMask, 0, 1, 0, 1)
	);
	
	if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eTransferDstOptimal) {
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;

		sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
		destinationStage = vk::PipelineStageFlagBits::eTransfer;
	} else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
		barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
		barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

		sourceStage = vk::PipelineStageFlagBits::eTransfer;
		destinationStage = vk::PipelineStageFlagBits::eFragmentShader;
	} else if (oldLayout == vk::ImageLayout::eUndefined && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
		barrier.srcAccessMask = vk::AccessFlags();
		barrier.dstAccessMask = vk::AccessFlags(vk::AccessFlagBits::eDepthStencilAttachmentRead | k::AccessFlagBits::eDepthStencilAttachmentWrite);
		
		sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
		destinationStage = vk::PipelineStageFlagBits::eEarlyFragmentTests;
	} else {
		assert(false);
	}

	vkCmdPipelineBarrier(
		commandBuffer,
		sourceStage, destinationStage,
		0,
		0, nullptr,
		0, nullptr,
		1, &barrier
	);
	commandBuffer.end();
	vk::SubmitInfo submitInfo(
		0, nullptr,//waitsemaphores
		nullptr,//pWaitDstStageMask
		1, &commandBuffer,
		0, nullptr//signalsemaphores
		);

	vGlobal.deviceWrapper.getPGCQueue()->submitGraphics(1,&submitInfo, VK_NULL_HANDLE);
	vGlobal.deviceWrapper.getPGCQueue()->waitForFinish();
	deleteCommandBuffer(singleImageTransitionCommandPool, commandBuffer);
}
void transferData(const void* srcData, vk::Image targetImage, vk::DeviceSize size, vk::Offset3D offset, vk::Extent3D extent){
	if(stagingBuffer){
		if(stagingBuffer->bufferSize < size){
			printf("Recreate New Staging-Buffer\n");
			delete stagingBuffer;
			stagingBuffer = new MappedBufferWrapper(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		}
	}else{
		printf("Create Staging-Buffer\n");
		stagingBuffer = new MappedBufferWrapper(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	}
	memcpy(stagingBuffer->data, srcData, size);
	copyBufferToImage(stagingBuffer->buffer, targetImage, offset, extent);
	
	if(size > V_MAX_STAGINGBUFFER_SIZE){
		printf("Stagingbuffer too big -> delete\n");
		delete stagingBuffer;
		stagingBuffer = nullptr;
	}
}