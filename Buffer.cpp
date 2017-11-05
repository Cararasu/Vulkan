
#include "VHeader.h"
#include "VGlobal.h"
#include "DataWrapper.h"

VkCommandPool singleTransferCommandPool = VK_NULL_HANDLE;
VkCommandPool transferCommandBuffer = VK_NULL_HANDLE;
VkCommandBuffer singleTransferCommandBuffer = VK_NULL_HANDLE;
VkCommandBuffer singleImageTransitionBuffer = VK_NULL_HANDLE;



void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags needed, VkMemoryPropertyFlags recommended, VkBuffer* buffer, VkDeviceMemory* bufferMemory) {
	VkBufferCreateInfo bufferInfo = {};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size;
	bufferInfo.usage = usage;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VCHECKCALL(vkCreateBuffer(vGlobal.deviceWrapper.device, &bufferInfo, nullptr, buffer), printf("Failed To Create Buffer\n"));

	printf("Create Buffer of size %d\n", size);
	
	*bufferMemory = allocateBufferMemory(*buffer, needed, recommended);
	
	vkBindBufferMemory(vGlobal.deviceWrapper.device, *buffer, *bufferMemory, 0);
}
void destroyBuffer(VkBuffer buffer, VkDeviceMemory bufferMemory){
	vkDestroyBuffer(vGlobal.deviceWrapper.device, buffer, nullptr);
	vkFreeMemory(vGlobal.deviceWrapper.device, bufferMemory, nullptr);
}

void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize offset, VkDeviceSize size) {
	
	if(transferCommandBuffer == VK_NULL_HANDLE){
		VTQueue* queue = vGlobal.deviceWrapper.requestTransferQueue();
		if(queue)
			singleTransferCommandPool = createCommandPool(queue->transferQId, VK_COMMAND_POOL_CREATE_TRANSIENT_BIT);
		else
			singleTransferCommandPool = createCommandPool(vGlobal.deviceWrapper.getPGCQueue()->graphicsQId, VK_COMMAND_POOL_CREATE_TRANSIENT_BIT);
	}
	
	VkCommandBuffer commandBuffer = createCommandBuffer(singleTransferCommandPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY);
	
    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);
	
	VkBufferCopy copyRegion = {};
	copyRegion.srcOffset = 0; // Optional
	copyRegion.dstOffset = offset; // Optional
	copyRegion.size = size;
	vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
	
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

void transferData(const void* srcData, VkBuffer targetBuffer, VkDeviceSize offset, VkDeviceSize size){
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
	copyBuffer(stagingBuffer->buffer, targetBuffer, offset, size);
	
	if(size > V_MAX_STAGINGBUFFER_SIZE){
		printf("Stagingbuffer too big -> delete\n");
		delete stagingBuffer;
		stagingBuffer = nullptr;
	}
	
}