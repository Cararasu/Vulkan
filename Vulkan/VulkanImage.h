#pragma once
#include "VulkanInstance.h"


struct VulkanImageWrapper {
	VulkanInstance* instance;
	GPUMemory memory;
	vk::Image image;
	vk::Extent3D extent;
	u32 mipMapLevels;
	u32 arraySize;
	vk::Format format;
	vk::ImageTiling tiling;
	vk::ImageUsageFlags usage;
	vk::ImageType type;
	StaticArray<vk::ImageLayout> layouts;
	vk::ImageAspectFlags aspectFlags;
	vk::MemoryPropertyFlags needed;
	vk::MemoryPropertyFlags recommended;

	VulkanImageWrapper() {}

	VulkanImageWrapper ( VulkanInstance* instance, vk::Extent3D extent, u32 mipMapLevels, u32 arraySize, vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags usage, vk::ImageAspectFlags aspectFlags, vk::MemoryPropertyFlags needed, vk::MemoryPropertyFlags recommended = vk::MemoryPropertyFlags() );
	VulkanImageWrapper ( VulkanInstance* instance, vk::Image image, vk::Extent3D extent, u32 mipMapLevels, u32 arraySize, vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags usage, vk::ImageAspectFlags aspectFlags );
	virtual ~VulkanImageWrapper();

	void create();
	void destroy();

	//Vulkan-specific stuff
	vk::ImageMemoryBarrier transition_image_layout_impl ( vk::ImageLayout oldLayout, vk::ImageLayout newLayout, Range<u32> miprange, Range<u32> arrayrange, vk::PipelineStageFlags* srcStageFlags, vk::PipelineStageFlags* dstStageFlags );

	void transition_image_layout ( vk::ImageLayout newLayout, Range<u32> mip_range, Range<u32> array_range, vk::CommandBuffer commandBuffer );

	inline void transition_image_layout ( vk::ImageLayout newLayout, vk::CommandBuffer commandBuffer ) {
		transition_image_layout ( newLayout, {0, mipMapLevels}, {0, arraySize}, commandBuffer );
	}

	void generate_mipmaps ( Range<u32> mip_range, Range<u32> array_range, vk::ImageLayout targetLayout, vk::CommandBuffer commandBuffer );

	inline void generate_mipmaps ( u32 baseLevel, vk::ImageLayout targetLayout, vk::CommandBuffer commandBuffer ) {
		generate_mipmaps ( {baseLevel, mipMapLevels}, {0, arraySize}, targetLayout, commandBuffer );
	}
	inline void generate_mipmaps ( vk::ImageLayout targetLayout, vk::CommandBuffer commandBuffer ) {
		generate_mipmaps ( {0, mipMapLevels}, {0, arraySize}, targetLayout, commandBuffer );
	}
};

struct VulkanSingleImage {
	VulkanImageWrapper imagewrapper;
	vk::ImageView imageview;

	VulkanSingleImage ( VulkanInstance* instance, vk::Extent3D extent, u32 mipMapLevels, u32 arraySize, vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags usage, vk::ImageAspectFlags aspectFlags, vk::MemoryPropertyFlags needed, vk::MemoryPropertyFlags recommended = vk::MemoryPropertyFlags() ) :
		imagewrapper ( instance, extent, mipMapLevels, arraySize, format, tiling, usage, aspectFlags, needed, recommended ), imageview() {

	}

	RendResult resize ( u32 width, u32 height = 0, u32 depth = 0 ) {
		if ( height == 0 ) {
			if ( imagewrapper.type != vk::ImageType::e1D )
				return RendResult::eFail;
		} else if ( depth == 0 ) {
			if ( imagewrapper.type != vk::ImageType::e2D )
				return RendResult::eFail;
		} else {
			if ( imagewrapper.type != vk::ImageType::e3D )
				return RendResult::eFail;
		}
		imagewrapper.destroy();
		if ( height == 0 ) {
			imagewrapper.extent.width = width;
		} else if ( depth == 0 ) {
			imagewrapper.extent.width = width;
			imagewrapper.extent.height = height;
		} else {
			imagewrapper.extent.width = width;
			imagewrapper.extent.height = height;
			imagewrapper.extent.depth = depth;
		}
		imagewrapper.create();
	}
};
