#pragma once

#include "Monitor.h"
#include "Header.h"
#include "Window.h"
#include "Model.h"
#include "RenderPass.h"
#include "ResourceManager.h"

struct Device {
	const char* name;
	u64 rating;
	virtual ~Device() {}
};

struct InstanceOptions{
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
	virtual void prepare_render (Array<Window*> windows) = 0;
	
	virtual void render_bundles(Array<RenderBundle*> bundles) = 0;
	
	virtual Monitor* get_primary_monitor() = 0;

	virtual ResourceManager* resource_manager() = 0;
//------------ Resources
	virtual const DataGroupDef* register_datagroupdef ( Array<DataValueDef> valuedefs, u32 size, u32 arraycount ) = 0;
	virtual const DataGroupDef* datagroupdef ( RId handle ) = 0;

	virtual const ContextBase* register_contextbase ( RId datagroup/*image-defs*/ ) = 0;
	virtual const ContextBase* register_contextbase ( const DataGroupDef* datagroup/*image-defs*/ ) = 0;
	virtual const ContextBase* contextbase ( RId handle ) = 0;

	virtual const ModelBase* register_modelbase ( RId vertexdatagroup ) = 0;
	virtual const ModelBase* register_modelbase ( const DataGroupDef* vertexdatagroup ) = 0;
	virtual const ModelBase* modelbase ( RId handle ) = 0;

	virtual const Model load_generic_model ( RId modelbase, void* vertices, u32 vertexcount, u16* indices, u32 indexcount ) = 0;
	virtual const Model load_generic_model ( const ModelBase* modelbase, void* vertices, u32 vertexcount, u16* indices, u32 indexcount ) = 0;

	virtual const Model load_generic_model ( RId modelbase, void* vertices, u32 vertexcount, u32* indices, u32 indexcount ) = 0;
	virtual const Model load_generic_model ( const ModelBase* modelbase, void* vertices, u32 vertexcount, u32* indices, u32 indexcount ) = 0;

	virtual const ModelInstanceBase* register_modelinstancebase ( Model model, RId datagroup = 0 ) = 0;
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
	virtual const RenderStage* renderstage ( RId handle ) = 0;
	
	virtual InstanceGroup* create_instancegroup() = 0;
	virtual ContextGroup* create_contextgroup() = 0;

	virtual RenderBundle* create_renderbundle(InstanceGroup* igroup, ContextGroup* cgroup, Array<const RenderStage*>& rstages, Array<ImageType>& image_types, Array<ImageDependency>& dependencies) = 0;
	
	
};

Instance* create_instance ( String name );

void destroy_instance ( Instance* instance );
