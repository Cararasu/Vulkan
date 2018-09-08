#pragma once

#include "Monitor.h"
#include "Header.h"
#include "Window.h"
#include "Model.h"

struct Device{
	const char* name;
	u64 rating;
	virtual ~Device(){}
};

struct Instance{
	
	virtual ~Instance(){}
	
	virtual bool initialize(Device* device = nullptr) = 0;
	
	virtual Window* create_window() = 0;
	virtual bool destroy_window(Window* window) = 0;
	
	virtual void process_events() = 0;
	virtual void render_window(Window* window) = 0;
	virtual void render_windows() = 0;
	virtual bool is_window_open() = 0;
	
	virtual Monitor* get_primary_monitor() = 0;
	
	virtual Array<Monitor*>& get_monitors() = 0;
	virtual Array<Device*>& get_devices() = 0;
	
	
//------------ Resources
	virtual RId register_datagroupdef ( DataGroupDef& datagroupdef ) = 0;
	virtual const DataGroupDef* datagroupdef ( RId handle ) = 0;
	
	virtual RId register_contextbase ( RId datagroup/*image-defs*/ ) = 0;
	virtual const ContextBase contextbase ( RId handle ) = 0;
	
	virtual RId register_modelbase ( RId vertexdatagroup ) = 0;
	virtual const ModelBase modelbase ( RId handle ) = 0;
	
	virtual const Model load_generic_model ( RId modelbase, u8* vertices, u32 vertexcount, u16* indices, u32 indexcount ) = 0;
	virtual const Model load_generic_model ( RId modelbase, u8* vertices, u32 vertexcount, u32* indices, u32 indexcount ) = 0;
	
	virtual RId register_modelinstancebase ( Model model, RId datagroup = 0 ) = 0;
	virtual const ModelInstanceBase modelinstancebase ( RId handle ) = 0;
	
	virtual const ModelInstance create_instance( RId modelinstancebase ) = 0;
	
};

Instance* initialize_instance(const char* name);

void destroy_instance(Instance* instance);