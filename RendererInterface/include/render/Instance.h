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
	const DataGroupDefId register_datagroupdef ( Array<DataValueDef> valuedefs, u32 size, u32 arraycount ) {
		DataGroupDef datagroupdef ( valuedefs, size, arraycount );
		return datagroup_store.insert ( datagroupdef );
	}
	const DataGroupDef* datagroupdef ( DataGroupDefId id ) {
		if ( id >= RID_STATIC_IDS )
			return static_datagroup_store.get ( id );
		return datagroup_store.get ( id );
	}

	IdArray<ContextBase> contextbase_store;
	IdArray<ContextBase, RID_STATIC_IDS> static_contextbase_store;
	const ContextBaseId register_contextbase ( DataGroupDef* datagroup ) {
		ContextBase contextbase ( datagroup );
		return contextbase_store.insert ( contextbase );
	}
	const ContextBaseId register_contextbase ( DataGroupDefId datagroup_id ) {
		return register_contextbase ( &datagroup_store[datagroup_id] );
	}
	const ContextBase* contextbase ( ContextBaseId id ) {
		if ( id >= RID_STATIC_IDS )
			return static_contextbase_store.get ( id );
		return contextbase_store.get ( id );
	}

	virtual const ModelBase* register_modelbase ( DataGroupDefId vertexdatagroup ) = 0;
	virtual const ModelBase* register_modelbase ( const DataGroupDef* vertexdatagroup ) = 0;
	virtual const ModelBase* modelbase ( RId handle ) = 0;

	virtual const ModelInstanceBase* register_modelinstancebase ( Model model, DataGroupDefId datagroup = 0 ) = 0;
	virtual const ModelInstanceBase* register_modelinstancebase ( Model model, const DataGroupDef* datagroup = nullptr ) = 0;
	virtual const ModelInstanceBase* modelinstancebase ( RId handle ) = 0;

	virtual const RenderStage* create_renderstage (
	    const ModelInstanceBase* model_instance_base,
	    const Array<const ContextBase*> context_bases,
	    StringReference vertex_shader,
	    StringReference fragment_shader,
	    StringReference geometry_shader,
	    StringReference tess_cntrl_shader,
	    StringReference tess_eval_shader,
	    Array<RenderImageDef> input_image_defs ) = 0;
	//virtual const RenderStage* create_present_renderstage ( Window* window ) = 0;
	virtual const RenderStage* renderstage ( RId handle ) = 0;

	virtual InstanceGroup* create_instancegroup() = 0;
	virtual ContextGroup* create_contextgroup() = 0;

	virtual RenderBundle* create_renderbundle ( InstanceGroup* igroup, ContextGroup* cgroup, Array<const RenderStage*>& rstages, Array<ImageType>& image_types, Array<ImageDependency>& dependencies ) = 0;

//---------- Specialized Functions

	virtual RenderBundle* create_main_bundle ( InstanceGroup* igroup, ContextGroup* cgroup ) = 0;
};

Instance* create_instance ( String name );

void destroy_instance ( Instance* instance );
