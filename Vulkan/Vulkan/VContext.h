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

struct VContextBuffer {
	//DynArray<>

};
struct VContextBase {
	vk::DescriptorSetLayout descriptorset_layout;
};

struct VBaseImage;

struct VContext {
	ContextId id;
	ContextBaseId contextbase_id;
	VInstance* v_instance;
	VBuffer v_buffer;
	
	void* data;
	Array<VBaseImage*> images;
	
	vk::Sampler temp_sampler;
	vk::DescriptorPool descriptor_pool;
	vk::DescriptorSet descriptor_set;
	
	VContext ( VInstance* instance, ContextBaseId contextbase_id );
	~VContext();

	Context context() {
		return {id, contextbase_id};
	}
};

struct VContextGroup : public ContextGroup {
	VInstance* v_instance;
	HashMap<ContextBaseId, VContext*> context_map;

	VContextGroup ( VInstance* instance ) : v_instance ( instance ) { }

	virtual void set_context ( Context& context ) override;
	virtual void remove_context ( ContextBaseId base_id ) override;
	virtual void clear() override;
};
