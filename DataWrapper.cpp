#include "DataWrapper.h"
#include "VGlobal.h"


MappedBufferWrapper* stagingBuffer = nullptr;

BufferWrapper::BufferWrapper (VInstance* instance, vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags needed, vk::MemoryPropertyFlags recommended) :
	bufferSize (size), buffer (vk::Buffer()), backedMemory (vk::DeviceMemory()) {
	instance->createBuffer (size, usage, needed, recommended, &buffer, &backedMemory);
}
void BufferWrapper::destroy(VInstance* instance){
	instance->destroyBuffer (buffer, backedMemory);
}
BufferWrapper::~BufferWrapper() {
}
MappedBufferWrapper::MappedBufferWrapper (VInstance* instance, vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags needed, vk::MemoryPropertyFlags recommended) :
	BufferWrapper (instance, size, usage, needed) {
	printf ("Map %d Bytes of Memory\n", bufferSize);
	vkMapMemory (instance->device, backedMemory, 0, bufferSize, 0, &data);
}
void MappedBufferWrapper::destroy(VInstance* instance){
	printf ("Unmap %d Bytes of Memory\n", bufferSize);
	vkUnmapMemory (instance->device, backedMemory);
}
MappedBufferWrapper::~MappedBufferWrapper() {
}

ImageWrapper::ImageWrapper (VInstance* instance, vk::Extent3D extent, uint32_t mipMapLevels, uint32_t arraySize, vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags usage, vk::ImageAspectFlags aspectFlags, vk::MemoryPropertyFlags needed, vk::MemoryPropertyFlags recommended) :
	instance(instance), image(), backedMemory(), extent (extent), mipMapLevels (mipMapLevels), arraySize (arraySize), format (format), tiling (tiling), usage (usage), type(), layout (vk::ImageLayout::eUndefined), aspectFlags (aspectFlags) {
	if (extent.height == 1)
		type = vk::ImageType::e1D;
	else if (extent.depth == 1)
		type = vk::ImageType::e2D;
	else
		type = vk::ImageType::e3D;

	vk::ImageCreateInfo imageInfo (vk::ImageCreateFlags(), type, format, extent, mipMapLevels, arraySize, vk::SampleCountFlagBits::e1, tiling, usage, vk::SharingMode::eExclusive, 0, nullptr, layout);

	V_CHECKCALL (instance->device.createImage (&imageInfo, nullptr, &image), printf ("Failed To Create Image\n"));

	printf ("Create Image of dimensions %dx%dx%d\n", extent.width, extent.height, extent.depth);

	vk::MemoryRequirements memRequirements;
	instance->device.getImageMemoryRequirements (image, &memRequirements);
	backedMemory = instance->allocateMemory (memRequirements, needed | recommended);
	if (!backedMemory)
		backedMemory = instance->allocateMemory (memRequirements, needed);

	vkBindImageMemory (instance->device, image, backedMemory, 0);
}
ImageWrapper::~ImageWrapper() {
}
void ImageWrapper::destroy(){
	vkDestroyImage (instance->device, image, nullptr);
	vkFreeMemory (instance->device, backedMemory, nullptr);
}


void ImageWrapper::transitionImageLayout (vk::ImageLayout newLayout, uint32_t mipbase, uint32_t mipcount, uint32_t arrayIndex, uint32_t arrayCount, vk::CommandBuffer commandBuffer) {

	vk::AccessFlags srcAccessMask, dstAccessMask;
	vk::PipelineStageFlags sourceStage, destinationStage;

	if (layout == vk::ImageLayout::eUndefined){
		srcAccessMask = vk::AccessFlags();
		sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
	}else if(layout == vk::ImageLayout::eTransferDstOptimal){
		srcAccessMask = vk::AccessFlagBits::eTransferWrite;
		sourceStage = vk::PipelineStageFlagBits::eTransfer;
	}else if(layout == vk::ImageLayout::eTransferSrcOptimal){
		srcAccessMask = vk::AccessFlagBits::eTransferRead;
		sourceStage = vk::PipelineStageFlagBits::eTransfer;
	}else if(layout == vk::ImageLayout::eShaderReadOnlyOptimal){
		srcAccessMask = vk::AccessFlagBits::eShaderRead;
		sourceStage = destinationStage = vk::PipelineStageFlagBits::eFragmentShader;
	}else{
		assert(false);
	}
	
	if (newLayout == vk::ImageLayout::eUndefined){
		dstAccessMask = vk::AccessFlags();
		destinationStage = vk::PipelineStageFlagBits::eBottomOfPipe;
	}else if(newLayout == vk::ImageLayout::eTransferDstOptimal){
		dstAccessMask = vk::AccessFlagBits::eTransferWrite;
		destinationStage = vk::PipelineStageFlagBits::eTransfer;
	}else if(newLayout == vk::ImageLayout::eTransferSrcOptimal){
		dstAccessMask = vk::AccessFlagBits::eTransferRead;
		destinationStage = vk::PipelineStageFlagBits::eTransfer;
	}else if(newLayout == vk::ImageLayout::eDepthStencilAttachmentOptimal){
		dstAccessMask = vk::AccessFlags (vk::AccessFlagBits::eDepthStencilAttachmentRead | vk::AccessFlagBits::eDepthStencilAttachmentWrite);
		destinationStage = vk::PipelineStageFlagBits::eEarlyFragmentTests;
	}else if(newLayout == vk::ImageLayout::eShaderReadOnlyOptimal){
		dstAccessMask = vk::AccessFlagBits::eShaderRead;
		destinationStage = destinationStage = vk::PipelineStageFlagBits::eFragmentShader;
	}else{
		assert(false);
	}

	commandBuffer.pipelineBarrier (
	    sourceStage, destinationStage,
	    vk::DependencyFlags(),
	    {},//memoryBarriers
	{}, {
		//bufferBarriers
		vk::ImageMemoryBarrier (
		    srcAccessMask, dstAccessMask,
		    layout, newLayout,
		    instance->graphQId, instance->graphQId,
		    image,
		    vk::ImageSubresourceRange (aspectFlags, mipbase, mipcount == 0 ? this->mipMapLevels : mipcount, arrayIndex, arrayCount == 0 ? this->arraySize : arrayCount)
		)
	}//imageBarriers
	);
	layout = newLayout;
}

void ImageWrapper::transitionImageLayout (vk::ImageLayout newLayout, uint32_t mipbase, uint32_t mipcount, uint32_t arrayIndex, uint32_t arrayCount, vk::CommandPool commandPool, vk::Queue submitQueue) {

	vk::CommandBuffer commandBuffer = instance->createCommandBuffer (commandPool, vk::CommandBufferLevel::ePrimary);

	commandBuffer.begin (vk::CommandBufferBeginInfo (vk::CommandBufferUsageFlagBits::eOneTimeSubmit));

	transitionImageLayout (newLayout, mipbase, mipcount, arrayIndex, arrayCount, commandBuffer);

	commandBuffer.end();
	vk::SubmitInfo submitInfo (
	    0, nullptr,//waitSemaphores
	    nullptr,//pWaitDstStageMask
	    1, &commandBuffer,
	    0, nullptr//signalsemaphores
	);
	submitQueue.submit ({submitInfo}, vk::Fence());
}
void ImageWrapper::generateMipmaps (uint32_t baseLevel, uint32_t generateLevels, uint32_t arrayIndex, uint32_t arrayCount, vk::ImageLayout targetLayout, vk::CommandBuffer commandBuffer) {
		
	if (generateLevels == 0)
		generateLevels = mipMapLevels - baseLevel;
	if (arrayCount == 0)
		arrayCount = arraySize;
		
	vk::ImageLayout oldLayout = layout;
	transitionImageLayout (vk::ImageLayout::eTransferSrcOptimal, baseLevel, 1, arrayIndex, arrayCount, commandBuffer);
	layout = oldLayout;
	transitionImageLayout (vk::ImageLayout::eTransferDstOptimal, baseLevel + 1, generateLevels - 1, arrayIndex, arrayCount, commandBuffer);
		
	for (unsigned i = 1; i < generateLevels; i++) {
		uint32_t index = i + baseLevel;
		// Set up a blit from previous mip-level to the next.
		vk::ImageBlit imageBlit(vk::ImageSubresourceLayers (aspectFlags, index - 1, arrayIndex, arrayCount), {
			vk::Offset3D(0, 0, 0), vk::Offset3D(std::max (int (extent.width >> (index - 1)), 1), std::max (int (extent.height >> (index - 1)), 1), std::max (int (extent.depth >> (index - 1)), 1) )
		}, vk::ImageSubresourceLayers (aspectFlags, index, arrayIndex, arrayCount), {
			vk::Offset3D(0, 0, 0), vk::Offset3D(std::max (int (extent.width >> index), 1), std::max (int (extent.height >> index), 1), std::max (int (extent.depth >> index), 1) )
		});
		commandBuffer.blitImage (image, layout, image, vk::ImageLayout::eTransferDstOptimal, {imageBlit}, vk::Filter::eLinear);
		layout = vk::ImageLayout::eTransferDstOptimal;
		transitionImageLayout(vk::ImageLayout::eTransferSrcOptimal, index, 1, arrayIndex, arrayCount, commandBuffer);
	}
	transitionImageLayout(targetLayout, baseLevel, generateLevels, arrayIndex, arrayCount, commandBuffer);
	
}
void ImageWrapper::generateMipmaps (uint32_t baseLevel, uint32_t generateLevels, uint32_t arrayIndex, uint32_t arrayCount, vk::ImageLayout targetLayout, vk::CommandPool commandPool, vk::Queue submitQueue) {

	vk::CommandBuffer commandBuffer = instance->createCommandBuffer (commandPool, vk::CommandBufferLevel::ePrimary);

	commandBuffer.begin (vk::CommandBufferBeginInfo (vk::CommandBufferUsageFlagBits::eOneTimeSubmit));

	generateMipmaps (baseLevel, generateLevels, arrayIndex, arrayCount, targetLayout, commandBuffer);

	commandBuffer.end();
	vk::SubmitInfo submitInfo (
	    0, nullptr,//waitSemaphores
	    nullptr,//pWaitDstStageMask
	    1, &commandBuffer,
	    0, nullptr//signalsemaphores
	);
	submitQueue.submit ({submitInfo}, vk::Fence());

}
