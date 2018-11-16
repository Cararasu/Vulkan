#pragma once

#include <render/RenderTypes.h>
#include "VHeader.h"
#include "VBuffer.h"
#include <render/IdArray.h>
#include <render/Model.h>
#include "VBufferStorage.h"

struct VBuffer;
struct VInstance;
struct VModel;
struct VModelBase;

struct VContextBase : public ContextBase {
	VTransientBufferStorage bufferstorage;

	VContextBase ( VInstance* instance, const DataGroupDefId datagroup_id ) : ContextBase ( datagroup_id ),
		bufferstorage ( instance, vk::BufferUsageFlagBits::eUniformBuffer ) {

	}
};

struct VContextGroup : public ContextGroup {
	VInstance* instance;
	HashMap<ContextBaseId, Context*> context_map;

	VContextGroup ( VInstance* instance ) : instance ( instance ) { }

	virtual void set_context ( Context* context ) override {
		context_map.insert ( std::make_pair ( context->contextbase_id, context ) );
	}
	virtual void remove_context ( ContextBaseId base_id ) override {
		context_map.erase ( base_id );
	}
	virtual void clear() override {
		context_map.clear();
	}
};
