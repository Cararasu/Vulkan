#pragma once

#include <render/RenderTypes.h>
#include "VHeader.h"
#include "VBuffer.h"
#include <render/IdArray.h>
#include <render/Model.h>
#include "VBufferStorage.h"
#include "VImage.h"

struct VBuffer;
struct VInstance;
struct VModel;
struct VSampler;

extern const u32 CONTEXT_DESCRIPTOR_SET_COUNT;

struct VContextDS {
	u64 last_updated_frame_index = 0;
	vk::DescriptorSet descriptor_set;
};

struct VContextBase {
	vk::DescriptorSetLayout descriptorset_layout;
};


struct VBoundTextureResource {
	VImageUseRef imageuse;
	VSampler* sampler = nullptr;
	//Bufferuse
	vk::DescriptorType type;
};

struct VContext {
	ContextId id;
	ContextBaseId contextbase_id;
	VInstance* v_instance;
	Chunk buffer_chunk;
	u64 last_frame_index_updated = 0;

	void* data;
	Array<VBoundTextureResource> texture_resources;
	bool needs_update = true;

	vk::DescriptorPool descriptor_pool;
	u32 current_ds;
	VContextDS descriptor_sets[2];

	VContext ( VInstance* instance, ContextBaseId contextbase_id );
	~VContext();

	Context context() {
		return {id, contextbase_id};
	}
	void cycle_descriptor_set() {
		current_ds = (current_ds + 1) % CONTEXT_DESCRIPTOR_SET_COUNT;
	}
	vk::DescriptorSet descriptor_set() {
		return descriptor_sets[current_ds].descriptor_set;
	}
	void prepare_for_use();
};

struct VContextGroup : public ContextGroup {
	VInstance* v_instance;
	HashMap<ContextBaseId, VContext*> context_map;

	VContextGroup ( VInstance* instance ) : v_instance ( instance ) { }

	virtual void set_context ( Context& context ) override;
	virtual void remove_context ( ContextBaseId base_id ) override;
	virtual void clear() override;
};
