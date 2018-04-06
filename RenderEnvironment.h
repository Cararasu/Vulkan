#ifndef RENDERENVIRONMENT_H
#define RENDERENVIRONMENT_H

#include <vector>
#include <assert.h>
#include <stdint.h>
#include <Dispatcher.h>

//load to RAM -> ram_Data -> group together -> upload to GPU -> BufferedData

template<typename VERTEXDATA, typename INDEXTYPE>
struct ObjectVertexData{
	std::vector<VERTEXDATA> vertices;
	std::vector<INDEXTYPE> indices;
};

template<typename VERTEXDATA, typename INDEXTYPE>
struct LoadedObjectData{
	uint32_t vertex_offset, index_offset, index_size;
	uint32_t diffuseTexId;
};

template<typename VERTEXDATA, typename INDEXTYPE>
struct LoadedObjectWrapper{
	ObjectVertexData<VERTEXDATA, INDEXTYPE> vertex_data;
	std::vector<LoadedObjectData<VERTEXDATA, INDEXTYPE>> object_def;
	bool loaded = false;
	BufferWrapper* vertexData;
	BufferWrapper* indexData;
};


extern struct RenderEnvironment{
	
	std::vector<LoadedObjectWrapper<Vertex, uint32_t>> objects;
	
	//finish the frame
	void finish(){
		//Not Implemented
		assert(false);
	}
	
} g_render_environment;


struct ThreadRenderEnvironment{
	OpaqueObjectDispatcher dispatcher;
	RenderEnvironment* render_environment;
	
	void init(RenderEnvironment* rendEnv, VInstance* instance){
		render_environment = rendEnv;
		dispatcher.init(instance);
	}
	
	void reset(){
		dispatcher.reset_instances();
	}
	
	void finit(VInstance* instance){
		dispatcher.finit(instance);
	}
};

extern thread_local ThreadRenderEnvironment g_thread_data;

#endif // RENDERENVIRONMENT_H
