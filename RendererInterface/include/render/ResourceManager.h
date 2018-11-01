#pragma once
#include "String.h"
#include "Resources.h"


struct ResourceManager {
	
	virtual ~ResourceManager() {}
	//shaders
	virtual u64 load_shader (ShaderType type, String name, String file) = 0;
	
	virtual ShaderModule* get_shader(StringReference ref) = 0;
};