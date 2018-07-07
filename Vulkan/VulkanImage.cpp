
#include "VulkanImage.h"

VulkanImageWrapper::VulkanImageWrapper ( VulkanInstance* instance, vk::Extent3D extent, u32 mipMapLevels, u32 arraySize, vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags usage, vk::ImageAspectFlags aspectFlags, vk::MemoryPropertyFlags needed, vk::MemoryPropertyFlags recommended ) :
	instance ( instance ), memory(), image(), extent ( extent ), mipMapLevels ( mipMapLevels ), arraySize ( arraySize ), format ( format ), tiling ( tiling ), usage ( usage ), type(), layouts ( arraySize, vk::ImageLayout::eUndefined ), aspectFlags ( aspectFlags ) {
	if ( extent.height == 0 ) {
		extent.height = 1;
		extent.depth = 1;
		type = vk::ImageType::e1D;
	} else if ( extent.depth == 0 ) {
		extent.depth = 1;
		type = vk::ImageType::e2D;
	} else
		type = vk::ImageType::e3D;

	vk::ImageCreateInfo imageInfo ( vk::ImageCreateFlags(), type, format, extent, mipMapLevels, arraySize, vk::SampleCountFlagBits::e1, tiling, usage, vk::SharingMode::eExclusive, 0, nullptr, vk::ImageLayout::eUndefined );

	V_CHECKCALL ( instance->m_device.createImage ( &imageInfo, nullptr, &image ), printf ( "Failed To Create Image\n" ) );

	printf ( "Create Image of dimensions %dx%dx%d\n", extent.width, extent.height, extent.depth );

	vk::MemoryRequirements mem_req;
	instance->m_device.getImageMemoryRequirements ( image, &mem_req );
	memory = instance->allocate_gpu_memory(mem_req, needed, recommended);
	vkBindImageMemory ( instance->m_device, image, memory.memory, 0 );
}
VulkanImageWrapper::VulkanImageWrapper ( VulkanInstance* instance, vk::Image image, vk::Extent3D extent, u32 mipMapLevels, u32 arraySize, vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags usage, vk::ImageAspectFlags aspectFlags ) :
	instance ( instance ), memory(), image ( image ), extent ( extent ), mipMapLevels ( mipMapLevels ), arraySize ( arraySize ), format ( format ), tiling ( tiling ), usage ( usage ), type(), layouts ( arraySize, vk::ImageLayout::eUndefined ), aspectFlags ( aspectFlags ) {

}
VulkanImageWrapper::~VulkanImageWrapper() {
	destroy();
}
void VulkanImageWrapper::destroy() {
	if ( memory.memory ) { //if the image is managed externally
		instance->m_device.destroyImage ( image );
		image = vk::Image();
		printf ( "Freeing %d Bytes of Memory\n", memory.size );
		instance->m_device.freeMemory ( memory.memory );
		memory.memory = vk::DeviceMemory();
	}
}
vk::ImageMemoryBarrier VulkanImageWrapper::transition_image_layout_impl ( vk::ImageLayout oldLayout, vk::ImageLayout newLayout, Range<u32> miprange, Range<u32> arrayrange, vk::PipelineStageFlags* srcStageFlags, vk::PipelineStageFlags* dstStageFlags ) {

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
	           vk::ImageSubresourceRange ( aspectFlags, miprange.min, miprange.max - miprange.min, arrayrange.min, arrayrange.max - arrayrange.min )
	       );
}
void VulkanImageWrapper::transition_image_layout ( vk::ImageLayout newLayout, Range<u32> mip_range, Range<u32> array_range, vk::CommandBuffer commandBuffer ) {

	if ( mip_range.max == 0 )
		mip_range.max = mipMapLevels;
	if ( array_range.max == 0 )
		array_range.max = arraySize;

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

void VulkanImageWrapper::generate_mipmaps ( Range<u32> mip_range, Range<u32> array_range, vk::ImageLayout targetLayout, vk::CommandBuffer commandBuffer ) {

	if ( mip_range.max == 0 )
		mip_range.max = mipMapLevels;
	if ( array_range.max == 0 )
		array_range.max = arraySize;

	transition_image_layout ( vk::ImageLayout::eTransferSrcOptimal, {mip_range.min, mip_range.min + 1}, array_range, commandBuffer );
	transition_image_layout ( vk::ImageLayout::eTransferDstOptimal, {mip_range.min + 1, mip_range.max}, array_range, commandBuffer );

	for ( u32 index = mip_range.min + 1; index < mip_range.max; index++ ) {
		// Set up a blit from previous mip-level to the next.
		vk::ImageBlit imageBlit ( vk::ImageSubresourceLayers ( aspectFlags, index - 1, array_range.min, array_range.max - array_range.min ), {
			vk::Offset3D ( 0, 0, 0 ), vk::Offset3D ( std::max ( int ( extent.width >> ( index - 1 ) ), 1 ), std::max ( int ( extent.height >> ( index - 1 ) ), 1 ), std::max ( int ( extent.depth >> ( index - 1 ) ), 1 ) )
		}, vk::ImageSubresourceLayers ( aspectFlags, index, array_range.min, array_range.max - array_range.min ), {
			vk::Offset3D ( 0, 0, 0 ), vk::Offset3D ( std::max ( int ( extent.width >> index ), 1 ), std::max ( int ( extent.height >> index ), 1 ), std::max ( int ( extent.depth >> index ), 1 ) )
		} );
		commandBuffer.blitImage ( image, vk::ImageLayout::eTransferSrcOptimal, image, vk::ImageLayout::eTransferDstOptimal, {imageBlit}, vk::Filter::eLinear );
	}
	transition_image_layout ( targetLayout, mip_range, array_range, commandBuffer );

}