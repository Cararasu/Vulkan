#pragma once

#include "Header.h"
#include "Model.h"
#include "Resources.h"
#include "RenderPass.h"
#include "IdArray.h"

template<typename VERTEXOBJ>
struct ModelStore {
	
	virtual void load(Array<VERTEXOBJ> vertices, Array<u16> indices) = 0;
	virtual void load(Array<VERTEXOBJ> vertices, Array<u32> indices) = 0;
};
template<typename CONTEXTOBJ>
struct ContextStore {
	
	virtual Context* create_new(ResourceHandle handle) = 0;
};
template<typename INSTANCE>
struct InstanceStore {
	
	virtual INSTANCE* create_new_from_model(ResourceHandle handle) = 0;
};

struct ResourceManager {
	IdStore<DataGroupDef> datagroup_store;
	IdStore<ModelBase> modelbase_store;
	IdStore<ContextBase> contextbase_store;

	ResourceHandle register_modelbase ( ModelBase& modelbase ) {
		return {ResourceType::eModelBase, modelbase_store.insert ( modelbase ) };
	}
	ResourceHandle register_contextbase ( ContextBase& contextbase ) {
		return {ResourceType::eContextBase, contextbase_store.insert ( contextbase ) };
	}
	virtual const Model load_generic_model ( RId modelbase_id, u8* vertices, u32 vertexcount, u16* indices, u32 indexcount ) = 0;
	virtual const Model load_generic_model ( RId modelbase_id, u8* vertices, u32 vertexcount, u32* indices, u32 indexcount ) = 0;
	
	virtual const ModelInstance create_instance( RId model_id, RId context_id = 0 ) = 0;
	virtual const ModelInstance create_instance( Model model, RId context_id = 0 ) = 0;

	RenderPass* create_renderpass ( RenderPassDef renderpass );

	Image* create_depthimage ( ImageFormat format, u32 width, u32 height = 0, u32 depth = 0, u32 layers = 1, u32 mipmaplayers = 1 );
	Image* create_colorimage ( ImageFormat format, u32 width, u32 height = 0, u32 depth = 0, u32 layers = 1, u32 mipmaplayers = 1 );
	Image* create_gbuffer ( ImageFormat format, u32 width, u32 height = 0, u32 depth = 0, u32 layers = 1, u32 mipmaplayers = 1 );

	Resource* get_resource ( const char* string );
	Resource* get_resource ( ResourceHandle handle );
};
