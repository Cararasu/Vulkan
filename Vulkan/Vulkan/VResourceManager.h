#pragma once
#include <render/ResourceManager.h>
#include <render/IdArray.h>
#include "VShader.h"

struct VInstance;

struct VResourceManager : public ResourceManager{
	VInstance* instance;
	Map<String, u64> shader_string_id_map;
	IdPtrArray<VShaderModule> shader_array;
	
	VResourceManager(VInstance* instance);
	virtual ~VResourceManager() override;

	virtual u64 load_shader (ShaderType type, String name, String file) override;
	
	virtual ShaderModule* get_shader(StringReference ref) override;
	VShaderModule* v_get_shader(StringReference ref);
};
