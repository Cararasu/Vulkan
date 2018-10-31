#pragma once
#include <render/IdArray.h>
#include <render/Model.h>
#include "VBuffer.h"
#include "VBufferStorage.h"

struct VModelInstances {
	SparseStore<void> store;
};

struct DataGroupDef;
struct VModel;
struct VModelBase;

struct VModelInstanceBase : public ModelInstanceBase {
	VTransientBufferStorage bufferstorage;
	
	VModelInstanceBase(VInstance* instance, const ModelInstanceBase* modelinstancebase) : ModelInstanceBase(modelinstancebase->datagroup, modelinstancebase->model),
		bufferstorage(instance, vk::BufferUsageFlagBits::eVertexBuffer){
	}
	VModelInstanceBase(VInstance* instance, const DataGroupDef* datagroup, Model model) : ModelInstanceBase(datagroup, model),
		bufferstorage(instance, vk::BufferUsageFlagBits::eVertexBuffer){
	}
};
struct VModel : public Model {
	bool index_is_2byte;
	u32 vertexoffset, vertexcount;
	u32 indexoffset, indexcount;
	VBuffer vertexbuffer;
	VBuffer indexbuffer;
	
	VModel(VInstance* v_instance, VModelBase* modelbase);
};
struct VModelBase : public ModelBase {

	VBuffer* vertexbuffer = nullptr;
	VBuffer* indexbuffer2 = nullptr;//16-bit indices
	VBuffer* indexbuffer4 = nullptr;//32-bit indices
	
	VModelBase(const DataGroupDef* datagroup);
};
struct VInstanceGroup : public InstanceGroup {
	VInstance* instance;
	VTransientBufferStorage buffer_storeage;
	Map<const ModelInstanceBase*, RId> base_to_id_map;
	bool finished = false;
	
	VInstanceGroup(VInstance* instance) : InstanceGroup(), instance(instance), buffer_storeage(instance, vk::BufferUsageFlagBits::eVertexBuffer) { }
	virtual ~VInstanceGroup() {}
	
	
	//returns offset
	virtual u64 register_instances ( const ModelInstanceBase* base, u32 count ) override;
	//get base offset
	virtual void* finish_register () override;
	virtual void clear() override;
};