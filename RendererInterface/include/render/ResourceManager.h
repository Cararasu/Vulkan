#pragma once

#include "Header.h"
#include "Model.h"
#include "Resources.h"
#include "RenderPass.h"
#include "IdArray.h"

struct ResourceManager {

	virtual RId register_datagroupdef ( DataGroupDef& datagroupdef ) = 0;
	virtual const DataGroupDef* datagroupdef ( RId handle ) = 0;
	
	virtual RId register_contextbase ( RId datagroup/*image-defs*/ ) = 0;
	virtual const ContextBase contextbase ( RId handle ) = 0;
	
	virtual RId register_modelbase ( RId vertexdatagroup ) = 0;
	virtual const ModelBase modelbase ( RId handle ) = 0;
	
	virtual const Model load_generic_model ( RId modelbase, u8* vertices, u32 vertexcount, u16* indices, u32 indexcount ) = 0;
	virtual const Model load_generic_model ( RId modelbase, u8* vertices, u32 vertexcount, u32* indices, u32 indexcount ) = 0;
	
	virtual RId register_modelinstancebase ( Model model, RId context = 0 ) = 0;
	virtual const ModelInstanceBase modelinstancebase ( RId handle ) = 0;
	
	virtual const ModelInstance create_instance( RId modelinstancebase ) = 0;

	Image* create_depthimage ( ImageFormat format, u32 width, u32 height = 0, u32 depth = 0, u32 layers = 1, u32 mipmaplayers = 1 );
	Image* create_colorimage ( ImageFormat format, u32 width, u32 height = 0, u32 depth = 0, u32 layers = 1, u32 mipmaplayers = 1 );
	Image* create_gbuffer ( ImageFormat format, u32 width, u32 height = 0, u32 depth = 0, u32 layers = 1, u32 mipmaplayers = 1 );

};
