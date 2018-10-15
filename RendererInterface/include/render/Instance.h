#pragma once

#include "Monitor.h"
#include "Header.h"
#include "Window.h"
#include "Model.h"
#include "RenderPass.h"

struct Device {
	const char* name;
	u64 rating;
	virtual ~Device() {}
};

struct Instance {
	Array<Monitor*> monitors;
	Array<Device*> devices;
	Array<Window*> windows;
	
	virtual ~Instance() {}

	virtual bool initialize ( Device* device = nullptr ) = 0;

	virtual Window* create_window() = 0;
	virtual bool destroy_window ( Window* window ) = 0;

	virtual void process_events() = 0;
	virtual void render_window ( Window* window ) = 0;
	virtual void render_windows() = 0;
	virtual bool is_window_open() = 0;

	virtual Monitor* get_primary_monitor() = 0;


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

	virtual const Renderer* create_renderer (
	    const ModelInstanceBase* model_instance_base,
	    const Array<const ContextBase*> context_bases/*,
		StringReference vertex_shader,
		StringReference fragment_shader,
		StringReference geometry_shader,
		StringReference tesselation_control_shader,
		StringReference tesselation_evaluation_shader*/
	) = 0;
	virtual const Renderer* renderer ( RId handle ) = 0;

	virtual const RenderStage* create_renderstage ( Array<const Renderer*> renderers, Array<void*> dependencies, Array<void*> inputs, Array<void*> outputs, Array<void*> temporaries ) = 0;
	virtual const RenderStage* renderstage ( RId handle ) = 0;
	
	virtual InstanceGroup* create_instancegroup() = 0;
	virtual ContextGroup* create_contextgroup() = 0;

	virtual RenderBundle* create_renderbundle(InstanceGroup* igroup, ContextGroup* cgroup, const RenderStage* rstage, void* targets) = 0;
	
	virtual void render_bundles(Array<RenderBundle*> bundles) = 0;
	
};

Instance* create_instance ( const char* name );

void destroy_instance ( Instance* instance );
