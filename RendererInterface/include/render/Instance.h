#pragma once

#include "Monitor.h"
#include "Header.h"
#include "Window.h"

struct Device{
	const char* name;
	u64 rating;
	virtual ~Device(){}
};

struct Instance{
	
	virtual ~Instance(){}
	
	virtual Window* create_window() = 0;
	
	virtual bool initialize(Device* device = nullptr) = 0;
	
	virtual bool destroy_window(Window* window) = 0;
	
	virtual void process_events() = 0;
	
	virtual Monitor* get_primary_monitor() = 0;
	
	virtual Array<Monitor*>& get_monitors() = 0;
	virtual Array<Device*>& get_devices() = 0;
};

Instance* initialize_instance(const char* name);

void destroy_instance(Instance* instance);