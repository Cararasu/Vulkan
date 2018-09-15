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

struct VulkanContextBase : public ContextBase{
	VulkanBufferStorage bufferstorage;
	
	VulkanContextBase(VulkanInstance* instance, const DataGroupDef* datagroup) : ContextBase(datagroup),
		bufferstorage(instance, vk::BufferUsageFlagBits::eUniformBuffer) {
		
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

struct VulkanContextGroup : public ContextGroup {
	VulkanInstance* instance;
	HashMap<IdHandle, Context*> context_map;
	
	VulkanContextGroup(VulkanInstance* instance) : instance(instance) { }
	
	virtual void set_context(Context* context) override {
		context_map.insert(std::make_pair((IdHandle)*context->contextbase, context));
	}
	virtual void remove_context(ContextBase* base) override {
		context_map.erase(*base);
	}
	virtual void clear() override {
		context_map.clear();
	}
};