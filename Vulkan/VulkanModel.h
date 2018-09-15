#pragma once
#include <render/IdArray.h>
#include <render/Model.h>
#include "VulkanBuffer.h"
#include "VulkanTransferOperator.h"

struct VulkanModelInstances {
	SparseStore<void> store;
};

struct DataGroupDef;
struct VulkanModel;
struct VulkanModelBase;

struct VulkanModelInstanceBase : public ModelInstanceBase {
	VulkanBufferStorage bufferstorage;
	
	VulkanModelInstanceBase(VulkanInstance* instance, const ModelInstanceBase* modelinstancebase) : ModelInstanceBase(modelinstancebase->datagroup, modelinstancebase->model),
		bufferstorage(instance, vk::BufferUsageFlagBits::eVertexBuffer){
	}
	VulkanModelInstanceBase(VulkanInstance* instance, const DataGroupDef* datagroup, Model model) : ModelInstanceBase(datagroup, model),
		bufferstorage(instance, vk::BufferUsageFlagBits::eVertexBuffer){
	}
};
struct VulkanModel : public Model {
	bool index_is_2byte;
	u32 vertexoffset, vertexcount;
	u32 indexoffset, indexcount;
	VulkanBuffer vertexbuffer;
	VulkanBuffer indexbuffer;
	
	VulkanModel(VulkanInstance* v_instance, VulkanModelBase* modelbase);
};
struct VulkanModelBase : public ModelBase {

	VulkanBuffer* vertexbuffer = nullptr;
	VulkanBuffer* indexbuffer2 = nullptr;//16-bit indices
	VulkanBuffer* indexbuffer4 = nullptr;//32-bit indices
	
	VulkanModelBase(const DataGroupDef* datagroup);
};
struct VulkanInstanceGroup : public InstanceGroup {
	VulkanInstance* instance;
	
	VulkanInstanceGroup(VulkanInstance* instance) : InstanceGroup(), instance(instance) { }
	
	virtual ModelInstance add_instance(ModelInstanceBase* base);
	virtual void* get_data_ptr(ModelInstance instance);
	virtual void clear();
};