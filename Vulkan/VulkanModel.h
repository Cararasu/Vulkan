#pragma once
#include <render/IdArray.h>
#include "VulkanBuffer.h"

struct VulkanModelInstances {
	SparseStore<void> store;
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
