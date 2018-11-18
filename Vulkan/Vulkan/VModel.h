#pragma once
#include <render/IdArray.h>
#include <render/Model.h>
#include "VBuffer.h"
#include "VContext.h"
#include "VBufferStorage.h"

struct VModelInstances {
	SparseStore<void> store;
};

struct DataGroupDef;
struct VModel;
struct VContext;


struct VModel : public Model {
	ModelId id;
	ModelBaseId modelbase_id;
	Array<VContext*> v_contexts;
	VInstance* v_instance;

	bool index_is_2byte;
	u32 vertexoffset, vertexcount;
	u32 indexoffset, indexcount;
	VBuffer vertexbuffer;
	VBuffer indexbuffer;

	VModel ( VInstance* instance, ModelBaseId modelbase_id, Array<VContext*>& contexts );

	Model model() {
		Array<Context> contexts ( v_contexts.size );
		for ( u32 i = 0; i < v_contexts.size; i++ ) {
			contexts[i] = v_contexts[i]->context();
		}
		return {id, modelbase_id, contexts};
	}
};
struct InstanceBlock {
	InstanceBaseId base_id;
	ModelId model_id;
	ModelBaseId modelbase_id;
	u64 offset;
	void* data;
	u32 count;
};
struct VInstanceGroup : public InstanceGroup {
	VInstance* v_instance;
	VTransientBufferStorage buffer_storeage;
	Map<InstanceBaseId, DynArray<InstanceBlock>> instance_to_data_map;
	bool finished = false;

	VInstanceGroup ( VInstance* instance ) :
		InstanceGroup(), v_instance ( instance ), 
		buffer_storeage ( instance, vk::BufferUsageFlags() | vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst ) { }
	virtual ~VInstanceGroup() {}

	//returns offset
	virtual void register_instances ( InstanceBaseId instancebase_id, Model& model, void* data, u32 count ) override;
	virtual void clear() override;
};
