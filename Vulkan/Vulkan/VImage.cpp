
#include "VImage.h"
#include "VResourceManager.h"
#include "VWindow.h"


vk::Format transform_image_format ( ImageFormat format ) {
	switch ( format ) {
	case ImageFormat::eU8:
		return vk::Format::eR8Uint;
	case ImageFormat::eU16:
		return vk::Format::eR16Uint;
	case ImageFormat::eU32:
		return vk::Format::eR32Uint;
	case ImageFormat::eU64:
		return vk::Format::eR64Uint;

	case ImageFormat::eUnorm8:
		return vk::Format::eR8Unorm;
	case ImageFormat::eUnorm16:
		return vk::Format::eR16Unorm;

	case ImageFormat::eS8:
		return vk::Format::eR8Sint;
	case ImageFormat::eS16:
		return vk::Format::eR16Sint;
	case ImageFormat::eS32:
		return vk::Format::eR32Sint;
	case ImageFormat::eS64:
		return vk::Format::eR64Sint;

	case ImageFormat::eSnorm8:
		return vk::Format::eR8Snorm;
	case ImageFormat::eSnorm16:
		return vk::Format::eR16Snorm;

	case ImageFormat::eF16:
		return vk::Format::eR16Sfloat;
	case ImageFormat::eF32:
		return vk::Format::eR32Sfloat;
	case ImageFormat::eF64:
		return vk::Format::eR64Sfloat;


	case ImageFormat::e2U8:
		return vk::Format::eR8G8Uint;
	case ImageFormat::e2U16:
		return vk::Format::eR16G16Uint;
	case ImageFormat::e2U32:
		return vk::Format::eR32G32Uint;
	case ImageFormat::e2U64:
		return vk::Format::eR64G64Uint;

	case ImageFormat::e2Unorm8:
		return vk::Format::eR8G8Unorm;
	case ImageFormat::e2Unorm16:
		return vk::Format::eR16G16Unorm;

	case ImageFormat::e2S8:
		return vk::Format::eR8G8Sint;
	case ImageFormat::e2S16:
		return vk::Format::eR16G16Sint;
	case ImageFormat::e2S32:
		return vk::Format::eR32G32Sint;
	case ImageFormat::e2S64:
		return vk::Format::eR64G64Sint;

	case ImageFormat::e2Snorm8:
		return vk::Format::eR8G8Snorm;
	case ImageFormat::e2Snorm16:
		return vk::Format::eR16G16Snorm;

	case ImageFormat::e2F16:
		return vk::Format::eR16G16Sfloat;
	case ImageFormat::e2F32:
		return vk::Format::eR32G32Sfloat;
	case ImageFormat::e2F64:
		return vk::Format::eR64G64Sfloat;


	case ImageFormat::e4U8:
		return vk::Format::eR8G8B8A8Uint;
	case ImageFormat::e4U16:
		return vk::Format::eR16G16B16A16Uint;
	case ImageFormat::e4U32:
		return vk::Format::eR32G32B32A32Uint;
	case ImageFormat::e4U64:
		return vk::Format::eR64G64B64A64Uint;

	case ImageFormat::e4Unorm8:
		return vk::Format::eR8G8B8A8Unorm;
	case ImageFormat::e4Unorm16:
		return vk::Format::eR16G16B16A16Unorm;

	case ImageFormat::e4S8:
		return vk::Format::eR8G8B8A8Sint;
	case ImageFormat::e4S16:
		return vk::Format::eR16G16B16A16Sint;
	case ImageFormat::e4S32:
		return vk::Format::eR32G32B32A32Sint;
	case ImageFormat::e4S64:
		return vk::Format::eR64G64B64A64Sint;

	case ImageFormat::e4Snorm8:
		return vk::Format::eR8G8B8A8Snorm;
	case ImageFormat::e4Snorm16:
		return vk::Format::eR16G16B16A16Snorm;

	case ImageFormat::e4F16:
		return vk::Format::eR16G16B16A16Sfloat;
	case ImageFormat::e4F32:
		return vk::Format::eR32G32B32A32Sfloat;
	case ImageFormat::e4F64:
		return vk::Format::eR64G64B64A64Sfloat;


	case ImageFormat::e3U10_U2:
		return vk::Format::eA2B10G10R10UintPack32;
	case ImageFormat::e3S10_S2:
		return vk::Format::eA2B10G10R10SintPack32;
	case ImageFormat::e3Unorm10_Unorm2:
		return vk::Format::eA2B10G10R10UnormPack32;
	case ImageFormat::e3Snorm10_Snorm2:
		return vk::Format::eA2B10G10R10SnormPack32;

	case ImageFormat::eD16Unorm:
		return vk::Format::eD16Unorm;
	case ImageFormat::eD32F:
		return vk::Format::eD32Sfloat;
	case ImageFormat::eD24Unorm_St8U:
		return vk::Format::eD24UnormS8Uint;
	case ImageFormat::eD32F_St8Uint:
		return vk::Format::eD32SfloatS8Uint;
	case ImageFormat::eSt8Uint:
		return vk::Format::eS8Uint;

	default:
		return vk::Format::eUndefined;
	}
}
ImageFormat transform_image_format ( vk::Format format ) {
	switch ( format ) {
	case vk::Format::eR8Uint:
		return ImageFormat::eU8;
	case vk::Format::eR16Uint:
		return ImageFormat::eU16;
	case vk::Format::eR32Uint:
		return ImageFormat::eU32;
	case vk::Format::eR64Uint:
		return ImageFormat::eU64;

	case vk::Format::eR8Unorm:
		return ImageFormat::eUnorm8;
	case vk::Format::eR16Unorm:
		return ImageFormat::eUnorm16;

	case vk::Format::eR8Sint:
		return ImageFormat::eS8;
	case vk::Format::eR16Sint:
		return ImageFormat::eS16;
	case vk::Format::eR32Sint:
		return ImageFormat::eS32;
	case vk::Format::eR64Sint:
		return ImageFormat::eS64;

	case vk::Format::eR8Snorm:
		return ImageFormat::eSnorm8;
	case vk::Format::eR16Snorm:
		return ImageFormat::eSnorm16;

	case vk::Format::eR16Sfloat:
		return ImageFormat::eF16;
	case vk::Format::eR32Sfloat:
		return ImageFormat::eF32;
	case vk::Format::eR64Sfloat:
		return ImageFormat::eF64;


	case vk::Format::eR8G8Uint:
		return ImageFormat::e2U8;
	case vk::Format::eR16G16Uint:
		return ImageFormat::e2U16;
	case vk::Format::eR32G32Uint:
		return ImageFormat::e2U32;
	case vk::Format::eR64G64Uint:
		return ImageFormat::e2U64;

	case vk::Format::eR8G8Unorm:
		return ImageFormat::e2Unorm8;
	case vk::Format::eR16G16Unorm:
		return ImageFormat::e2Unorm16;

	case vk::Format::eR8G8Sint:
		return ImageFormat::e2S8;
	case vk::Format::eR16G16Sint:
		return ImageFormat::e2S16;
	case vk::Format::eR32G32Sint:
		return ImageFormat::e2S32;
	case vk::Format::eR64G64Sint:
		return ImageFormat::e2S64;

	case vk::Format::eR8G8Snorm:
		return ImageFormat::e2Snorm8;
	case vk::Format::eR16G16Snorm:
		return ImageFormat::e2Snorm16;

	case vk::Format::eR16G16Sfloat:
		return ImageFormat::e2F16;
	case vk::Format::eR32G32Sfloat:
		return ImageFormat::e2F32;
	case vk::Format::eR64G64Sfloat:
		return ImageFormat::e2F64;


	case vk::Format::eR8G8B8A8Uint:
		return ImageFormat::e4U8;
	case vk::Format::eR16G16B16A16Uint:
		return ImageFormat::e4U16;
	case vk::Format::eR32G32B32A32Uint:
		return ImageFormat::e4U32;
	case vk::Format::eR64G64B64A64Uint:
		return ImageFormat::e4U64;

	case vk::Format::eR8G8B8A8Unorm:
		return ImageFormat::e4Unorm8;
	case vk::Format::eR16G16B16A16Unorm:
		return ImageFormat::e4Unorm16;

	case vk::Format::eR8G8B8A8Sint:
		return ImageFormat::e4S8;
	case vk::Format::eR16G16B16A16Sint:
		return ImageFormat::e4S16;
	case vk::Format::eR32G32B32A32Sint:
		return ImageFormat::e4S32;
	case vk::Format::eR64G64B64A64Sint:
		return ImageFormat::e4S64;

	case vk::Format::eR8G8B8A8Snorm:
		return ImageFormat::e4Snorm8;
	case vk::Format::eR16G16B16A16Snorm:
		return ImageFormat::e4Snorm16;

	case vk::Format::eR16G16B16A16Sfloat:
		return ImageFormat::e4F16;
	case vk::Format::eR32G32B32A32Sfloat:
		return ImageFormat::e4F32;
	case vk::Format::eR64G64B64A64Sfloat:
		return ImageFormat::e4F64;


	case vk::Format::eA2B10G10R10UintPack32:
		return ImageFormat::e3U10_U2;
	case vk::Format::eA2B10G10R10SintPack32:
		return ImageFormat::e3S10_S2;
	case vk::Format::eA2B10G10R10UnormPack32:
		return ImageFormat::e3Unorm10_Unorm2;
	case vk::Format::eA2B10G10R10SnormPack32:
		return ImageFormat::e3Snorm10_Snorm2;

	case vk::Format::eD16Unorm:
		return ImageFormat::eD16Unorm;
	case vk::Format::eD32Sfloat:
		return ImageFormat::eD32F;
	case vk::Format::eD24UnormS8Uint:
		return ImageFormat::eD24Unorm_St8U;
	case vk::Format::eD32SfloatS8Uint:
		return ImageFormat::eD32F_St8Uint;
	case vk::Format::eS8Uint:
		return ImageFormat::eSt8Uint;

	default:
		return ImageFormat::eUndefined;
	}
}
vk::Extent3D extent;
vk::ImageType type;
vk::ImageTiling tiling;
vk::ImageUsageFlags usage;
vk::ImageAspectFlags aspect;

vk::Semaphore image_available_sem;


VBaseImage::VBaseImage ( VInstance* instance, u32 width, u32 height, u32 depth, u32 layers, u32 mipmap_layers, vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags usage, vk::ImageAspectFlags aspect, vk::MemoryPropertyFlags needed, vk::MemoryPropertyFlags recommended ) :
	Image ( transform_image_format ( format ), width, height, depth, layers, mipmap_layers, false ),
	v_instance ( instance ), v_format ( format ), tiling ( tiling ), usage ( usage ), aspect ( aspect ),
	memory ( needed, recommended ), current_index ( 0 ), per_image_data ( 1 ), dependent ( false ), fraction ( 0.0f ) {
	v_set_extent ( width, height, depth );
	init();
}
VBaseImage::VBaseImage ( VInstance* instance, VWindow* window ) :
	Image ( transform_image_format ( window->present_swap_format.format ), window->swap_chain_extend.x, window->swap_chain_extend.y, 0, 1, 1, true ),
	v_instance ( instance ), v_format ( window->present_swap_format.format ), tiling ( vk::ImageTiling::eOptimal ), usage ( vk::ImageUsageFlags ( vk::ImageUsageFlagBits::eColorAttachment ) ), aspect ( vk::ImageAspectFlags ( vk::ImageAspectFlagBits::eColor ) ),
	window(window),
	memory(), current_index ( 0 ), per_image_data ( 0 ), dependent ( false ), fraction ( 0.0f ) {
	
	fetch_new_window_images();
}
VBaseImage::~VBaseImage() {
	v_instance->m_resource_manager->v_delete_dependant_images ( this );
	destroy();
}
void VBaseImage::v_set_format ( vk::Format format ) {
	v_format = format;
	this->format = transform_image_format ( format );
}
void VBaseImage::init() {
	per_image_data.resize ( 1 );

	for ( PerImageData& data : per_image_data ) {
		vk::ImageCreateInfo imageInfo ( vk::ImageCreateFlags(), type, v_format, extent, mipmap_layers, layers, vk::SampleCountFlagBits::e1, tiling, usage, vk::SharingMode::eExclusive, 0, nullptr, vk::ImageLayout::eUndefined );

		V_CHECKCALL ( v_instance->m_device.createImage ( &imageInfo, nullptr, &data.image ), printf ( "Failed To Create Image\n" ) );

		printf ( "Create Image 0x%x of dimensions %dx%dx%d with usage %s\n", data.image, extent.width, extent.height, extent.depth, to_string ( usage ).c_str() );

		vk::MemoryRequirements mem_req;
		v_instance->m_device.getImageMemoryRequirements ( data.image, &mem_req );
		v_instance->allocate_gpu_memory ( mem_req, &memory );
		vkBindImageMemory ( v_instance->m_device, data.image, memory.memory, 0 );

		if ( type == vk::ImageType::e2D && layers == 1 ) {
			data.imageview = v_instance->createImageView2D ( data.image, 0, mipmap_layers, v_format, aspect );
		} else if ( type == vk::ImageType::e2D ) {
			data.imageview = v_instance->createImageView2DArray ( data.image, 0, mipmap_layers, 0, layers, v_format, aspect );
		} else {
			printf ( "Not implemented %s\n", to_string ( type ).c_str() );
			assert ( false );
		}
	}
	created_frame_index = v_instance->frame_index;
}
void VBaseImage::init ( DynArray<vk::Image>& images ) {
	per_image_data.resize ( images.size() );
	for ( size_t i = 0; i < images.size(); i++ ) {
		per_image_data[i].image = images[i];
		per_image_data[i].imageview = v_instance->createImageView2D ( images[i], 0, 1, v_format, vk::ImageAspectFlags ( vk::ImageAspectFlagBits::eColor ) );
	}
	created_frame_index = v_instance->frame_index;
}
void VBaseImage::destroy() {
	for ( PerImageData& data : per_image_data ) {
		if ( data.imageview ) {
			v_instance->destroyImageView ( data.imageview );
		}
		if ( memory.memory ) { //if the image is not managed externally
			v_instance->m_device.destroyImage ( data.image );
			data.image = vk::Image();
			v_instance->free_gpu_memory ( memory );
			memory.memory = vk::DeviceMemory();
		}
	}
	per_image_data.clear();
}

void VBaseImage::rebuild_image ( u32 width, u32 height, u32 depth ) {
	destroy();
	v_set_extent ( width * fraction, height * fraction, depth * fraction );
	init();
}
void VBaseImage::set_current_image ( u32 index ) {
	current_index = index;
}

void VBaseImage::fetch_new_window_images ( ) {
	assert ( window_target );
	
	destroy();
	printf ( "Resizing Window Frame from size %" PRId32 "x%" PRId32 "x%" PRId32 " to %" PRId32 "x%" PRId32 "x%" PRId32 "\n",
	         this->width, this->height, this->depth, window->swap_chain_extend.width, window->swap_chain_extend.height, 0 );

	v_set_extent ( window->swap_chain_extend.width, window->swap_chain_extend.height, 0 );
	v_set_format ( window->present_swap_format.format );
	std::vector<vk::Image> images = v_instance->vk_device().getSwapchainImagesKHR ( window->swap_chain );
	init ( images );

	auto it = v_instance->m_resource_manager->dependency_map.find ( this );
	if ( it != v_instance->m_resource_manager->dependency_map.end() ) {
		printf ( "Found %d dependant Image(s)\n", it->second.size() );
		for ( VBaseImage* image : it->second ) {
			image->rebuild_image ( width, height, depth );
		}
	}
}

void VBaseImage::v_set_extent ( u32 width, u32 height, u32 depth ) {
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
	this->width = width;
	this->height = height;
	this->depth = depth;
}
vk::ImageMemoryBarrier VBaseImage::transition_layout_impl ( vk::ImageLayout oldLayout, vk::ImageLayout newLayout, Range<u32> miprange, Range<u32> arrayrange, vk::PipelineStageFlags* srcStageFlags, vk::PipelineStageFlags* dstStageFlags ) {

	vk::AccessFlags srcAccessMask, dstAccessMask;

	switch ( oldLayout ) {
	case vk::ImageLayout::eUndefined:
		srcAccessMask = vk::AccessFlags();
		*srcStageFlags |= vk::PipelineStageFlagBits::eTopOfPipe;
		break;
	case vk::ImageLayout::eGeneral:
		srcAccessMask = vk::AccessFlags();
		*srcStageFlags |= vk::PipelineStageFlagBits::eTopOfPipe;
		break;
	case vk::ImageLayout::eColorAttachmentOptimal:
		srcAccessMask = vk::AccessFlags ( vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite );
		*srcStageFlags |= vk::PipelineStageFlagBits::eColorAttachmentOutput;
		break;
	case vk::ImageLayout::eDepthStencilAttachmentOptimal:
		srcAccessMask = vk::AccessFlags ( vk::AccessFlagBits::eDepthStencilAttachmentRead | vk::AccessFlagBits::eDepthStencilAttachmentWrite );
		*srcStageFlags |= vk::PipelineStageFlagBits::eLateFragmentTests;
		break;
	case vk::ImageLayout::eDepthStencilReadOnlyOptimal:
		dstAccessMask = vk::AccessFlags ( vk::AccessFlagBits::eDepthStencilAttachmentRead );
		*dstStageFlags |= vk::PipelineStageFlagBits::eEarlyFragmentTests;
		break;
	case vk::ImageLayout::eShaderReadOnlyOptimal:
		srcAccessMask = vk::AccessFlagBits::eShaderRead;
		*srcStageFlags |= vk::PipelineStageFlagBits::eFragmentShader;
		break;
	case vk::ImageLayout::eTransferSrcOptimal:
		srcAccessMask = vk::AccessFlagBits::eTransferRead;
		*srcStageFlags |= vk::PipelineStageFlagBits::eTransfer;
		break;
	case vk::ImageLayout::eTransferDstOptimal:
		srcAccessMask = vk::AccessFlagBits::eTransferWrite;
		*srcStageFlags |= vk::PipelineStageFlagBits::eTransfer;
		break;
	case vk::ImageLayout::ePreinitialized:
		assert ( false );
		break;
	case vk::ImageLayout::ePresentSrcKHR:
		srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
		*srcStageFlags |= vk::PipelineStageFlagBits::eColorAttachmentOutput;
		break;
	case vk::ImageLayout::eSharedPresentKHR:
		srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
		*srcStageFlags |= vk::PipelineStageFlagBits::eColorAttachmentOutput;
		break;
	case vk::ImageLayout::eDepthReadOnlyStencilAttachmentOptimalKHR:
		srcAccessMask = vk::AccessFlags ( vk::AccessFlagBits::eDepthStencilAttachmentRead );
		*srcStageFlags |= vk::PipelineStageFlagBits::eLateFragmentTests;
		break;
	case vk::ImageLayout::eDepthAttachmentStencilReadOnlyOptimalKHR:
		srcAccessMask = vk::AccessFlags ( vk::AccessFlagBits::eDepthStencilAttachmentRead );
		*srcStageFlags |= vk::PipelineStageFlagBits::eLateFragmentTests;
		break;
	default:
		assert ( false );
	}

	switch ( newLayout ) {
	case vk::ImageLayout::eUndefined:
		dstAccessMask = vk::AccessFlags();
		*dstStageFlags |= vk::PipelineStageFlagBits::eBottomOfPipe;
		break;
	case vk::ImageLayout::eGeneral:
		dstAccessMask = vk::AccessFlags();
		*dstStageFlags |= vk::PipelineStageFlagBits::eBottomOfPipe;
		break;
	case vk::ImageLayout::eColorAttachmentOptimal:
		dstAccessMask = vk::AccessFlags ( vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite );
		*dstStageFlags |= vk::PipelineStageFlagBits::eColorAttachmentOutput;
		break;
	case vk::ImageLayout::eDepthStencilAttachmentOptimal:
		dstAccessMask = vk::AccessFlags ( vk::AccessFlagBits::eDepthStencilAttachmentRead | vk::AccessFlagBits::eDepthStencilAttachmentWrite );
		*dstStageFlags |= vk::PipelineStageFlagBits::eEarlyFragmentTests;
		break;
	case vk::ImageLayout::eDepthStencilReadOnlyOptimal:
		dstAccessMask = vk::AccessFlags ( vk::AccessFlagBits::eDepthStencilAttachmentRead );
		*dstStageFlags |= vk::PipelineStageFlagBits::eEarlyFragmentTests;
		break;
	case vk::ImageLayout::eShaderReadOnlyOptimal:
		dstAccessMask = vk::AccessFlagBits::eShaderRead;
		*dstStageFlags |= vk::PipelineStageFlagBits::eFragmentShader;
		break;
	case vk::ImageLayout::eTransferSrcOptimal:
		dstAccessMask = vk::AccessFlagBits::eTransferRead;
		*dstStageFlags |= vk::PipelineStageFlagBits::eTransfer;
		break;
	case vk::ImageLayout::eTransferDstOptimal:
		dstAccessMask = vk::AccessFlagBits::eTransferWrite;
		*dstStageFlags |= vk::PipelineStageFlagBits::eTransfer;
		break;
	case vk::ImageLayout::ePreinitialized:
		assert ( false );
		break;
	case vk::ImageLayout::ePresentSrcKHR:
		dstAccessMask = vk::AccessFlagBits::eMemoryRead;
		*dstStageFlags |= vk::PipelineStageFlagBits::eTopOfPipe;
		break;
	case vk::ImageLayout::eSharedPresentKHR:
		dstAccessMask = vk::AccessFlagBits::eMemoryRead;
		*dstStageFlags |= vk::PipelineStageFlagBits::eTopOfPipe;
		break;
	case vk::ImageLayout::eDepthReadOnlyStencilAttachmentOptimalKHR:
		dstAccessMask = vk::AccessFlags ( vk::AccessFlagBits::eDepthStencilAttachmentRead );
		*dstStageFlags |= vk::PipelineStageFlagBits::eEarlyFragmentTests;
		break;
	case vk::ImageLayout::eDepthAttachmentStencilReadOnlyOptimalKHR:
		dstAccessMask = vk::AccessFlags ( vk::AccessFlagBits::eDepthStencilAttachmentRead );
		*dstStageFlags |= vk::PipelineStageFlagBits::eEarlyFragmentTests;
		break;
	default:
		assert ( false );
	}
	return vk::ImageMemoryBarrier (
	           srcAccessMask, dstAccessMask,
	           oldLayout, newLayout,
	           0, 0,
	           instance_image(),
	           vk::ImageSubresourceRange ( aspect, miprange.min, miprange.max - miprange.min, arrayrange.min, arrayrange.max - arrayrange.min )
	       );
}

void VBaseImage::transition_layout ( vk::ImageLayout oldLayout, vk::ImageLayout newLayout, Range<u32> mip_range, Range<u32> array_range, vk::CommandBuffer commandBuffer ) {

	if ( oldLayout == newLayout )
		return;
	PerImageData& data = per_image_data[current_index];
	if ( mip_range.max == 0 )
		mip_range.max = mipmap_layers;
	if ( array_range.max == 0 )
		array_range.max = layers;

	vk::PipelineStageFlags sourceStage, destinationStage;
	vk::ImageMemoryBarrier barrier = transition_layout_impl ( oldLayout, newLayout, mip_range, array_range, &sourceStage, &destinationStage );

	barrier.srcQueueFamilyIndex = 0;
	barrier.dstQueueFamilyIndex = 0;
	commandBuffer.pipelineBarrier (
	    sourceStage, destinationStage,
	    vk::DependencyFlags(),
	    {},//memoryBarriers
	    {},//bufferBarriers
	    vk::ArrayProxy<const vk::ImageMemoryBarrier> ( 1, &barrier ) //imageBarriers
	);
}
void VBaseImage::transition_layout ( vk::ImageLayout* oldLayout, vk::ImageLayout* newLayout, Range<u32> mip_range, Range<u32> array_range, vk::CommandBuffer commandBuffer ) {
	assert ( false );
	/*
	PerImageData& data = per_image_data[current_index];
	if ( mip_range.max == 0 )
		mip_range.max = mipmap_layers;
	if ( array_range.max == 0 )
		array_range.max = layers;

	vk::PipelineStageFlags sourceStage, destinationStage;
	u32 different_layout_count = data.layouts[array_range.min] == newLayout ? 0 : 1;
	for ( u32 i = array_range.min + 1; i < array_range.max; ++i ) {
		if ( data.layouts[array_range.min + i] != data.layouts[array_range.min + i - 1] && data.layouts[array_range.min + i] != newLayout ) {
			++different_layout_count;
		}
	}
	vk::ImageMemoryBarrier barriers[different_layout_count];
	{
		u32 different_layout_index = 0;
		u32 startid = array_range.min;
		vk::ImageLayout last_layout = data.layouts[0];
		for ( u32 i = array_range.min + 1; i < array_range.max; ++i ) {
			if ( data.layouts[array_range.min + i] != last_layout ) {
				if ( last_layout != newLayout )
					barriers[different_layout_index++] = transition_layout_impl ( last_layout, newLayout, mip_range, {startid, i - startid}, &sourceStage, &destinationStage );
				startid = i;
				last_layout = data.layouts[array_range.min + i];
			}
		}
		if ( last_layout != newLayout )
			barriers[different_layout_index] = transition_layout_impl ( last_layout, newLayout, mip_range, {startid, array_range.max - startid}, &sourceStage, &destinationStage );
	}

	for ( u32 i = 0; i < different_layout_count; ++i ) {
		barriers[i].srcQueueFamilyIndex = 0;
		barriers[i].dstQueueFamilyIndex = 0;
	}
	if(different_layout_count > 0) {
		commandBuffer.pipelineBarrier (
			sourceStage, destinationStage,
			vk::DependencyFlags(),
			{},//memoryBarriers
			{},//bufferBarriers
			vk::ArrayProxy<const vk::ImageMemoryBarrier> ( different_layout_count, barriers ) //imageBarriers
		);
		for ( size_t i = array_range.min; i < array_range.max; ++i ) {
			data.layouts[i] = newLayout;
		}
	}*/
}

void VBaseImage::generate_mipmaps ( vk::ImageLayout oldLayout, vk::ImageLayout newLayout, Range<u32> mip_range, Range<u32> array_range, vk::ImageLayout targetLayout, vk::CommandBuffer commandBuffer ) {

	if ( mip_range.max == 0 )
		mip_range.max = mipmap_layers;
	if ( array_range.max == 0 )
		array_range.max = layers;

	Array<vk::ImageLayout> start_layouts ( mip_range.max - mip_range.min, oldLayout );
	Array<vk::ImageLayout> transfer_layouts ( mip_range.max - mip_range.min, vk::ImageLayout::eTransferDstOptimal );
	transfer_layouts[0] = vk::ImageLayout::eTransferSrcOptimal;
	Array<vk::ImageLayout> end_layouts ( mip_range.max - mip_range.min, newLayout );

	transition_layout ( start_layouts.data, transfer_layouts.data, {mip_range.min, mip_range.min + 1}, array_range, commandBuffer );

	for ( u32 index = mip_range.min + 1; index < mip_range.max; index++ ) {
		// Set up a blit from previous mip-level to the next.
		vk::ImageBlit imageBlit ( vk::ImageSubresourceLayers ( aspect, index - 1, array_range.min, array_range.max - array_range.min ), {
			vk::Offset3D ( 0, 0, 0 ), vk::Offset3D ( std::max ( int ( extent.width >> ( index - 1 ) ), 1 ), std::max ( int ( extent.height >> ( index - 1 ) ), 1 ), std::max ( int ( extent.depth >> ( index - 1 ) ), 1 ) )
		}, vk::ImageSubresourceLayers ( aspect, index, array_range.min, array_range.max - array_range.min ), {
			vk::Offset3D ( 0, 0, 0 ), vk::Offset3D ( std::max ( int ( extent.width >> index ), 1 ), std::max ( int ( extent.height >> index ), 1 ), std::max ( int ( extent.depth >> index ), 1 ) )
		} );
		commandBuffer.blitImage ( instance_image(), vk::ImageLayout::eTransferSrcOptimal, instance_image(), vk::ImageLayout::eTransferDstOptimal, {imageBlit}, vk::Filter::eLinear );
		//commandBuffer.blitImage ( instance_image(), vk::ImageLayout::eGeneral, instance_image(), vk::ImageLayout::eGeneral, {imageBlit}, vk::Filter::eLinear );
	}
	transition_layout ( transfer_layouts.data, end_layouts.data, {mip_range.min, mip_range.min + 1}, array_range, commandBuffer );
}
