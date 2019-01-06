#pragma once

#include "Header.h"
#include "Dimensions.h"
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

	StringReference ( const String& name ) : name ( name ), id ( 0 ) {}
	StringReference ( const String&& name ) : name ( name ), id ( 0 ) {}
	StringReference ( const String name, u64 id ) : name ( name ), id ( id ) {}
};

inline bool operator== ( const StringReference& lhs, const StringReference& rhs ) {
	if ( lhs.id ) return lhs.id == rhs.id;
	else return lhs.name == rhs.name;
}


struct Resource {
	RId id;
	u64 created_frame_index;
	//String sid
	//ResourceURI
	std::atomic<bool> loading;
	std::atomic<bool> loaded;

};

//stores the objects to dispatch
struct DispatchQueue {

};

struct ImageTypeGroup {
	u32 width, height, depth;
	u32 dims;
	ImageFormat format;
	//sampling-options
};

struct ImageUseRef {
	RId id = 0;
	Image* base_image = nullptr;
	
	ImageUseRef(RId id, Image* base_image);
	~ImageUseRef();
	
	void detach();
};

struct Image : public Resource {
	ImageFormat format;
	u32 width, height, depth;
	u32 layers;//array-layers
	u32 mipmap_layers;

	const bool window_target;

	Image ( ImageFormat format, u32 width, u32 height, u32 depth, u32 layers, u32 mipmap_layers, bool window_target ) :
		format ( format ), width ( width ), height ( height ), depth ( depth ), layers ( layers ), mipmap_layers ( mipmap_layers ),
		window_target ( window_target ) {
	}
	
	virtual ~Image() {}
	
	virtual ImageUseRef create_use(ImagePart part, Range<u32> mipmaps, Range<u32> layers) = 0;
	virtual void register_use(RId id) = 0;
	virtual void deregister_use(RId id) = 0;
};

inline ImageUseRef::ImageUseRef(RId id, Image* base_image) : id(id), base_image(base_image) {
	base_image->register_use(id);
}
inline ImageUseRef::~ImageUseRef() {
	base_image->deregister_use(id);
}
inline void ImageUseRef::detach() {
	base_image->deregister_use(id);
	id = 0;
	base_image = nullptr;
}
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
