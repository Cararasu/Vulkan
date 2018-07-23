#pragma once

#include "Header.h"
#include "Model.h"
#include "Resources.h"
#include "RenderPass.h"

struct ResourceManager {

	ModelBase* create_modelbase ( ModelBaseDef base );
	ContextBase* create_contextbase ( ContextBaseDef base );
	InstanceBase* create_instancebase ( InstanceBaseDef base );
	
	Model* create_model ( ModelDef model );
	ModelInstance* create_modelinstance ( ModelInstanceDef instance );
	
	RenderPass* create_renderpass ( RenderPassDef renderpass );

	Image* create_depthimage ( ImageFormat format, u32 width, u32 height = 0, u32 depth = 0, u32 layers = 1, u32 mipmaplayers = 1 );
	Image* create_colorimage ( ImageFormat format, u32 width, u32 height = 0, u32 depth = 0, u32 layers = 1, u32 mipmaplayers = 1 );
	Image* create_gbuffer ( ImageFormat format, u32 width, u32 height = 0, u32 depth = 0, u32 layers = 1, u32 mipmaplayers = 1 );

	Resource* get_resource ( const char* string );
	Resource* get_resource ( ResourceHandle handle );
};

