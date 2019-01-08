#pragma once

#include "VInstance.h"
#include "render/Resources.h"

struct VInstance;
struct VImageWrapper;
struct VImageUse;

vk::Format transform_image_format ( ImageFormat format );
ImageFormat transform_image_format ( vk::Format format );

struct VImageUseRef {
	RId id;
	VBaseImage* image;
	u64 created_frame_index;

	VImageUseRef() : id(0), image(nullptr), created_frame_index(0) {}
	VImageUseRef(ImageUseRef& useref);
	VImageUseRef(RId id, VBaseImage* image, u64 created_frame_index);
	VImageUseRef(const VImageUseRef& rhs);
	VImageUseRef(const VImageUseRef&& rhs);
	~VImageUseRef();
	
	VImageUseRef& operator=(const VImageUseRef& rhs);
	
	operator bool() {
		return id != 0;
	}
	operator!() {
		return id == 0;
	}
	bool is_updated ();
	void set_updated ();
	vk::ImageView imageview();
	VImageUse* deref();
};
struct VImageUse {
	RId id;
	VBaseImage* image;
	Range<u32> mipmaps, layers;
	vk::ImageAspectFlags aspects;
	vk::ImageView imageview;
	std::atomic<u32> refcount;
	
	VImageUse() : id(0), image(nullptr), refcount(0) { }
	VImageUse& operator=(const VImageUse& rhs) {
		id = rhs.id;
		image = rhs.image;
		mipmaps = rhs.mipmaps;
		layers = rhs.layers;
		aspects = rhs.aspects;
		imageview = rhs.imageview;
		refcount.store(rhs.refcount);
	}
	VImageUse(const VImageUse& rhs) {
		id = rhs.id;
		image = rhs.image;
		mipmaps = rhs.mipmaps;
		layers = rhs.layers;
		aspects = rhs.aspects;
		imageview = rhs.imageview;
		refcount.store(rhs.refcount);
	}
	
};

struct VBaseImage : public Image {
	VInstance* v_instance;

	vk::Format v_format;
	vk::Extent3D extent;
	vk::ImageType type;
	vk::ImageTiling tiling;
	vk::ImageUsageFlags usage;
	vk::ImageAspectFlags aspect;
	vk::Image image;
	IdArray<VImageUse> usages;

	//window-image
	vk::Semaphore image_available_sem;
	VWindow *window;
	//own image
	GPUMemory memory;
	u32 current_index;
	bool dependent;
	float fraction;

	VBaseImage ( VInstance* instance, u32 width, u32 height, u32 depth, u32 layers, u32 mipmap_layers,
	             vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags usage, vk::ImageAspectFlags aspect, vk::MemoryPropertyFlags needed, vk::MemoryPropertyFlags recommended = vk::MemoryPropertyFlags() );

	VBaseImage ( VInstance* instance, VWindow* window );
	virtual ~VBaseImage() override;

	void init();
	void init ( vk::Image image );
	void destroy();

	void rebuild_image ( u32 width, u32 height, u32 depth );
	void set_current_image ( u32 index );

	VImageUseRef v_create_use ( vk::ImageAspectFlags aspects, Range<u32> mipmaps, Range<u32> layers );
	void v_create_imageview ( VImageUse* imageuse );

	void v_delete_use ( RId id ) {
		v_instance->destroyImageView ( usages[id].imageview );
		usages.remove ( id );
	}
	
	virtual ImageUseRef create_use ( ImagePart part, Range<u32> mipmaps, Range<u32> layers ) override;
	void v_register_use(RId id) {
		if(!id) return;
		usages[id].refcount++;
	}
	void v_deregister_use(RId id) {
		if(!id) return;
		if(--usages[id].refcount == 0) {
			v_delete_use (id);
		}
	}
	virtual void register_use(RId id) override {
		v_register_use(id);
	}
	virtual void deregister_use(RId id) override {
		v_deregister_use(id);
	}


	void v_set_extent ( u32 width, u32 height, u32 depth );
	void v_set_format ( vk::Format format );

	vk::ImageMemoryBarrier transition_layout_impl ( vk::ImageLayout oldLayout, vk::ImageLayout newLayout,
	        Range<u32> miprange, Range<u32> arrayrange, u32 instance_index,
	        vk::PipelineStageFlags* srcStageFlags, vk::PipelineStageFlags* dstStageFlags );
	vk::ImageMemoryBarrier transition_layout_impl ( vk::ImageLayout* oldLayout, vk::ImageLayout* newLayout,
	        Range<u32> miprange, Range<u32> arrayrange, u32 instance_index,
	        vk::PipelineStageFlags* srcStageFlags, vk::PipelineStageFlags* dstStageFlags );

	void transition_layout ( vk::ImageLayout oldLayout, vk::ImageLayout newLayout, Range<u32> mip_range, Range<u32> array_range, vk::CommandBuffer commandBuffer, u32 instance_index = 0 );
	void transition_layout ( vk::ImageLayout* oldLayout, vk::ImageLayout* newLayout, Range<u32> mip_range, Range<u32> array_range, vk::CommandBuffer commandBuffer, u32 instance_index = 0 );
	void transition_layout ( vk::ImageLayout oldLayout, vk::ImageLayout newLayout, vk::CommandBuffer commandBuffer, u32 instance_index = 0 ) {
		transition_layout ( oldLayout, newLayout, {0, mipmap_layers}, {0, layers}, commandBuffer, instance_index );
	}

	void generate_mipmaps ( vk::ImageLayout oldLayout, vk::ImageLayout newLayout, Range<u32> mip_range, Range<u32> array_range, vk::CommandBuffer commandBuffer );
	void generate_mipmaps ( vk::ImageLayout* oldLayout, vk::ImageLayout* newLayout, Range<u32> mip_range, Range<u32> array_range, vk::CommandBuffer commandBuffer );
	void generate_mipmaps ( vk::ImageLayout oldLayout, vk::ImageLayout newLayout, u32 baseLevel, vk::CommandBuffer commandBuffer ) {
		generate_mipmaps ( oldLayout, newLayout, {baseLevel, mipmap_layers}, {0, layers}, commandBuffer );
	}
	void generate_mipmaps ( vk::ImageLayout oldLayout, vk::ImageLayout newLayout, vk::CommandBuffer commandBuffer ) {
		generate_mipmaps ( oldLayout, newLayout, {0, mipmap_layers}, {0, layers}, commandBuffer );
	}
};


inline VImageUseRef::VImageUseRef(ImageUseRef& useref) : id(useref.id), image(static_cast<VBaseImage*>(useref.base_image)), created_frame_index(0) {
	image->v_register_use(id);
}
inline VImageUseRef::VImageUseRef(RId id, VBaseImage* image, u64 created_frame_index) : id(id), image(image), created_frame_index(created_frame_index) {
	image->v_register_use(id);
}
inline VImageUseRef::VImageUseRef(const VImageUseRef& rhs) : id(rhs.id), image(rhs.image), created_frame_index(rhs.created_frame_index) {
	image->v_register_use(id);
}
inline VImageUseRef::VImageUseRef(const VImageUseRef&& rhs) : id(rhs.id), image(rhs.image), created_frame_index(rhs.created_frame_index) {
	
}
inline VImageUseRef& VImageUseRef::operator=(const VImageUseRef& rhs) {
	image->v_deregister_use(id);
	id = rhs.id;
	image = rhs.image;
	created_frame_index = rhs.created_frame_index;
	image->v_register_use(id);
	return *this;
}
inline VImageUseRef::~VImageUseRef() {
	image->v_deregister_use(id);
}
inline VImageUse* VImageUseRef::deref() {
	return id ? &image->usages[id] : nullptr;
}
inline bool VImageUseRef::is_updated () {
	return id ? image->created_frame_index > created_frame_index : false;
}
inline void VImageUseRef::set_updated () {
	created_frame_index = image->created_frame_index;
}
inline vk::ImageView VImageUseRef::imageview() {
	return id ? image->usages[id].imageview : vk::ImageView();
}
