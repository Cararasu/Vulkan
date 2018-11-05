#pragma once

#include "Header.h"
#include <atomic>

template<typename IT>
struct ObjectInstance {
	glm::vec3 pos;
	float radius;
	IT instance;
};

struct Image;
//contiguous GPU memory
struct Buffer;
struct RenderPass;

struct StringReference {
	const String name;
	const u64 id;
	
	StringReference(const String& name) : name(name), id(0){}
	StringReference(const String&& name) : name(name), id(0){}
	StringReference(const String name, u64 id) : name(name), id(id){}
};

inline bool operator==(const StringReference& lhs, const StringReference& rhs){
	if(lhs.id) return lhs.id == rhs.id;
	else return lhs.name == rhs.name;
}


struct Resource {
	u64 created_frame_index;
	RId id;
	//String sid
	//ResourceURI
	std::atomic<bool> loading;
	std::atomic<bool> loaded;

};

//stores the objects to dispatch
struct DispatchQueue {

};

struct ImageDependency {
private:
	u64 id;
	u32 width, height, depth;
	u64 last_change_frame;
public:
	virtual void add_dependentcy(Image* image, float factor = 1.0f) = 0;
	virtual void remove_dependency(Image* image, float factor = 1.0f) = 0;
};

enum class RenderImageType {
	eColorR8,
	eColorRG8,
	eColorRGB8,
	eColorRGBA8,
	eDepthImage,
	eStencilImage,
	eDepthStencilImage,
};

struct Image : public Resource {
	ImageFormat format;
	u32 width, height, depth;
	u32 layers;//arra-layers
	u32 mipmap_layers;
	const bool window_target;

	Image ( ImageFormat format, u32 width, u32 height, u32 depth, u32 layers, u32 mipmap_layers, bool window_target ) :
		format ( format ), width ( width ), height ( height ), depth ( depth ), layers ( layers ), mipmap_layers ( mipmap_layers ), window_target ( window_target ) {

	}
	virtual ~Image() {}
};
struct Buffer : public Resource {

};

enum class ShaderType {
	eVertex,
	eTessEval,
	eTessControl,
	eGeometry,
	eFragment,
	eCompute
};

struct ShaderModule : public Resource {
	ShaderType shadertype;
	String shadername;
};