#pragma once

#include "VInstance.h"
#include "render/Resources.h"

struct VInstance;
struct VImageWrapper;

vk::Format transform_image_format ( ImageFormat format );
ImageFormat transform_image_format ( vk::Format format );

struct VImageUse {
	RId id = 0;
	VBaseImage* image = nullptr;
	Range<u32> mipmaps, layers;
	vk::ImageView imageview;
	vk::ImageAspectFlags aspects;
	u64 created_frame_index;

	void destroy();
	operator bool() {
		return id != 0;
	}
	bool equals(VImageUse& rhs){
		return image == rhs.image && mipmaps == rhs.mipmaps && layers == rhs.layers && aspects == rhs.aspects;
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

	virtual ImageUse create_use(ImagePart part, Range<u32> mipmaps, Range<u32> layers) override;
	virtual void delete_use(RId id) {
		v_delete_use(id);
	}
	
	VImageUse v_create_use(ImagePart part, Range<u32> mipmaps, Range<u32> layers);
	void v_create_use(VImageUse* imageuse);
	
	void v_delete_use(RId id) {
		v_instance->destroyImageView(usages[id].imageview);
		usages.remove(id);
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
