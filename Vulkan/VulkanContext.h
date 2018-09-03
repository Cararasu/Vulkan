#pragma once

#include <render/RenderTypes.h>
#include "VulkanHeader.h"
#include "VulkanBuffer.h"
#include <render/IdArray.h>
#include <render/Model.h>
#include "VulkanTransferOperator.h"

struct VulkanBuffer;
struct VulkanInstance;
struct VulkanModel;
struct VulkanModelBase;

struct VulkanContextBase{
	RId id;
	const DataGroupDef* datagroup;
	VulkanBufferStorage bufferstorage;
	
	VulkanContextBase(VulkanInstance* instance, DataGroupDef* datagroup) : bufferstorage(instance, vk::BufferUsageFlagBits::eUniformBuffer), datagroup(datagroup){
		
	}
};

struct VulkanGPUDataStore {
	VulkanInstance* instance;
	RId datagroup;
	VulkanBuffer buffer;
	SparseStore<void> datastore;

	VulkanGPUDataStore ( VulkanInstance* instance, const DataGroupDef* datagroup, vk::BufferUsageFlags usage );
	~VulkanGPUDataStore();

	RId allocate_block();
	void remove_block ( RId index );
	void update_data();
};
