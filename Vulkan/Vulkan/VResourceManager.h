#pragma once
#include <render/ResourceManager.h>
#include <render/IdArray.h>
#include "VShader.h"

struct VInstance;
struct VImageWrapper;
struct VBaseImage;

struct VResourceManager : public ResourceManager{
	VInstance* instance;
	Map<String, u64> shader_string_id_map;
	IdPtrArray<VShaderModule> shader_array;
	
	DynArray<VImageWrapper*> images;
	Map<VBaseImage*, DynArray<VImageWrapper*>> dependency_map;
	
	VResourceManager(VInstance* instance);
	virtual ~VResourceManager() override;

	virtual u64 load_shader (ShaderType type, String name, String file) override;
	
	virtual ShaderModule* get_shader(StringReference ref) override;
	VShaderModule* v_get_shader(StringReference ref);
	
	virtual Image* create_dependant_image(Image* image, ImageFormat type, float scaling) override;
	VImageWrapper* v_create_dependant_image(VBaseImage* image, ImageFormat type, float scaling);
	void v_delete_dependant_images(VBaseImage* image);
	virtual void delete_image(Image* image) override;
};
