#pragma once
#include <render/ResourceManager.h>
#include <render/IdArray.h>
#include "VShader.h"

struct VInstance;
struct VBaseImage;
struct VSampler;

struct VResourceManager : public ResourceManager {
	VInstance* v_instance;
	Map<String, u64> shader_string_id_map;
	IdPtrArray<VShaderModule> shader_array;

	IdPtrArray<VBaseImage> images;
	Map<VBaseImage*, DynArray<VBaseImage*>> dependency_map;

	VResourceManager ( VInstance* instance );
	virtual ~VResourceManager() override;

	virtual u64 load_shader ( ShaderType type, String name, String file ) override;

	virtual ShaderModule* get_shader ( StringReference ref ) override;
	VShaderModule* v_get_shader ( StringReference ref );

	IdPtrArray<VBaseImage> v_images;
	IdPtrArray<VSampler> v_samplers;

	virtual Image* create_texture ( u32 width, u32 height, u32 depth, u32 array_layers, u32 mipmap_layers ) override;
	virtual Image* load_image_to_texture ( std::string file, Image* image, u32 array_layer, u32 mipmap_layer ) override;
	virtual Image* load_image_to_texture ( std::string file, u32 mipmap_layers ) override;

	virtual Image* create_dependant_image ( Image* image, ImageFormat type, u32 mipmap_layers, float scaling ) override;
	VBaseImage* v_create_dependant_image ( VBaseImage* image, ImageFormat type, u32 mipmap_layers, float scaling );
	void v_delete_dependant_images ( VBaseImage* image );
	virtual void delete_image ( Image* image ) override;
	void v_delete_image ( VBaseImage* image );
	
	virtual Sampler* create_sampler(FilterType magnification, FilterType minification, FilterType mipmapping, EdgeHandling u, EdgeHandling v, EdgeHandling w, float lodbias, Range<float> lodrange, float anismax, DepthComparison comp) override;
};
