
#include "VGlobal.h"
#include "VHeader.h"

vk::CommandPool singleImageTransitionCommandPool = vk::CommandPool();


void createImage(vk::Extent3D size, vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags usage, vk::MemoryPropertyFlags needed, vk::MemoryPropertyFlags recommended, vk::Image* image, vk::DeviceMemory* imageMemory){
	
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

	V_CHECKCALL(global.deviceWrapper.device.createImage(&imageInfo, nullptr, image), printf("Failed To Create Image\n"));

	printf("Create Image of dimensions %dx%dx%d\n", size.width, size.height, size.depth);
	
	vk::MemoryRequirements memRequirements;
	global.deviceWrapper.device.getImageMemoryRequirements(*image, &memRequirements);
	*imageMemory = allocateMemory(memRequirements, needed | recommended);
	if(!*imageMemory)
		*imageMemory = allocateMemory(memRequirements, needed);
	
	vkBindImageMemory(global.deviceWrapper.device, *image, *imageMemory, 0);
}
void destroyImage(vk::Image image, vk::DeviceMemory imageMemory){
	vkDestroyImage(global.deviceWrapper.device, image, nullptr);
	vkFreeMemory(global.deviceWrapper.device, imageMemory, nullptr);
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
	
	V_CHECKCALL(global.deviceWrapper.device.createImageView(&imageViewCreateInfo, nullptr, &imageView), printf("Creation of ImageView failed\n"));
	
	return imageView;
}
void copyBufferToImage(vk::CommandPool commandPool, vk::Buffer buffer, vk::Image image, vk::Offset3D offset, vk::Extent3D extent) {

    vk::CommandBuffer commandBuffer = createCommandBuffer(commandPool, vk::CommandBufferLevel::ePrimary);

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

	VTQueue* vtQueue = global.deviceWrapper.requestTransferQueue();
	if(vtQueue){
		vtQueue->submitTransfer(1,&submitInfo);
		vtQueue->waitForFinish();
	}else{
		global.deviceWrapper.getPGCQueue()->submitGraphics(1,&submitInfo);
		global.deviceWrapper.getPGCQueue()->waitForFinish();
	}
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
void transferData(vk::CommandPool commandPool, const void* srcData, vk::Image targetImage, vk::DeviceSize size, vk::Offset3D offset, vk::Extent3D extent){
	if(stagingBuffer){
		if(stagingBuffer->bufferSize < size){
			printf("Recreate New Staging-Buffer\n");
			delete stagingBuffer;
			stagingBuffer = new MappedBufferWrapper(size, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlags(vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent));
		}
	}else{
		printf("Create Staging-Buffer\n");
		stagingBuffer = new MappedBufferWrapper(size, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlags(vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent));
	}
	memcpy(stagingBuffer->data, srcData, size);
	copyBufferToImage(commandPool, stagingBuffer->buffer, targetImage, offset, extent);
	
	if(size > V_MAX_STAGINGBUFFER_SIZE){
		printf("Stagingbuffer too big -> delete\n");
		delete stagingBuffer;
		stagingBuffer = nullptr;
	}
}