
#include "VImage.h"
#include "VResourceManager.h"
#include "VWindow.h"
#include "VTransformEnums.h"


VBaseImage::VBaseImage ( VInstance* instance, u32 width, u32 height, u32 depth, u32 layers, u32 mipmap_layers, 
		vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags usage, 
		vk::ImageAspectFlags aspect, vk::MemoryPropertyFlags needed, vk::MemoryPropertyFlags recommended ) :
	Image ( transform_image_format ( format ), width, height, depth, layers, mipmap_layers, false ),
	v_instance ( instance ), v_format ( format ), tiling ( tiling ), usage ( usage ), aspect ( aspect ),
	memory ( needed, recommended ), image (), dependent ( false ), fraction ( 1.0f ) {
	v_set_extent ( width, height, depth );
	init();
}
VBaseImage::VBaseImage ( VInstance* instance, u32 width, u32 height, u32 depth, u32 layers, u32 mipmap_layers, 
		vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags usage, vk::ImageAspectFlags aspect, 
		float fraction, vk::MemoryPropertyFlags needed, vk::MemoryPropertyFlags recommended ) :
	Image ( transform_image_format ( format ), width, height, depth, layers, mipmap_layers, false ),
	v_instance ( instance ), v_format ( format ), tiling ( tiling ), usage ( usage ), aspect ( aspect ),
	memory ( needed, recommended ), image (), dependent ( true ), fraction ( fraction ) {
	v_set_extent ( width * fraction, height * fraction, depth * fraction );
	init();
}
VBaseImage::VBaseImage ( VInstance* instance, VWindow* window ) :
	Image ( transform_image_format ( window->present_swap_format.format ), window->swap_chain_extend.x, window->swap_chain_extend.y, 0, 1, 1, true ),
	v_instance ( instance ), v_format ( window->present_swap_format.format ), tiling ( vk::ImageTiling::eOptimal ), usage ( vk::ImageUsageFlags ( vk::ImageUsageFlagBits::eColorAttachment ) ), aspect ( vk::ImageAspectFlags ( vk::ImageAspectFlagBits::eColor ) ),
	window ( window ),
	memory(), image (), dependent ( false ), fraction ( 0.0f ) {

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

	for ( VImageUse& use : usages ) {
		if ( use.id ) v_instance->destroyImageView ( use.imageview );
		use.imageview = vk::ImageView();
	}
	u32 queueindices[2] = {v_instance->queue_wrapper()->graphics_queue_id, v_instance->queue_wrapper()->transfer_queue_id};
	vk::ImageCreateInfo imageInfo ( vk::ImageCreateFlags(), type, v_format, extent, mipmap_layers, layers, vk::SampleCountFlagBits::e1, tiling, usage, vk::SharingMode::eConcurrent, 2, queueindices, vk::ImageLayout::eUndefined );

	V_CHECKCALL ( v_instance->m_device.createImage ( &imageInfo, nullptr, &image ), v_logger.log<LogLevel::eError> ( "Failed To Create Image" ) );

	v_logger.log<LogLevel::eDebug> ( "Create Image 0x%x of dimensions %dx%dx%d with usage %s", image, extent.width, extent.height, extent.depth, to_string ( usage ).c_str() );

	vk::MemoryRequirements mem_req;
	v_instance->m_device.getImageMemoryRequirements ( image, &mem_req );
	v_instance->allocate_gpu_memory ( mem_req, &memory );
	vkBindImageMemory ( v_instance->m_device, image, memory.memory, 0 );

	created_frame_index = v_instance->frame_index;
	for ( VImageUse& use : usages ) {
		if ( use.id ) {
			use.image->v_create_imageview ( &use );
		}
	}
}
void VBaseImage::init ( vk::Image image_ ) {
	for ( VImageUse& use : usages ) {
		if ( use.id ) v_instance->destroyImageView ( use.imageview );
		use.imageview = vk::ImageView();
	}
	image = image_;
	created_frame_index = v_instance->frame_index;
	for ( VImageUse& use : usages ) {
		if ( use.id ) {
			use.image->v_create_imageview ( &use );
		}
	}
}
void VBaseImage::destroy() {
	if ( memory.memory ) { //if the image is not managed externally
		v_instance->m_device.destroyImage ( image );
		image = vk::Image();
		v_instance->free_gpu_memory ( memory );
		memory.memory = vk::DeviceMemory();
	}
	for ( VImageUse& use : usages ) {
		if ( use.id ) v_instance->destroyImageView ( use.imageview );
		use.imageview = vk::ImageView();
	}
	usages.clear();
}

void VBaseImage::rebuild_image ( u32 width, u32 height, u32 depth ) {
	if ( memory.memory ) { //if the image is not managed externally
		v_instance->m_device.destroyImage ( image );
		image = vk::Image();
		v_instance->free_gpu_memory ( memory );
		memory.memory = vk::DeviceMemory();
	}
	v_set_extent ( width * fraction, height * fraction, depth * fraction );
	init();
}

ImageUseRef VBaseImage::create_use ( ImagePart part, Range<u32> mipmaps, Range<u32> layers ) {
	vk::ImageAspectFlags aspects;
	switch ( part ) {
	case ImagePart::eColor:
		aspects |= vk::ImageAspectFlagBits::eColor;
		break;
	case ImagePart::eDepth:
		aspects |= vk::ImageAspectFlagBits::eDepth;
		break;
	case ImagePart::eDepthStencil:
		aspects |= vk::ImageAspectFlagBits::eDepth;
		aspects |= vk::ImageAspectFlagBits::eStencil;
		break;
	case ImagePart::eStencil:
		aspects |= vk::ImageAspectFlagBits::eStencil;
		break;
	};
	VImageUseRef vimageuse = v_create_use ( aspects, mipmaps, layers );
	return { vimageuse.id, this };
}
VImageUseRef VBaseImage::v_create_use ( vk::ImageAspectFlags aspects, Range<u32> mipmaps, Range<u32> layers ) {
	for ( VImageUse& use : usages ) {
		if ( use.aspects == aspects && use.mipmaps == mipmaps && use.layers == layers ) return VImageUseRef ( use.id, this, created_frame_index );
	}
	VImageUse imageuse;
	imageuse.mipmaps = mipmaps;
	imageuse.layers = layers;
	imageuse.aspects = aspects;
	imageuse.image = this;
	v_create_imageview ( &imageuse );
	return VImageUseRef ( usages.insert ( imageuse ), this, created_frame_index );
}
void VBaseImage::v_create_imageview ( VImageUse* imageuse ) {
	if ( type == vk::ImageType::e2D && layers == 1 ) {
		imageuse->imageview = v_instance->createImageView2D (
		                          image,
		                          imageuse->mipmaps.min, imageuse->mipmaps.max - imageuse->mipmaps.min,
		                          v_format, imageuse->aspects
		                      );
	} else if ( type == vk::ImageType::e2D ) {
		imageuse->imageview = v_instance->createImageView2DArray (
		                          image,
		                          imageuse->mipmaps.min, imageuse->mipmaps.max - imageuse->mipmaps.min,
		                          imageuse->layers.min, imageuse->layers.max - imageuse->layers.min,
		                          v_format, imageuse->aspects
		                      );
	} else {
		v_logger.log<LogLevel::eDebug> ( "Not implemented %s", to_string ( type ).c_str() );
		assert ( false );
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
	if ( v_logger.is_enabled<LogLevel::eTrace>() ) {
		v_logger.log<LogLevel::eTrace> (
		    "Transition Image 0x%x from %s to %s mip-min %d mip-max %d array-min %d array-max %d",
		    image, to_string ( oldLayout ).c_str(), to_string ( newLayout ).c_str(), miprange.min, miprange.max, arrayrange.min, arrayrange.max );
	}
	return vk::ImageMemoryBarrier (
	           srcAccessMask, dstAccessMask,
	           oldLayout, newLayout,
	           VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED,
	           image,
	           vk::ImageSubresourceRange ( aspect, miprange.min, miprange.max - miprange.min, arrayrange.min, arrayrange.max - arrayrange.min )
	       );
}

void VBaseImage::transition_layout ( vk::ImageLayout oldLayout, vk::ImageLayout newLayout, Range<u32> mip_range, Range<u32> array_range, vk::CommandBuffer commandBuffer ) {

	if ( oldLayout == newLayout )
		return;
	if ( mip_range.max == 0 )
		mip_range.max = mipmap_layers;
	if ( array_range.max == 0 )
		array_range.max = layers;

	vk::PipelineStageFlags sourceStage, destinationStage;
	vk::ImageMemoryBarrier barrier = transition_layout_impl ( oldLayout, newLayout, mip_range, array_range, &sourceStage, &destinationStage );

	commandBuffer.pipelineBarrier (
	    sourceStage, destinationStage,
	    vk::DependencyFlags(),
	    {},//memoryBarriers
	    {},//bufferBarriers
	    vk::ArrayProxy<const vk::ImageMemoryBarrier> ( 1, &barrier ) //imageBarriers
	);
}
void VBaseImage::transition_layout ( vk::ImageLayout* oldLayout, vk::ImageLayout* newLayout, Range<u32> mip_range, Range<u32> array_range, vk::CommandBuffer commandBuffer ) {
	u32 miplayers = mip_range.max - mip_range.min;

	u32 needed_barrier_count = 0;
	bool skipped_last = true;
	vk::ImageLayout lastOldLayout, lastNewLayout;
	for ( u32 i = 0; i < miplayers; i++ ) {
		if ( oldLayout[i] != newLayout[i] && ( skipped_last || oldLayout[i] != lastOldLayout || newLayout[i] != lastNewLayout ) ) {
			needed_barrier_count++;
			skipped_last = false;
			lastOldLayout = oldLayout[i];
			lastNewLayout = newLayout[i];
		}
		if ( oldLayout[i] == newLayout[i] ) skipped_last = true;
	}

	if ( !needed_barrier_count ) return;

	Array<vk::ImageMemoryBarrier> barriers ( needed_barrier_count );
	needed_barrier_count = 0;
	skipped_last = true;
	vk::PipelineStageFlags sourceStage, destinationStage;
	for ( u32 i = 0; i < miplayers; i++ ) {
		if ( oldLayout[i] != newLayout[i] ) {
			if ( skipped_last || oldLayout[i] != lastOldLayout || newLayout[i] != lastNewLayout ) {
				skipped_last = false;
				lastOldLayout = oldLayout[i];
				lastNewLayout = newLayout[i];
				barriers[needed_barrier_count] = transition_layout_impl ( oldLayout[i], newLayout[i], {i, i + 1}, array_range, &sourceStage, &destinationStage );
				needed_barrier_count++;
			} else {
				barriers[needed_barrier_count - 1].subresourceRange.levelCount++;
			}
		} else {
			if ( !skipped_last )
				barriers[needed_barrier_count] = transition_layout_impl ( oldLayout[i], newLayout[i], {i, i + 1}, array_range, &sourceStage, &destinationStage );
			skipped_last = true;
		}
	}
	//barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	//barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	commandBuffer.pipelineBarrier (
	    sourceStage, destinationStage,
	    vk::DependencyFlags(),
	    {},//memoryBarriers
	    {},//bufferBarriers
	    vk::ArrayProxy<const vk::ImageMemoryBarrier> ( barriers.size, barriers.data ) //imageBarriers
	);
}

void VBaseImage::generate_mipmaps ( vk::ImageLayout oldLayout, vk::ImageLayout newLayout, Range<u32> mip_range, Range<u32> array_range, vk::CommandBuffer commandBuffer ) {

	if ( mip_range.max == 0 )
		mip_range.max = mipmap_layers;
	if ( array_range.max == 0 )
		array_range.max = layers;

	Array<vk::ImageLayout> start_layouts ( mip_range.max - mip_range.min, oldLayout );
	Array<vk::ImageLayout> end_layouts ( mip_range.max - mip_range.min, newLayout );
	generate_mipmaps ( start_layouts.data, end_layouts.data, mip_range, array_range, commandBuffer );
}

void VBaseImage::generate_mipmaps ( vk::ImageLayout* oldLayout, vk::ImageLayout* newLayout, Range<u32> mip_range, Range<u32> array_range, vk::CommandBuffer commandBuffer ) {

	if ( mip_range.max == 0 )
		mip_range.max = mipmap_layers;
	if ( array_range.max == 0 )
		array_range.max = layers;

	if ( mip_range.max - mip_range.min < 2 ) return;

	Array<vk::ImageLayout> transfer_layouts ( mip_range.max - mip_range.min, vk::ImageLayout::eTransferDstOptimal );

	u32 count = mip_range.max - mip_range.min;
	for ( u32 i = 1; i < count; i++ ) {
		u32 index = mip_range.min + i;
		transfer_layouts.data[i - 1] = vk::ImageLayout::eTransferSrcOptimal;
		vk::PipelineStageFlags sourceStage, destinationStage;
		vk::ImageMemoryBarrier barriers[2] = {
			transition_layout_impl ( oldLayout[i - 1], transfer_layouts.data[i - 1], {index - 1, index}, array_range, &sourceStage, &destinationStage ),
			transition_layout_impl ( oldLayout[i], transfer_layouts.data[i], {index, index + 1}, array_range, &sourceStage, &destinationStage ),
		};
		oldLayout[i - 1] = transfer_layouts.data[i - 1];
		oldLayout[i] = transfer_layouts.data[i];

		commandBuffer.pipelineBarrier (
		    sourceStage, destinationStage,
		    vk::DependencyFlags(),
		    {},//memoryBarriers
		    {},//bufferBarriers
		    vk::ArrayProxy<const vk::ImageMemoryBarrier> ( 2, barriers ) //imageBarriers
		);

		// Set up a blit from previous mip-level to the next.
		vk::ImageBlit imageBlit ( vk::ImageSubresourceLayers ( aspect, index - 1, array_range.min, array_range.max - array_range.min ), {
			vk::Offset3D ( 0, 0, 0 ), vk::Offset3D ( std::max ( int ( extent.width >> ( index - 1 ) ), 1 ), std::max ( int ( extent.height >> ( index - 1 ) ), 1 ), std::max ( int ( extent.depth >> ( index - 1 ) ), 1 ) )
		}, vk::ImageSubresourceLayers ( aspect, index, array_range.min, array_range.max - array_range.min ), {
			vk::Offset3D ( 0, 0, 0 ), vk::Offset3D ( std::max ( int ( extent.width >> index ), 1 ), std::max ( int ( extent.height >> index ), 1 ), std::max ( int ( extent.depth >> index ), 1 ) )
		} );
		commandBuffer.blitImage ( image, vk::ImageLayout::eTransferSrcOptimal, image, vk::ImageLayout::eTransferDstOptimal, {imageBlit}, vk::Filter::eLinear );
		//commandBuffer.blitImage ( instance_image(), vk::ImageLayout::eGeneral, instance_image(), vk::ImageLayout::eGeneral, {imageBlit}, vk::Filter::eLinear );
	}
	transition_layout ( transfer_layouts.data, newLayout, mip_range, array_range, commandBuffer );
}
