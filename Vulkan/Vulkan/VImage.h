#pragma once

#include "VInstance.h"
#include "render/Resources.h"

struct VInstance;
struct VImageWrapper;

vk::Format transform_image_format ( ImageFormat format );
ImageFormat transform_image_format ( vk::Format format );

struct PerImageData {
	vk::Image image;
	vk::ImageView imageview;
	vk::ImageView depth_imageview;
	vk::ImageView stencil_imageview;
};

struct VBaseImage : public Image {
	VInstance* v_instance;

	vk::Format v_format;
	vk::Extent3D extent;
	vk::ImageType type;
	vk::ImageTiling tiling;
	vk::ImageUsageFlags usage;
	vk::ImageAspectFlags aspect;
	DynArray<PerImageData> per_image_data;

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
	void init ( DynArray<vk::Image>& images );
	void destroy();

	void rebuild_image ( u32 width, u32 height, u32 depth );
	void set_current_image ( u32 index );

	void fetch_new_window_images ( );

	void v_set_extent ( u32 width, u32 height, u32 depth );
	void v_set_format ( vk::Format format );

	u32 instance_count () {
		return per_image_data.size();
	}
	vk::Image instance_image ( u32 index ) {
		return per_image_data[index % per_image_data.size()].image;
	}
	vk::Image instance_image ( ) {
		return per_image_data[current_index].image;
	}
	vk::ImageView instance_imageview ( u32 index ) {
		return per_image_data[index % per_image_data.size()].imageview;
	}
	vk::ImageView instance_imageview ( ) {
		return per_image_data[current_index].imageview;
	}
	vk::ImageView instance_depth_imageview ( u32 index ) {
		return per_image_data[index % per_image_data.size()].depth_imageview;
	}
	vk::ImageView instance_depth_imageview ( ) {
		return per_image_data[current_index].depth_imageview;
	}
	vk::ImageView instance_stencil_imageview ( u32 index ) {
		return per_image_data[index % per_image_data.size()].stencil_imageview;
	}
	vk::ImageView instance_stencil_imageview ( ) {
		return per_image_data[current_index].stencil_imageview;
	}
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
	void generate_mipmaps ( vk::ImageLayout oldLayout, vk::ImageLayout newLayout, u32 baseLevel, vk::CommandBuffer commandBuffer ) {
		generate_mipmaps ( oldLayout, newLayout, {baseLevel, mipmap_layers}, {0, layers}, commandBuffer );
	}
	void generate_mipmaps ( vk::ImageLayout oldLayout, vk::ImageLayout newLayout, vk::CommandBuffer commandBuffer ) {
		generate_mipmaps ( oldLayout, newLayout, {0, mipmap_layers}, {0, layers}, commandBuffer );
	}
};
