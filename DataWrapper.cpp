#include "DataWrapper.h"
#include "VGlobal.h"


MappedBufferWrapper* stagingBuffer = nullptr;

BufferWrapper::BufferWrapper(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags needed, VkMemoryPropertyFlags recommended): 
	bufferSize(size), buffer(VK_NULL_HANDLE), backedMemory(VK_NULL_HANDLE){
	createBuffer(size, usage, needed, recommended, &buffer, &backedMemory);
}
BufferWrapper::~BufferWrapper(){
	destroyBuffer(buffer, backedMemory);
}
MappedBufferWrapper::MappedBufferWrapper(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags needed, VkMemoryPropertyFlags recommended): 
	BufferWrapper(size, usage, needed, recommended){
	printf("Map %d Bytes of Memory\n", bufferSize);
	vkMapMemory(vGlobal.deviceWrapper.device, backedMemory, 0, bufferSize, 0, &data);
}
MappedBufferWrapper::~MappedBufferWrapper(){
	printf("Unmap %d Bytes of Memory\n", bufferSize);
	vkUnmapMemory(vGlobal.deviceWrapper.device, backedMemory);
}

ImageWrapper::ImageWrapper(VkExtent3D size, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags needed, VkMemoryPropertyFlags recommended): 
	imageSize(size), imageFormat(format), image(VK_NULL_HANDLE), backedMemory(VK_NULL_HANDLE){
	if(imageSize.height == 1)
		imageType = VK_IMAGE_TYPE_1D;
	else if(imageSize.depth == 1)
		imageType = VK_IMAGE_TYPE_2D;
	else
		imageType = VK_IMAGE_TYPE_3D;
	createImage(size, format, tiling, usage, needed, recommended, &image, &backedMemory);
}
ImageWrapper::~ImageWrapper(){
	destroyImage(image, backedMemory);
}