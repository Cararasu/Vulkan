
#include "VulkanImage.h"


VulkanBaseImage::VulkanBaseImage ( vk::Format format, u32 width, u32 height, u32 depth, u32 layers, u32 mipmap_layers, bool window_target, VulkanInstance* instance,
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
VulkanBaseImage::~VulkanBaseImage() { 
	
}

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

VulkanImageWrapper::VulkanImageWrapper ( VulkanInstance* instance, vk::Extent3D extent, u32 layers, u32 mipmap_layers, vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags usage, vk::ImageAspectFlags aspect, vk::MemoryPropertyFlags needed, vk::MemoryPropertyFlags recommended ) :
	VulkanBaseImage ( format, extent.width, extent.height, extent.depth, layers, mipmap_layers, false, instance, tiling, usage, aspect ), 
		memory(), layouts ( layers, vk::ImageLayout::eUndefined ) {

	vk::ImageCreateInfo imageInfo ( vk::ImageCreateFlags(), this->type, format, this->extent, mipmap_layers, layers, vk::SampleCountFlagBits::e1, tiling, usage, vk::SharingMode::eExclusive, 0, nullptr, vk::ImageLayout::eUndefined );

	V_CHECKCALL ( instance->m_device.createImage ( &imageInfo, nullptr, &image ), printf ( "Failed To Create Image\n" ) );

	printf ( "Create Image of dimensions %dx%dx%d\n", extent.width, extent.height, extent.depth );

	vk::MemoryRequirements mem_req;
	instance->m_device.getImageMemoryRequirements ( image, &mem_req );
	memory = instance->allocate_gpu_memory ( mem_req, needed, recommended );
	vkBindImageMemory ( instance->m_device, image, memory.memory, 0 );
	
	if(type == vk::ImageType::e2D && layers == 1){
		imageview = instance->createImageView2D(image, 0, mipmap_layers, format, aspect);
	}
	else if(type == vk::ImageType::e2D){
		imageview = instance->createImageView2DArray(image, 0, mipmap_layers, 0, layers, format, aspect);
	}
}
VulkanImageWrapper::~VulkanImageWrapper() {
	destroy();
}

void VulkanImageWrapper::destroy() {
	if ( memory.memory ) { //if the image is managed externally
		v_instance->destroyImageView(imageview);
		v_instance->m_device.destroyImage ( image );
		image = vk::Image();
		v_instance->free_gpu_memory ( memory );
		memory.memory = vk::DeviceMemory();
	}
}
vk::ImageMemoryBarrier VulkanBaseImage::transition_image_layout_impl ( vk::ImageLayout oldLayout, vk::ImageLayout newLayout, Range<u32> miprange, Range<u32> arrayrange, vk::PipelineStageFlags* srcStageFlags, vk::PipelineStageFlags* dstStageFlags ) {

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
	           image,
	           vk::ImageSubresourceRange ( aspect, miprange.min, miprange.max - miprange.min, arrayrange.min, arrayrange.max - arrayrange.min )
	       );
}
void VulkanWindowImage::transition_image_layout ( vk::ImageLayout newLayout, Range<u32> mip_range, Range<u32> array_range, vk::CommandBuffer commandBuffer ) {
	
	vk::PipelineStageFlags sourceStage, destinationStage;
	vk::ImageMemoryBarrier barrier = transition_image_layout_impl ( per_image_data[current_index].layout, newLayout, mip_range, {0, 1}, &sourceStage, &destinationStage );

	commandBuffer.pipelineBarrier (
	    sourceStage, destinationStage,
	    vk::DependencyFlags(),
	    {},//memoryBarriers
		{},//bufferBarriers
		{barrier} //imageBarriers
	);
}
void VulkanImageWrapper::transition_image_layout ( vk::ImageLayout newLayout, Range<u32> mip_range, Range<u32> array_range, vk::CommandBuffer commandBuffer ) {

	if ( mip_range.max == 0 )
		mip_range.max = mipmap_layers;
	if ( array_range.max == 0 )
		array_range.max = layers;

	vk::PipelineStageFlags sourceStage, destinationStage;
	u32 different_layout_count = layouts[array_range.min] == newLayout ? 0 : 1;
	for ( u32 i = array_range.min + 1; i < array_range.max; ++i ) {
		if ( layouts[array_range.min + i] != layouts[array_range.min + i - 1] && layouts[array_range.min + i] != newLayout ) {
			++different_layout_count;
		}
	}
	vk::ImageMemoryBarrier barriers[different_layout_count];

	{
		u32 different_layout_index = 0;
		u32 startid = array_range.min;
		vk::ImageLayout last_layout = layouts[0];
		for ( u32 i = array_range.min + 1; i < array_range.max; ++i ) {
			if ( layouts[array_range.min + i] != last_layout ) {
				if ( last_layout != newLayout )
					barriers[different_layout_index++] = transition_image_layout_impl ( last_layout, newLayout, mip_range, {startid, i - startid}, &sourceStage, &destinationStage );
				startid = i;
				last_layout = layouts[array_range.min + i];
			}
		}
		if ( last_layout != newLayout )
			barriers[different_layout_index] = transition_image_layout_impl ( last_layout, newLayout, mip_range, {startid, array_range.max - startid}, &sourceStage, &destinationStage );
	}

	for ( u32 i = 0; i < different_layout_count; ++i ) {
		barriers[i].srcQueueFamilyIndex = 0;
		barriers[i].dstQueueFamilyIndex = 0;
	}

	commandBuffer.pipelineBarrier (
	    sourceStage, destinationStage,
	    vk::DependencyFlags(),
	    {},//memoryBarriers
	    {},//bufferBarriers
	    vk::ArrayProxy<const vk::ImageMemoryBarrier> ( different_layout_count, barriers ) //imageBarriers
	);

	for ( size_t i = array_range.min; i < array_range.max; ++i ) {
		layouts[i] = newLayout;
	}
}

void VulkanWindowImage::generate_mipmaps ( Range<u32> mip_range, Range<u32> array_range, vk::ImageLayout targetLayout, vk::CommandBuffer commandBuffer ) {

}
void VulkanImageWrapper::generate_mipmaps ( Range<u32> mip_range, Range<u32> array_range, vk::ImageLayout targetLayout, vk::CommandBuffer commandBuffer ) {

	if ( mip_range.max == 0 )
		mip_range.max = mipmap_layers;
	if ( array_range.max == 0 )
		array_range.max = layers;

	transition_image_layout ( vk::ImageLayout::eTransferSrcOptimal, {mip_range.min, mip_range.min + 1}, array_range, commandBuffer );
	transition_image_layout ( vk::ImageLayout::eTransferDstOptimal, {mip_range.min + 1, mip_range.max}, array_range, commandBuffer );

	for ( u32 index = mip_range.min + 1; index < mip_range.max; index++ ) {
		// Set up a blit from previous mip-level to the next.
		vk::ImageBlit imageBlit ( vk::ImageSubresourceLayers ( aspect, index - 1, array_range.min, array_range.max - array_range.min ), {
			vk::Offset3D ( 0, 0, 0 ), vk::Offset3D ( std::max ( int ( extent.width >> ( index - 1 ) ), 1 ), std::max ( int ( extent.height >> ( index - 1 ) ), 1 ), std::max ( int ( extent.depth >> ( index - 1 ) ), 1 ) )
		}, vk::ImageSubresourceLayers ( aspect, index, array_range.min, array_range.max - array_range.min ), {
			vk::Offset3D ( 0, 0, 0 ), vk::Offset3D ( std::max ( int ( extent.width >> index ), 1 ), std::max ( int ( extent.height >> index ), 1 ), std::max ( int ( extent.depth >> index ), 1 ) )
		} );
		commandBuffer.blitImage ( image, vk::ImageLayout::eTransferSrcOptimal, image, vk::ImageLayout::eTransferDstOptimal, {imageBlit}, vk::Filter::eLinear );
	}
	transition_image_layout ( targetLayout, mip_range, array_range, commandBuffer );

}


VulkanWindowImage::VulkanWindowImage ( VulkanInstance* instance, u32 imagecount, vk::Image* images, vk::Extent3D extent, u32 layers, vk::Format format ) :
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
VulkanWindowImage::~VulkanWindowImage() {
	for ( PerImageData& data : per_image_data ) {
		v_instance->destroyImageView ( data.imageview );
	}
}

void VulkanWindowImage::set_current_image ( u32 index ) {
	current_index = index;
	image = per_image_data[current_index].image;
	imageview = per_image_data[current_index].imageview;
}