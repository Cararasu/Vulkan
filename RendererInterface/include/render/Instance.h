#pragma once

#include "Monitor.h"
#include "Header.h"
#include "Window.h"
#include "AModel.h"
#include "RenderPass.h"
#include "ResourceManager.h"
#include "IdArray.h"
#include <atomic>

struct Device {
	const char* name;
	u64 rating;
	virtual ~Device() {}
};

struct InstanceOptions {
	u64 max_number_of_window_frames_in_pipeline = 3;
};


struct Instance {
	Array<Monitor*> monitors;
	Array<Device*> devices;
	Array<Window*> windows;

	virtual ~Instance() {}

	virtual bool initialize ( InstanceOptions options, Device* device = nullptr ) = 0;

	virtual Window* create_window() = 0;
	virtual bool destroy_window ( Window* window ) = 0;

	virtual void process_events() = 0;
	virtual void process_events(double timeout) = 0;
	virtual bool is_window_open() = 0;

	virtual void render_bundles ( Array<RenderBundle*> bundles ) = 0;

	virtual Monitor* get_primary_monitor() = 0;

	virtual ResourceManager* resource_manager() = 0;
//------------ Resources

	virtual void set_context (Model& model, Context& context) = 0;

	IdArray<ContextBase> contextbase_store;
	IdArray<ContextBase, RID_STATIC_IDS> static_contextbase_store;
	ContextBaseId register_contextbase ( DataGroupDef* datagroup ) {
		ContextBase contextbase = { 0, *datagroup };
		ContextBaseId id = contextbase_store.insert ( contextbase );
		contextbase_registered ( id );
		return id;
	}
	const ContextBase* contextbase ( ContextBaseId id ) {
		if ( id >= RID_STATIC_IDS )
			return static_contextbase_store.get ( id );
		return contextbase_store.get ( id );
	}

	IdArray<ModelBase> modelbase_store;
	IdArray<ModelBase, RID_STATIC_IDS> static_modelbase_store;
	ModelBaseId register_modelbase ( DataGroupDef* datagroup, Array<ContextBaseId>& contextbase_ids ) {
		ModelBase modelbase = { 0, *datagroup, contextbase_ids };
		ModelBaseId id = modelbase_store.insert ( modelbase );
		modelbase_registered ( id );
		return id;
	}
	const ModelBase* modelbase ( ModelBaseId id ) {
		if ( id >= RID_STATIC_IDS )
			return static_modelbase_store.get ( id );
		return modelbase_store.get ( id );
	}
	
	IdArray<InstanceBase> instancebase_store;
	IdArray<InstanceBase, RID_STATIC_IDS> static_instancebase_store;
	InstanceBaseId register_instancebase ( DataGroupDef* datagroup ) {
		InstanceBase instancebase = { 0, *datagroup };
		InstanceBaseId id = instancebase_store.insert ( instancebase );
		instancebase_registered ( id );
		return id;
	}
	const InstanceBase* instancebase ( InstanceBaseId id ) {
		if ( id >= RID_STATIC_IDS )
			return static_instancebase_store.get ( id );
		return instancebase_store.get ( id );
	}
	
	virtual Context create_context ( ContextBaseId id ) = 0;
	virtual Model create_model ( ModelBaseId id ) = 0;
	
	virtual void contextbase_registered ( ContextBaseId id ) = 0;
	virtual void modelbase_registered ( ModelBaseId id ) = 0;
	virtual void instancebase_registered ( InstanceBaseId id ) = 0;
	
	virtual void load_generic_model ( Model& model, void* vertices, u32 vertexcount, u16* indices, u32 indexcount ) = 0;
	virtual void load_generic_model ( Model& model, void* vertices, u32 vertexcount, u32* indices, u32 indexcount ) = 0;
	
	virtual void unload_model ( ModelId model_id ) = 0;

	virtual void update_context_data(Context& context, void* data) = 0;
	virtual void update_context_image_sampler ( Context& context, u32 index, u32 array_index, ImageUseRef imageuse, Sampler* sampler ) = 0;

	virtual InstanceGroup* create_instancegroup() = 0;
	virtual ContextGroup* create_contextgroup() = 0;

	virtual RenderBundle* create_renderbundle ( InstanceGroup* igroup, ContextGroup* cgroup, Array<const RenderStage*>& rstages, Array<ImageUsage>& image_types, Array<ImageDependency>& dependencies ) = 0;

//---------- Specialized Functions

	virtual RenderBundle* create_main_bundle ( ) = 0;
};

Instance* create_instance ( String name );

void destroy_instance ( Instance* instance );
