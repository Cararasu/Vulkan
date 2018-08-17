#pragma once

#include "Header.h"
#include <atomic>

//World
//    contextid -> context
//    modelid -> instances

//Context
//    contextid
//    contextdata
//    
/*
//type, count, datadef_id, 
struct RenderContext{
	u32 id;
	Array<u32> data_def_ids;
	Array<u32> data_def_ids;other stuff
};*/

template<typename IT>
struct ObjectInstance {
	glm::vec3 pos;
	float radius;
	IT instance;
};

//IdArray

//Vertex, VertexDef, VertexId
//Model, ModelDef, ModelId

//Context, ContextDef, ContextId

//TypeDef
//    Array of ValueDef

//Textures
//	ImageType


struct Image;
//contiguous GPU memory
struct Buffer;
struct RenderPass;

struct Resource {
	ResourceHandle handle;
	//String sid
	//ResourceURI
	std::atomic<bool> loading;
	std::atomic<bool> loaded;

	virtual Image* cast_to_image() {
		return nullptr;
	}
	virtual Buffer* cast_to_buffer() {
		return nullptr;
	}
	virtual RenderPass* cast_to_renderpass() {
		return nullptr;
	}
};

//stores the objects to dispatch
struct DispatchQueue {

};

struct Image : public Resource {
	const ImageFormat format;
	const u32 width, height, depth;
	const u32 layers, mipmap_layers;
	const bool window_target;

	Image ( ImageFormat format, u32 width, u32 height, u32 depth, u32 layers, u32 mipmap_layers, bool window_target ) :
		format ( format ), width ( width ), height ( height ), depth ( depth ), layers ( layers ), mipmap_layers ( mipmap_layers ), window_target ( window_target ) {

	}

	virtual Image* cast_to_image() {
		return this;
	}
};
struct Buffer : public Resource {

	virtual Buffer* cast_to_buffer() {
		return this;
	}
};
