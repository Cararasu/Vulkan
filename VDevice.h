#ifndef VDEVICE_H
#define VDEVICE_H

#include <vector>
#include "VHeader.h"

#define V_MAX_PGCQUEUE_COUNT (4)

struct VPGCQueue{
	uint32_t presentQId;
	uint32_t graphicsQId;
	uint32_t computeQId;
	bool combinedPGQ;
	bool combinedGCQ;
	vk::Queue presentQueue;
	vk::Queue graphicsQueue;
	vk::Queue computeQueue;
	
	virtual void submitPresent(uint32_t count, vk::SubmitInfo* submitInfo, vk::Fence fence = VK_NULL_HANDLE) = 0;
	virtual void submitGraphics(uint32_t count, vk::SubmitInfo* submitInfo, vk::Fence fence = VK_NULL_HANDLE) = 0;
	virtual void submitCompute(uint32_t count, vk::SubmitInfo* submitInfo, vk::Fence fence = VK_NULL_HANDLE) = 0;
	virtual void waitForFinish() = 0;
};
struct VSinglePGCQueue : VPGCQueue{
	virtual void submitPresent(uint32_t count, vk::SubmitInfo* submitInfo, vk::Fence fence){
		V_CHECKCALL(presentQueue.submit(count, submitInfo, fence), printf("Submit to Present Queue Failed\n"));
	}
	virtual void submitGraphics(uint32_t count, vk::SubmitInfo* submitInfo, vk::Fence fence){
		V_CHECKCALL(graphicsQueue.submit(count, submitInfo, fence), printf("Submit to Graphics Queue Failed\n"));
	}
	virtual void submitCompute(uint32_t count, vk::SubmitInfo* submitInfo, vk::Fence fence){
		V_CHECKCALL(computeQueue.submit(count, submitInfo, fence), printf("Submit to Compute Queue Failed\n"));
	}
	virtual void waitForFinish(){
		V_CHECKCALL(presentQueue.waitIdle(),printf("Wait for Present Queue Failed\n"));
		V_CHECKCALL(graphicsQueue.waitIdle(),printf("Wait for Graphics Queue Failed\n"));
		V_CHECKCALL(computeQueue.waitIdle(),printf("Wait for Compute Queue Failed\n"));

	}
};
struct VPartlyPGCQueue : VPGCQueue{
	virtual void submitPresent(uint32_t count, vk::SubmitInfo* submitInfo, vk::Fence fence){
		V_CHECKCALL(presentQueue.submit(count, submitInfo, fence), printf("Submit to Present Queue Failed\n"));
	}
	virtual void submitGraphics(uint32_t count, vk::SubmitInfo* submitInfo, vk::Fence fence){
		V_CHECKCALL(presentQueue.submit(count, submitInfo, fence), printf("Submit to Graphics Queue Failed\n"));
	}
	virtual void submitCompute(uint32_t count, vk::SubmitInfo* submitInfo, vk::Fence fence){
		V_CHECKCALL(computeQueue.submit(count, submitInfo, fence), printf("Submit to Compute Queue Failed\n"));
	}
	virtual void waitForFinish(){
		V_CHECKCALL(presentQueue.waitIdle(),printf("Wait for Present Queue Failed\n"));
		V_CHECKCALL(computeQueue.waitIdle(),printf("Wait for Compute Queue Failed\n"));
	}
};
struct VCombinedPGCQueue : VPGCQueue{
	virtual void submitPresent(uint32_t count, vk::SubmitInfo* submitInfo, vk::Fence fence){
		V_CHECKCALL(presentQueue.submit(count, submitInfo, fence), printf("Submit to Present Queue Failed\n"));
	}
	virtual void submitGraphics(uint32_t count, vk::SubmitInfo* submitInfo, vk::Fence fence){
		V_CHECKCALL(presentQueue.submit(count, submitInfo, fence), printf("Submit to Graphics Queue Failed\n"));
	}
	virtual void submitCompute(uint32_t count, vk::SubmitInfo* submitInfo, vk::Fence fence){
	}
	virtual void waitForFinish(){
		V_CHECKCALL(presentQueue.waitIdle(),printf("Wait for Present Queue Failed\n"));
	}
};
struct VTQueue{
	uint32_t transferQId;
	vk::Queue transferQueue;
	
	VTQueue(uint32_t transferQId, vk::Queue transferQueue) : transferQId(transferQId), transferQueue(transferQueue) {}
	
	void submitTransfer(uint32_t count, vk::SubmitInfo* submitInfo, vk::Fence fence){
		V_CHECKCALL(transferQueue.submit(count, submitInfo, fence), printf("Submit to Compute Queue Failed\n"));
	}
	void waitForFinish(){
		V_CHECKCALL(transferQueue.waitIdle(), printf("Wait-Idle for Queue Failed\n"));
	}
	//submit(vk::SubmitInfo, vk::Fence)
	//waitForFinish
};

struct VDevice{
	vk::Device device;
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
