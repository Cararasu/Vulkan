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
	bool presentCanTransfer;
	bool graphicsCanTransfer;
	bool computeCanTransfer;
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
		VCHECKCALL(vkQueueSubmit(presentQueue, count, submitInfo, fence),"Submit to Present Queue Failed\n");
	}
	virtual void submitGraphics(uint32_t count, VkSubmitInfo* submitInfo, VkFence fence){
		VCHECKCALL(vkQueueSubmit(graphicsQueue, count, submitInfo, fence),"Submit to Graphics Queue Failed\n");
	}
	virtual void submitCompute(uint32_t count, VkSubmitInfo* submitInfo, VkFence fence){
		VCHECKCALL(vkQueueSubmit(computeQueue, count, submitInfo, fence),"Submit to Compute Queue Failed\n");
	}
	virtual void waitForFinish(){
		VCHECKCALL(vkQueueWaitIdle(presentQueue),"Wait-Idle for Present Queue Failed\n");
		VCHECKCALL(vkQueueWaitIdle(graphicsQueue),"Wait-Idle for Graphics Queue Failed\n");
		VCHECKCALL(vkQueueWaitIdle(computeQueue),"Wait-Idle for Compute Queue Failed\n");
	}
};
struct VPartlyPGCQueue : VPGCQueue{
	virtual void submitPresent(uint32_t count, VkSubmitInfo* submitInfo, VkFence fence){
		VCHECKCALL(vkQueueSubmit(presentQueue, count, submitInfo, fence),"Submit to Present Queue Failed\n");
	}
	virtual void submitGraphics(uint32_t count, VkSubmitInfo* submitInfo, VkFence fence){
		VCHECKCALL(vkQueueSubmit(presentQueue, count, submitInfo, fence),"Submit to Graphics Queue Failed\n");
	}
	virtual void submitCompute(uint32_t count, VkSubmitInfo* submitInfo, VkFence fence){
		VCHECKCALL(vkQueueSubmit(computeQueue, count, submitInfo, fence),"Submit to Compute Queue Failed\n");
	}
	virtual void waitForFinish(){
		VCHECKCALL(vkQueueWaitIdle(presentQueue),"Wait-Idle for Present-Graphics Queue Failed\n");
		VCHECKCALL(vkQueueWaitIdle(computeQueue),"Wait-Idle for Compute Queue Failed\n");
	}
};
struct VCombinedPGCQueue : VPGCQueue{
	virtual void submitPresent(uint32_t count, VkSubmitInfo* submitInfo, VkFence fence){
		VCHECKCALL(vkQueueSubmit(presentQueue, count, submitInfo, fence),"Submit to Present Queue Failed\n");
	}
	virtual void submitGraphics(uint32_t count, VkSubmitInfo* submitInfo, VkFence fence){
		VCHECKCALL(vkQueueSubmit(presentQueue, count, submitInfo, fence),"Submit to Graphics Queue Failed\n");
	}
	virtual void submitCompute(uint32_t count, VkSubmitInfo* submitInfo, VkFence fence){
		VCHECKCALL(vkQueueSubmit(presentQueue, count, submitInfo, fence),"Submit to Compute Queue Failed\n");
	}
	virtual void waitForFinish(){
		VCHECKCALL(vkQueueWaitIdle(presentQueue),"Wait-Idle for Queue Failed\n");
	}
};
class VTQueue{
	uint32_t transferQId;
	
	//submit(VkSubmitInfo, VkFence)
	//waitForFinish
};

struct VDevice{
	VkDevice device;
	size_t pgcIndex = 0;
	std::vector<VPGCQueue*> pgcQueues;
	VTQueue* tqueue;
	
	VTQueue* requestTransferQueue(){
		return tqueue;
	}
	VPGCQueue* requestPGCQueue(){
		VPGCQueue* queue = pgcQueues[pgcIndex];
		pgcIndex = (pgcIndex + 1) % pgcQueues.size();
		return queue;
	}
};

#endif // VDEVICE_H
