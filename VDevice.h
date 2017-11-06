#ifndef VDEVICE_H
#define VDEVICE_H

#include <vulkan/vulkan.h>
#include <vector>
#include "VHeader.h"

#define V_MAX_PGCQUEUE_COUNT (4)

struct VPGCQueue{
	uint32_t presentQId;
	uint32_t graphicsQId;
	uint32_t computeQId;
	bool combinedPGQ;
	bool combinedGCQ;
	VkQueue presentQueue;
	VkQueue graphicsQueue;
	VkQueue computeQueue;
	
	virtual void submitPresent(uint32_t count, VkSubmitInfo* submitInfo, VkFence fence = VK_NULL_HANDLE) = 0;
	virtual void submitGraphics(uint32_t count, VkSubmitInfo* submitInfo, VkFence fence = VK_NULL_HANDLE) = 0;
	virtual void submitCompute(uint32_t count, VkSubmitInfo* submitInfo, VkFence fence = VK_NULL_HANDLE) = 0;
	virtual void waitForFinish() = 0;
};
struct VSinglePGCQueue : VPGCQueue{
	virtual void submitPresent(uint32_t count, VkSubmitInfo* submitInfo, VkFence fence){
		VCHECKCALL(vkQueueSubmit(presentQueue, count, submitInfo, fence), {
			printf("Submit to Present Queue Failed\n");
		});
	}
	virtual void submitGraphics(uint32_t count, VkSubmitInfo* submitInfo, VkFence fence){
		VCHECKCALL(vkQueueSubmit(graphicsQueue, count, submitInfo, fence), {
			printf("Submit to Graphics Queue Failed\n");
		});
	}
	virtual void submitCompute(uint32_t count, VkSubmitInfo* submitInfo, VkFence fence){
		VCHECKCALL(vkQueueSubmit(computeQueue, count, submitInfo, fence), {
			printf("Submit to Compute Queue Failed\n");
		});
	}
	virtual void waitForFinish(){
		VCHECKCALL(vkQueueWaitIdle(presentQueue), {
			printf("Wait for Present Queue Failed\n");
		});
		VCHECKCALL(vkQueueWaitIdle(graphicsQueue), {
			printf("Wait for Graphics Queue Failed\n");
		});
		VCHECKCALL(vkQueueWaitIdle(computeQueue), {
			printf("Wait for Compute Queue Failed\n");
		});
	}
};
struct VPartlyPGCQueue : VPGCQueue{
	virtual void submitPresent(uint32_t count, VkSubmitInfo* submitInfo, VkFence fence){
		VCHECKCALL(vkQueueSubmit(presentQueue, count, submitInfo, fence), {
			printf("Submit to Present Queue Failed\n");
		});
	}
	virtual void submitGraphics(uint32_t count, VkSubmitInfo* submitInfo, VkFence fence){
		VCHECKCALL(vkQueueSubmit(presentQueue, count, submitInfo, fence), {
			printf("Submit to Graphics Queue Failed\n");
		});
	}
	virtual void submitCompute(uint32_t count, VkSubmitInfo* submitInfo, VkFence fence){
		VCHECKCALL(vkQueueSubmit(computeQueue, count, submitInfo, fence), {
			printf("Submit to Compute Queue Failed\n");
		});
	}
	virtual void waitForFinish(){
		VCHECKCALL(vkQueueWaitIdle(presentQueue), {
			printf("Wait-Idle for Present-Graphics Queue Failed\n");
		});
		VCHECKCALL(vkQueueWaitIdle(computeQueue), {
			printf("Wait-Idle for Compute Queue Failed\n");
		});
	}
};
struct VCombinedPGCQueue : VPGCQueue{
	virtual void submitPresent(uint32_t count, VkSubmitInfo* submitInfo, VkFence fence){
		VCHECKCALL(vkQueueSubmit(presentQueue, count, submitInfo, fence), {
			printf("Submit to Present Queue Failed\n");
		});
	}
	virtual void submitGraphics(uint32_t count, VkSubmitInfo* submitInfo, VkFence fence){
		VCHECKCALL(vkQueueSubmit(presentQueue, count, submitInfo, fence), {
			printf("Submit to Graphics Queue Failed\n");
		});
	}
	virtual void submitCompute(uint32_t count, VkSubmitInfo* submitInfo, VkFence fence){
		VCHECKCALL(vkQueueSubmit(presentQueue, count, submitInfo, fence), {
			printf("Submit to Compute Queue Failed\n");
		});
	}
	virtual void waitForFinish(){
		VCHECKCALL(vkQueueWaitIdle(presentQueue), {
			printf("Wait-Idle for Queue Failed\n");
		});
	}
};
struct VTQueue{
	uint32_t transferQId;
	VkQueue transferQueue;
	
	VTQueue(uint32_t transferQId, VkQueue transferQueue) : transferQId(transferQId), transferQueue(transferQueue) {}
	
	void submitTransfer(uint32_t count, VkSubmitInfo* submitInfo, VkFence fence){
		VCHECKCALL(vkQueueSubmit(transferQueue, count, submitInfo, fence), {
			printf("Submit to Compute Queue Failed\n");
		});
	}
	void waitForFinish(){
		VCHECKCALL(vkQueueWaitIdle(transferQueue), {
			printf("Wait-Idle for Queue Failed\n");
		});
	}
	//submit(VkSubmitInfo, VkFence)
	//waitForFinish
};

struct VDevice{
	VkDevice device;
	size_t pgcIndex = 0;
	std::vector<VPGCQueue*> pgcQueues;
	VTQueue* tqueue = nullptr;
	
	uint32_t compQId = -1, graphQId = -1, presentQId = -1, transfQId = -1;
	
	VTQueue* requestTransferQueue(){
		return tqueue;
	}
	VPGCQueue* requestPGCQueue(){//TODO synch
		VPGCQueue* queue = pgcQueues[pgcIndex];
		pgcIndex = (pgcIndex + 1) % pgcQueues.size();
		return queue;
	}
	VPGCQueue* getPGCQueue(){//TODO synch
		return pgcQueues[pgcIndex];
	}
};

#endif // VDEVICE_H
