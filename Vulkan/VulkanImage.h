#pragma once

#include "VulkanInstance.h"
#include "render/Resources.h"

//maybe...
extern ImageFormat format_to_imagetype_map[185];


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
	                  vk::ImageTiling tiling, vk::ImageUsageFlags usage, vk::ImageAspectFlags aspect ) :
		Image ( transform_image_format ( format ), width, height, depth, layers, mipmap_layers, window_target ), v_instance ( instance ), v_format ( format ),
		tiling ( tiling ), usage ( usage ), aspect ( aspect ) {
		if ( height == 0 ) {
			type = vk::ImageType::e1D;
			extent = vk::Extent3D ( width, 1, 1 );
		} else if ( depth == 0 ) {
			type = vk::ImageType::e2D;
			extent = vk::Extent3D ( width, height, 1 );
		} else {
			type = vk::ImageType::e3D;
			extent = vk::Extent3D ( width, height, depth );
		}
	}
	virtual ~VulkanBaseImage() { }
	/*
		vk::ImageView get_whole_image_view();
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
	*/
};

struct PerImageData {
	vk::ImageLayout layout;
	vk::Image image;
	vk::ImageView imageview;
};
struct VulkanWindowImage : public VulkanBaseImage {
	u32 current_index;
	StaticArray<PerImageData> per_image_data;

	VulkanWindowImage ( VulkanInstance* instance, u32 imagecount, vk::Image* images, vk::Extent3D extent, u32 layers, vk::Format format ) :
		VulkanBaseImage ( format, extent.width, extent.height, extent.depth, layers, 1, true, instance, vk::ImageTiling::eOptimal,
		                  vk::ImageUsageFlags ( vk::ImageUsageFlagBits::eColorAttachment ), vk::ImageAspectFlags ( vk::ImageAspectFlagBits::eColor ) ),
		current_index ( 0 ), per_image_data ( imagecount ) {
		for ( size_t i = 0; i < imagecount; i++ ) {
			per_image_data[i].layout = vk::ImageLayout::eUndefined;
			per_image_data[i].image = images[i];
			per_image_data[i].imageview = instance->createImageView2D ( images[i], 0, 1, format, vk::ImageAspectFlags ( vk::ImageAspectFlagBits::eColor ) );
		}
		image = per_image_data[current_index].image;
		imageview = per_image_data[current_index].imageview;
	}
	virtual ~VulkanWindowImage() {
		for ( PerImageData& data : per_image_data ) {
			v_instance->destroyImageView ( data.imageview );
		}
	}

	void set_current_image ( u32 index ) {
		current_index = index;
		image = per_image_data[current_index].image;
		imageview = per_image_data[current_index].imageview;
	}
	virtual u32 instance_count (){
		return per_image_data.size();
	}
	virtual vk::Image instance_image (u32 index){
		return per_image_data[index].image;
	}
	virtual vk::ImageView instance_imageview (u32 index) {
		return per_image_data[index].imageview;
	}
};

struct VulkanImageWrapper : public VulkanBaseImage {
	GPUMemory memory;
	vk::ImageType type;
	StaticArray<vk::ImageLayout> layouts;

	VulkanImageWrapper ( VulkanInstance* instance, vk::Extent3D extent, u32 layers, u32 mipmap_layers, vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags usage, vk::ImageAspectFlags aspectFlags, vk::MemoryPropertyFlags needed, vk::MemoryPropertyFlags recommended = vk::MemoryPropertyFlags() );
	virtual ~VulkanImageWrapper();

	void destroy();

	//Vulkan-specific stuff
	vk::ImageMemoryBarrier transition_image_layout_impl ( vk::ImageLayout oldLayout, vk::ImageLayout newLayout, Range<u32> miprange, Range<u32> arrayrange, vk::PipelineStageFlags* srcStageFlags, vk::PipelineStageFlags* dstStageFlags );

	void transition_image_layout ( vk::ImageLayout newLayout, Range<u32> mip_range, Range<u32> array_range, vk::CommandBuffer commandBuffer );
	inline void transition_image_layout ( vk::ImageLayout newLayout, vk::CommandBuffer commandBuffer ) {
		transition_image_layout ( newLayout, {0, mipmap_layers}, {0, layers}, commandBuffer );
	}

	void generate_mipmaps ( Range<u32> mip_range, Range<u32> array_range, vk::ImageLayout targetLayout, vk::CommandBuffer commandBuffer );
	inline void generate_mipmaps ( u32 baseLevel, vk::ImageLayout targetLayout, vk::CommandBuffer commandBuffer ) {
		generate_mipmaps ( {baseLevel, mipmap_layers}, {0, layers}, targetLayout, commandBuffer );
	}
	inline void generate_mipmaps ( vk::ImageLayout targetLayout, vk::CommandBuffer commandBuffer ) {
		generate_mipmaps ( {0, mipmap_layers}, {0, layers}, targetLayout, commandBuffer );
	}
	
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
