#pragma once
#include <render/IdArray.h>
#include "VulkanBuffer.h"
#include "VulkanTransferOperator.h"

struct VulkanModelInstances {
	SparseStore<void> store;
};

struct DataGroupDef;
struct VulkanModel;
struct VulkanModelBase;

struct VulkanModelInstanceBase{
	RId id;
	DataGroupDef* const datagroup;
	VulkanModel* const model;
	VulkanModelBase* const modelbase;
	VulkanBufferStorage bufferstorage;
	
	VulkanModelInstanceBase(VulkanInstance* instance, DataGroupDef* datagroup, VulkanModel* model, VulkanModelBase* modelbase) : 
		bufferstorage(instance, vk::BufferUsageFlagBits::eVertexBuffer), datagroup(datagroup), model(model), modelbase(modelbase){
		
	}
};
struct VulkanModel {
	RId id;
	RId modelbase;
	bool index_is_2byte;
	u32 vertexoffset, vertexcount;
	u32 indexoffset, indexcount;
	VulkanBuffer vertexbuffer;
	VulkanBuffer indexbuffer;
	
	VulkanModel(VulkanInstance* v_instance, RId modelbase);
};
struct VulkanModelBase {
	RId id = 0;
	RId datagroup;

	VulkanBuffer* vertexbuffer = nullptr;
	VulkanBuffer* indexbuffer2 = nullptr;//16-bit indices
	VulkanBuffer* indexbuffer4 = nullptr;//32-bit indices
	
	VulkanModelBase(RId datagroup);
};
