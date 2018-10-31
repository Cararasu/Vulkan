#pragma once
#include "String.h"



class ResourceManager {
private:
	Map<String, RId> shader_id_map;
	IdArray<Shader> shaders;
public:
	
	virtual StringReference load_vertex_shader (String file);
	virtual StringReference load_fragment_shader (String file);
}