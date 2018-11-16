#pragma once

#include "Monitor.h"
#include "Header.h"
#include "Window.h"
#include "Model.h"
#include "RenderPass.h"
#include "ResourceManager.h"
#include "IdArray.h"

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
	virtual void present_window ( Window* window ) = 0;
	virtual void present_windows() = 0;
	virtual bool is_window_open() = 0;

	virtual void prepare_render () = 0;
	virtual void prepare_render ( Array<Window*> windows ) = 0;

	virtual void render_bundles ( Array<RenderBundle*> bundles ) = 0;

	virtual Monitor* get_primary_monitor() = 0;

	virtual ResourceManager* resource_manager() = 0;
//------------ Resources

	IdArray<DataGroupDef> datagroup_store;
	IdArray<DataGroupDef, RID_STATIC_IDS> static_datagroup_store;
	DataGroupDefId register_datagroupdef ( Array<DataValueDef> valuedefs, u32 size, u32 arraycount ) {
		DataGroupDef datagroupdef ( valuedefs, size, arraycount );
		DataGroupDefId id = datagroup_store.insert ( datagroupdef );
		datagroupdef_registered ( id );
		return id;
	}
	const DataGroupDef* datagroupdef ( DataGroupDefId id ) {
		if ( id >= RID_STATIC_IDS )
			return static_datagroup_store.get ( id );
		return datagroup_store.get ( id );
	}

	IdArray<ContextBase> contextbase_store;
	IdArray<ContextBase, RID_STATIC_IDS> static_contextbase_store;
	ContextBaseId register_contextbase ( DataGroupDefId datagroup_id ) {
		ContextBase contextbase ( datagroup_id );
		ContextBaseId id = contextbase_store.insert ( contextbase );
		contextbase_registered ( id );
		return id;
	}
	ContextBaseId register_contextbase ( const DataGroupDef* datagroup ) {
		return register_contextbase ( datagroup->id );
	}
	const ContextBase* contextbase ( ContextBaseId id ) {
		if ( id >= RID_STATIC_IDS )
			return static_contextbase_store.get ( id );
		return contextbase_store.get ( id );
	}

	IdArray<ModelBase> modelbase_store;
	IdArray<ModelBase, RID_STATIC_IDS> static_modelbase_store;
	ModelBaseId register_modelbase ( DataGroupDefId datagroup_id ) {
		ModelBase modelbase ( datagroup_id );
		ModelBaseId id = modelbase_store.insert ( modelbase );
		modelbase_registered ( id );
		return id;
	}
	ModelBaseId register_modelbase ( const DataGroupDef* datagroup ) {
		return register_modelbase ( datagroup->id );
	}
	const ModelBase* modelbase ( ModelBaseId id ) {
		if ( id >= RID_STATIC_IDS )
			return static_modelbase_store.get ( id );
		return modelbase_store.get ( id );
	}

	IdArray<ModelInstanceBase> modelinstancebase_store;
	IdArray<ModelInstanceBase, RID_STATIC_IDS> static_modelinstancebase_store;
	ModelInstanceBaseId register_modelinstancebase ( DataGroupDefId datagroup_id, ModelBaseId modelbase_id ) {
		ModelInstanceBase modelinstancebase ( datagroup_id, modelbase_id );
		ModelInstanceBaseId id = modelinstancebase_store.insert ( modelinstancebase );
		modelinstancebase_registered ( id );
		return id;
	}
	ModelInstanceBaseId register_modelinstancebase ( const DataGroupDef* datagroup, const ModelBase* modelbase ) {
		return register_modelinstancebase ( datagroup->id, modelbase->id );
	}
	const ModelInstanceBase* modelinstancebase ( ModelInstanceBaseId id ) {
		if ( id >= RID_STATIC_IDS )
			return static_modelinstancebase_store.get ( id );
		return modelinstancebase_store.get ( id );
	}

	virtual void datagroupdef_registered ( DataGroupDefId id ) = 0;
	virtual void contextbase_registered ( ContextBaseId id ) = 0;
	virtual void modelbase_registered ( ModelBaseId id ) = 0;
	virtual void modelinstancebase_registered ( ModelInstanceBaseId id ) = 0;

	virtual InstanceGroup* create_instancegroup() = 0;
	virtual ContextGroup* create_contextgroup() = 0;

	virtual RenderBundle* create_renderbundle ( InstanceGroup* igroup, ContextGroup* cgroup, Array<const RenderStage*>& rstages, Array<ImageType>& image_types, Array<ImageDependency>& dependencies ) = 0;

//---------- Specialized Functions

	virtual RenderBundle* create_main_bundle ( InstanceGroup* igroup, ContextGroup* cgroup ) = 0;
};

Instance* create_instance ( String name );

void destroy_instance ( Instance* instance );
