#pragma once

#include "Header.h"
#include "Model.h"
#include "Resources.h"

struct ResourceManager {

	ModelBase* create_modelbase();
	Model* create_model ( ModelBase* base );
	ModelInstance* create_modelinstance ( Model* model, InstanceBase* base );

	Image* create_depthimage ( ImageFormat format, u32 width, u32 height = 0, u32 depth = 0, u32 layers = 1, u32 mipmaplayers = 1 );
	Image* create_colorimage ( ImageFormat format, u32 width, u32 height = 0, u32 depth = 0, u32 layers = 1, u32 mipmaplayers = 1 );
	Image* create_gbuffer ( ImageFormat format, u32 width, u32 height = 0, u32 depth = 0, u32 layers = 1, u32 mipmaplayers = 1 );

	Resource* get_resource ( const char* string );
	Resource* get_resource ( ResourceHandle handle );
};
