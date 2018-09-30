#pragma once

#include "VulkanInstance.h"
#include "render/Resources.h"

//maybe...
extern ImageFormat format_to_imagetype_map[185];

struct VulkanInstance;

vk::Format transform_image_format ( ImageFormat format );
ImageFormat transform_image_format ( vk::Format format );

struct VulkanBaseImage : public Image {
	VulkanInstance* v_instance;
	vk::Format v_format;
	vk::Extent3D extent;
	vk::Image image;
	vk::ImageView imageview;
	vk::ImageType type;
	vk::ImageTiling tiling;
	vk::ImageUsageFlags usage;
	vk::ImageAspectFlags aspect;

	VulkanBaseImage ( vk::Format format, u32 width, u32 height, u32 depth, u32 layers, u32 mipmap_layers, bool window_target, VulkanInstance* instance,
	                  vk::ImageTiling tiling, vk::ImageUsageFlags usage, vk::ImageAspectFlags aspect );
	virtual ~VulkanBaseImage();
	
	vk::ImageMemoryBarrier transition_image_layout_impl ( vk::ImageLayout oldLayout, vk::ImageLayout newLayout, Range<u32> miprange, Range<u32> arrayrange, vk::PipelineStageFlags* srcStageFlags, vk::PipelineStageFlags* dstStageFlags );

	virtual void transition_image_layout ( vk::ImageLayout newLayout, Range<u32> mip_range, Range<u32> array_range, vk::CommandBuffer commandBuffer ) = 0;
	void transition_layout ( vk::ImageLayout newLayout, vk::CommandBuffer commandBuffer ) {
		transition_image_layout ( newLayout, {0, mipmap_layers}, {0, layers}, commandBuffer );
	}

	virtual void generate_mipmaps ( Range<u32> mip_range, Range<u32> array_range, vk::ImageLayout targetLayout, vk::CommandBuffer commandBuffer ) = 0;
	void gen_mipmaps ( u32 baseLevel, vk::ImageLayout targetLayout, vk::CommandBuffer commandBuffer ) {
		generate_mipmaps ( {baseLevel, mipmap_layers}, {0, layers}, targetLayout, commandBuffer );
	}
	void gen_mipmaps ( vk::ImageLayout targetLayout, vk::CommandBuffer commandBuffer ) {
		generate_mipmaps ( {0, mipmap_layers}, {0, layers}, targetLayout, commandBuffer );
	}
};

struct PerImageData {
	vk::ImageLayout layout;
	vk::Image image;
	vk::ImageView imageview;
};
struct VulkanWindowImage : public VulkanBaseImage {
	u32 current_index;
	StaticArray<PerImageData> per_image_data;

	VulkanWindowImage ( VulkanInstance* instance, u32 imagecount, vk::Image* images, vk::Extent3D extent, u32 layers, vk::Format format );
	virtual ~VulkanWindowImage();

	void set_current_image ( u32 index );
	
	virtual void transition_image_layout ( vk::ImageLayout newLayout, Range<u32> mip_range, Range<u32> array_range, vk::CommandBuffer commandBuffer );
	virtual void generate_mipmaps ( Range<u32> mip_range, Range<u32> array_range, vk::ImageLayout targetLayout, vk::CommandBuffer commandBuffer );
};

struct VulkanImageWrapper : public VulkanBaseImage {
	GPUMemory memory;
	StaticArray<vk::ImageLayout> layouts;

	VulkanImageWrapper ( VulkanInstance* instance, vk::Extent3D extent, u32 layers, u32 mipmap_layers, vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags usage, vk::ImageAspectFlags aspectFlags, vk::MemoryPropertyFlags needed, vk::MemoryPropertyFlags recommended = vk::MemoryPropertyFlags() );
	virtual ~VulkanImageWrapper();

	void destroy();

	//Vulkan-specific stuff
	virtual void transition_image_layout ( vk::ImageLayout newLayout, Range<u32> mip_range, Range<u32> array_range, vk::CommandBuffer commandBuffer );
	virtual void generate_mipmaps ( Range<u32> mip_range, Range<u32> array_range, vk::ImageLayout targetLayout, vk::CommandBuffer commandBuffer );
	
	virtual u32 instance_count (){
		return 1;
	}
	virtual vk::Image instance_image (u32 index){
		return image;
	}
	virtual vk::ImageView instance_imageview (u32 index) {
		return imageview;
	}
};
