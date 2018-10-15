#pragma once

#include <render/RenderTypes.h>
#include "VHeader.h"
#include "VBuffer.h"
#include <render/IdArray.h>
#include <render/Model.h>
#include "VTransferOperator.h"

struct VBuffer;
struct VInstance;
struct VModel;
struct VModelBase;

struct VContextBase : public ContextBase {
	VBufferStorage bufferstorage;

	VContextBase ( VInstance* instance, const DataGroupDef* datagroup ) : ContextBase ( datagroup ),
		bufferstorage ( instance, vk::BufferUsageFlagBits::eUniformBuffer ) {

	}
};

struct VGPUDataStore {
	VInstance* instance;
	RId datagroup;
	VBuffer buffer;
	SparseStore<void> datastore;

	VGPUDataStore ( VInstance* instance, const DataGroupDef* datagroup, vk::BufferUsageFlags usage );
	~VGPUDataStore();

	RId allocate_block();
	void remove_block ( RId index );
	void update_data();
};

struct VContextGroup : public ContextGroup {
	VInstance* instance;
	HashMap<IdHandle, Context*> context_map;

	VContextGroup ( VInstance* instance ) : instance ( instance ) { }

	virtual void set_context ( Context* context ) override {
		context_map.insert ( std::make_pair ( ( IdHandle ) *context->contextbase, context ) );
	}
	virtual void remove_context ( ContextBase* base ) override {
		context_map.erase ( *base );
	}
	virtual void clear() override {
		context_map.clear();
	}
};
