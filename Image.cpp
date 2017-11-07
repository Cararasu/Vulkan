
#include "VGlobal.h"
#include "VHeader.h"

VkCommandPool singleImageTransitionCommandPool = VK_NULL_HANDLE;


void createImage(VkExtent3D size, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags needed, VkMemoryPropertyFlags recommended,  VkImage* image, VkDeviceMemory* imageMemory){
	
	VkImageCreateInfo imageInfo = {};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	
	if(size.height != 1)
		if(size.depth != 1)
			imageInfo.imageType = VK_IMAGE_TYPE_3D;
		else
			imageInfo.imageType = VK_IMAGE_TYPE_2D;
	else
		imageInfo.imageType = VK_IMAGE_TYPE_1D;
	
	imageInfo.extent.width = size.width;
	imageInfo.extent.height = size.height;
	imageInfo.extent.depth = size.depth;
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;
	imageInfo.format = format;
	imageInfo.tiling = tiling;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.usage = usage;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VCHECKCALL(vkCreateImage(vGlobal.deviceWrapper.device, &imageInfo, nullptr, image), printf("Failed To Create Image\n"));

	printf("Create Image of dimensions %dx%dx%d\n", size.width, size.height, size.depth);
	
	*imageMemory = allocateImageMemory(*image, needed, recommended);
	
	vkBindImageMemory(vGlobal.deviceWrapper.device, *image, *imageMemory, 0);
}
void destroyImage(VkImage image, VkDeviceMemory imageMemory){
	vkDestroyImage(vGlobal.deviceWrapper.device, image, nullptr);
	vkFreeMemory(vGlobal.deviceWrapper.device, imageMemory, nullptr);
}
VkImageView createImageView2D(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags) {
   
	VkImageViewCreateInfo imageViewCreateInfo = {};
	imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	imageViewCreateInfo.image = image;
	imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	imageViewCreateInfo.format = format;
	imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
	imageViewCreateInfo.subresourceRange.aspectMask = aspectFlags;
	imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
	imageViewCreateInfo.subresourceRange.levelCount = 1;
	imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
	imageViewCreateInfo.subresourceRange.layerCount = 1;
	
	VkImageView imageView;
	
	VCHECKCALL(vkCreateImageView(vGlobal.deviceWrapper.device, &imageViewCreateInfo, nullptr, &imageView), {
		printf("Creation of ImageView failed\n");
	});
	return imageView;
}
void copyBufferToImage(VkBuffer buffer, VkImage image, VkOffset3D offset, VkExtent3D extent) {
	if(singleTransferCommandPool == VK_NULL_HANDLE){
		VTQueue* queue = vGlobal.deviceWrapper.requestTransferQueue();
		if(queue)
			singleTransferCommandPool = createCommandPool(queue->transferQId, vk::CommandPoolCreateFlags(vk::CommandPoolCreateFlagBits::eTransient) );
		else
			singleTransferCommandPool = createCommandPool(vGlobal.deviceWrapper.getPGCQueue()->graphicsQId, vk::CommandPoolCreateFlags(vk::CommandPoolCreateFlagBits::eTransient) );
	}
	
    VkCommandBuffer commandBuffer = createCommandBuffer(singleTransferCommandPool, vk::CommandBufferLevel::ePrimary);

    VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	vkBeginCommandBuffer(commandBuffer, &beginInfo);
	
	VkBufferImageCopy region = {};
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;

	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;

	region.imageOffset = offset;
	region.imageExtent = extent;
	vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
	
	vkEndCommandBuffer(commandBuffer);
	
	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	VTQueue* vtQueue = vGlobal.deviceWrapper.requestTransferQueue();
	if(vtQueue){
		vtQueue->submitTransfer(1,&submitInfo, VK_NULL_HANDLE);
		vtQueue->waitForFinish();
	}else{
		vGlobal.deviceWrapper.getPGCQueue()->submitGraphics(1,&submitInfo, VK_NULL_HANDLE);
		vGlobal.deviceWrapper.getPGCQueue()->waitForFinish();
	}
	deleteCommandBuffer(singleTransferCommandPool, commandBuffer);
}
void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, VkImageAspectFlags aspectMask) {
	if(singleImageTransitionCommandPool == VK_NULL_HANDLE){
		singleImageTransitionCommandPool = createCommandPool(vGlobal.deviceWrapper.getPGCQueue()->graphicsQId, vk::CommandPoolCreateFlags(vk::CommandPoolCreateFlagBits::eTransient) );
	}
	
    VkCommandBuffer commandBuffer = createCommandBuffer(singleImageTransitionCommandPool, vk::CommandBufferLevel::ePrimary);
	
    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(commandBuffer, &beginInfo);
	
	VkPipelineStageFlags sourceStage;
	VkPipelineStageFlags destinationStage;

	VkImageMemoryBarrier barrier = {};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = oldLayout;
	barrier.newLayout = newLayout;
	
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	
	barrier.image = image;
	barrier.subresourceRange.aspectMask = aspectMask;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;
	
	if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	} else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	} else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		
		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
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
	
	vkEndCommandBuffer(commandBuffer);
	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	vGlobal.deviceWrapper.getPGCQueue()->submitGraphics(1,&submitInfo, VK_NULL_HANDLE);
	vGlobal.deviceWrapper.getPGCQueue()->waitForFinish();
	deleteCommandBuffer(singleImageTransitionCommandPool, commandBuffer);
}
void transferData(const void* srcData, VkImage targetImage, VkDeviceSize size, VkOffset3D offset, VkExtent3D extent){
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