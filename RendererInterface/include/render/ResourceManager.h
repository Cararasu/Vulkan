#pragma once
#include "String.h"
#include "Resources.h"


struct ResourceManager {
	
	virtual ~ResourceManager() {}
	//shaders
	virtual u64 load_shader (ShaderType type, String name, String file) = 0;
	
	virtual ShaderModule* get_shader(StringReference ref) = 0;
	
	//Images
	virtual Image* create_texture (u32 width, u32 height, u32 depth, u32 array_layers, u32 mipmap_layers, ImageFormat format) = 0;
	virtual Image* load_image_to_texture(std::string file, Image* image, u32 array_layer, u32 mipmap_layer) = 0;
	virtual Image* load_image_to_texture(std::string file, u32 mipmap_layers) = 0;
	
	virtual Image* create_dependant_image(Image* image, ImageFormat type, u32 mipmap_layers, float scaling) = 0;
	virtual void delete_image(Image* image) = 0;
	
	virtual Sampler* create_sampler(FilterType magnification, FilterType minification, FilterType mipmapping, EdgeHandling u, EdgeHandling v, EdgeHandling w, float lodbias, Range<float> lodrange, float anismax, DepthComparison comp) = 0;
};