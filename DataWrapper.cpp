#include "DataWrapper.h"
#include "VGlobal.h"


MappedBufferWrapper* stagingBuffer = nullptr;

BufferWrapper::BufferWrapper(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags needed, vk::MemoryPropertyFlags recommended): 
	bufferSize(size), buffer(vk::Buffer()), backedMemory(vk::DeviceMemory()){
	createBuffer(size, usage, needed, recommended, &buffer, &backedMemory);
}
BufferWrapper::~BufferWrapper(){
	destroyBuffer(buffer, backedMemory);
}
MappedBufferWrapper::MappedBufferWrapper(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags needed, vk::MemoryPropertyFlags recommended): 
	BufferWrapper(size, usage, needed){
	printf("Map %d Bytes of Memory\n", bufferSize);
	vkMapMemory(global.deviceWrapper.device, backedMemory, 0, bufferSize, 0, &data);
}
MappedBufferWrapper::~MappedBufferWrapper(){
	printf("Unmap %d Bytes of Memory\n", bufferSize);
	vkUnmapMemory(global.deviceWrapper.device, backedMemory);
}

ImageWrapper::ImageWrapper(vk::Extent3D extent, uint32_t arraySize, vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags usage, vk::ImageAspectFlags aspectFlags, vk::MemoryPropertyFlags needed, vk::MemoryPropertyFlags recommended): 
	image(), backedMemory(), extent(extent), arraySize(arraySize), format(format), tiling(tiling), usage(usage), type(), layout(vk::ImageLayout::eUndefined), aspectFlags(aspectFlags){
	if(extent.height == 1)
		type = vk::ImageType::e1D;
	else if(extent.depth == 1)
		type = vk::ImageType::e2D;
	else
		type = vk::ImageType::e3D;
		
	vk::ImageCreateInfo imageInfo(vk::ImageCreateFlags(), type, format, extent, 1, arraySize, vk::SampleCountFlagBits::e1, tiling, usage, vk::SharingMode::eExclusive, 0, nullptr, layout);

	V_CHECKCALL(global.deviceWrapper.device.createImage(&imageInfo, nullptr, &image), printf("Failed To Create Image\n"));

	printf("Create Image of dimensions %dx%dx%d\n", extent.width, extent.height, extent.depth);
	
	vk::MemoryRequirements memRequirements;
	global.deviceWrapper.device.getImageMemoryRequirements(image, &memRequirements);
	backedMemory = allocateMemory(memRequirements, needed | recommended);
	if(!backedMemory)
		backedMemory = allocateMemory(memRequirements, needed);
	
	vkBindImageMemory(global.deviceWrapper.device, image, backedMemory, 0);
}
ImageWrapper::~ImageWrapper(){
	vkDestroyImage(global.deviceWrapper.device, image, nullptr);
	vkFreeMemory(global.deviceWrapper.device, backedMemory, nullptr);
}


void ImageWrapper::transitionImageLayout(vk::ImageLayout newLayout, vk::CommandPool commandPool, vk::Queue submitQueue){
	
    vk::CommandBuffer commandBuffer = createCommandBuffer(commandPool, vk::CommandBufferLevel::ePrimary);
	
	commandBuffer.begin(vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit));
	
	vk::AccessFlags srcAccessMask, dstAccessMask;
	vk::PipelineStageFlags sourceStage, destinationStage;
	
	if (layout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eTransferDstOptimal) {
		srcAccessMask = vk::AccessFlags();
		dstAccessMask = vk::AccessFlagBits::eTransferWrite;

		sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
		destinationStage = vk::PipelineStageFlagBits::eTransfer;
	} else if (layout == vk::ImageLayout::eTransferDstOptimal && newLayout == vk::ImageLayout::eShaderReadOnlyOptimal) {
		srcAccessMask = vk::AccessFlagBits::eTransferWrite;
		dstAccessMask = vk::AccessFlagBits::eShaderRead;

		sourceStage = vk::PipelineStageFlagBits::eTransfer;
		destinationStage = vk::PipelineStageFlagBits::eFragmentShader;
	} else if (layout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eDepthStencilAttachmentOptimal) {
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
				layout, newLayout,
				global.deviceWrapper.graphQId, global.deviceWrapper.graphQId,
				image,
				vk::ImageSubresourceRange(aspectFlags, 0, 1, 0, arraySize)
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
	layout = newLayout;
}