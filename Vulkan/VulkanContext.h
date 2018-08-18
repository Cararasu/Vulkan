#pragma once

#include <render/RenderTypes.h>
#include "VulkanHeader.h"
#include "VulkanBuffer.h"
#include <render/IdArray.h>
#include <render/Model.h>

struct VulkanBuffer;
struct VulkanInstance;

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
