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

ImageWrapper::ImageWrapper(vk::Extent3D size, vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags usage, vk::MemoryPropertyFlags needed, vk::MemoryPropertyFlags recommended): 
	imageSize(size), imageFormat(format), image(vk::Image()), backedMemory(vk::DeviceMemory()){
	if(imageSize.height == 1)
		imageType = vk::ImageType::e1D;
	else if(imageSize.depth == 1)
		imageType = vk::ImageType::e2D;
	else
		imageType = vk::ImageType::e3D;
	createImage(size, format, tiling, usage, needed, recommended, &image, &backedMemory);
}
ImageWrapper::~ImageWrapper(){
	destroyImage(image, backedMemory);
}