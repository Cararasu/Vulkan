#pragma once

#include "VHeader.h"
#include <vector>

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
	
	virtual void submitPresent(uint32_t count, vk::SubmitInfo* submitInfo, vk::Fence fence = vk::Fence()) = 0;
	virtual void submitGraphics(uint32_t count, vk::SubmitInfo* submitInfo, vk::Fence fence = vk::Fence()) = 0;
	virtual void submitCompute(uint32_t count, vk::SubmitInfo* submitInfo, vk::Fence fence = vk::Fence()) = 0;
	virtual void waitForFinish() = 0;
	virtual ~VPGCQueue(){}
};

struct VSinglePGCQueue : VPGCQueue{
	
	virtual ~VSinglePGCQueue(){}
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
		V_CHECKCALL_MAYBE(presentQueue.waitIdle(),printf("Wait for Present Queue Failed\n"));
		V_CHECKCALL_MAYBE(graphicsQueue.waitIdle(),printf("Wait for Graphics Queue Failed\n"));
		V_CHECKCALL_MAYBE(computeQueue.waitIdle(),printf("Wait for Compute Queue Failed\n"));

	}
};
struct VPartlyPGCQueue : VPGCQueue{
	virtual ~VPartlyPGCQueue(){}
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
		V_CHECKCALL_MAYBE(presentQueue.waitIdle(),printf("Wait for Present Queue Failed\n"));
		V_CHECKCALL_MAYBE(computeQueue.waitIdle(),printf("Wait for Compute Queue Failed\n"));
	}
};
struct VCombinedPGCQueue : VPGCQueue{
	virtual ~VCombinedPGCQueue(){}
	virtual void submitPresent(uint32_t count, vk::SubmitInfo* submitInfo, vk::Fence fence){
		V_CHECKCALL(presentQueue.submit(count, submitInfo, fence), printf("Submit to Present Queue Failed\n"));
	}
	virtual void submitGraphics(uint32_t count, vk::SubmitInfo* submitInfo, vk::Fence fence){
		V_CHECKCALL(graphicsQueue.submit(count, submitInfo, fence), printf("Submit to Graphics Queue Failed\n"));
	}
	virtual void submitCompute(uint32_t count, vk::SubmitInfo* submitInfo, vk::Fence fence){
	}
	virtual void waitForFinish(){
		V_CHECKCALL_MAYBE(presentQueue.waitIdle(),printf("Wait for Present Queue Failed\n"));
	}
};
struct VTQueue{
	uint32_t transferQId;
	vk::Queue transferQueue;
	
	VTQueue(uint32_t transferQId, vk::Queue transferQueue) : transferQId(transferQId), transferQueue(transferQueue) {}
	
	void submitTransfer(uint32_t count, vk::SubmitInfo* submitInfo, vk::Fence fence = vk::Fence()){
		V_CHECKCALL(transferQueue.submit(count, submitInfo, fence), printf("Submit to Compute Queue Failed\n"));
	}
	void waitForFinish(){
		V_CHECKCALL_MAYBE(transferQueue.waitIdle(), printf("Wait-Idle for Queue Failed\n"));
	}
	//submit(vk::SubmitInfo, vk::Fence)
	//waitForFinish
};