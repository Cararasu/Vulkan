#pragma once

#include "Monitor.h"
#include "Header.h"
#include "Window.h"

struct Instance{
	
	virtual Monitor* get_primary_monitor() = 0;
	
	virtual Array<Monitor*>& get_monitors() = 0;
	
	virtual Window* createWindow() = 0;
};

Instance* initialize_instance();

void destroy_instance(Instance* instance);