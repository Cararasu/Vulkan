
#include "VulkanImage.h"

VulkanImageWrapper::VulkanImageWrapper (VulkanInstance* instance, vk::Extent3D extent, u32 mipMapLevels, u32 arraySize, vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags usage, vk::ImageAspectFlags aspectFlags, vk::MemoryPropertyFlags needed, vk::MemoryPropertyFlags recommended) :
	instance (instance), memory(), image(), extent (extent), mipMapLevels (mipMapLevels), arraySize (arraySize), format (format), tiling (tiling), usage (usage), type(), layouts (arraySize, vk::ImageLayout::eUndefined), aspectFlags (aspectFlags) {
	if (extent.height == 1)
		type = vk::ImageType::e1D;
	else if (extent.depth == 1)
		type = vk::ImageType::e2D;
	else
		type = vk::ImageType::e3D;

	vk::ImageCreateInfo imageInfo (vk::ImageCreateFlags(), type, format, extent, mipMapLevels, arraySize, vk::SampleCountFlagBits::e1, tiling, usage, vk::SharingMode::eExclusive, 0, nullptr, vk::ImageLayout::eUndefined);

	V_CHECKCALL (instance->m_device.createImage (&imageInfo, nullptr, &image), printf ("Failed To Create Image\n"));

	printf ("Create Image of dimensions %dx%dx%d\n", extent.width, extent.height, extent.depth);

	vk::MemoryRequirements memRequirements;
	instance->m_device.getImageMemoryRequirements (image, &memRequirements);
	memory.memory = instance->allocateMemory (memRequirements, needed | recommended);
	memory.size = memRequirements.size;
	if (!memory.memory)
		memory.memory = instance->allocateMemory (memRequirements, needed);

	vkBindImageMemory (instance->m_device, image, memory.memory, 0);
}
VulkanImageWrapper::~VulkanImageWrapper() {
}
void VulkanImageWrapper::destroy() {
	vkDestroyImage (instance->m_device, image, nullptr);
	vkFreeMemory (instance->m_device, memory.memory, nullptr);
}
vk::ImageMemoryBarrier VulkanImageWrapper::transition_image_layout_impl (vk::ImageLayout oldLayout, vk::ImageLayout newLayout, Range<u32> miprange, Range<u32> arrayrange, vk::PipelineStageFlags* srcStageFlags, vk::PipelineStageFlags* dstStageFlags) {

	vk::AccessFlags srcAccessMask, dstAccessMask;

	if (oldLayout == vk::ImageLayout::eUndefined) {
		srcAccessMask = vk::AccessFlags();
		*srcStageFlags |= vk::PipelineStageFlagBits::eTopOfPipe;
	} else if (oldLayout == vk::ImageLayout::eTransferDstOptimal) {
		srcAccessMask = vk::AccessFlagBits::eTransferWrite;
		*srcStageFlags |= vk::PipelineStageFlagBits::eTransfer;
	} else if (oldLayout == vk::ImageLayout::eTransferSrcOptimal) {
		srcAccessMask = vk::AccessFlagBits::eTransferRead;
		*srcStageFlags |= vk::PipelineStageFlagBits::eTransfer;
	} else if (oldLayout == vk::ImageLayout::eShaderReadOnlyOptimal) {
		srcAccessMask = vk::AccessFlagBits::eShaderRead;
		*srcStageFlags |= vk::PipelineStageFlagBits::eFragmentShader;
	} else {
		assert (false);
	}
	if (newLayout == vk::ImageLayout::eUndefined) {
		dstAccessMask = vk::AccessFlags();
		*dstStageFlags |= vk::PipelineStageFlagBits::eBottomOfPipe;
	} else if (newLayout == vk::ImageLayout::eTransferDstOptimal) {
		dstAccessMask = vk::AccessFlagBits::eTransferWrite;
		*dstStageFlags |= vk::PipelineStageFlagBits::eTransfer;
	} else if (newLayout == vk::ImageLayout::eTransferSrcOptimal) {
		dstAccessMask = vk::AccessFlagBits::eTransferRead;
		*dstStageFlags |= vk::PipelineStageFlagBits::eTransfer;
	} else if (newLayout == vk::ImageLayout::eDepthStencilAttachmentOptimal) {
		dstAccessMask = vk::AccessFlags (vk::AccessFlagBits::eDepthStencilAttachmentRead | vk::AccessFlagBits::eDepthStencilAttachmentWrite);
		*dstStageFlags |= vk::PipelineStageFlagBits::eEarlyFragmentTests;
	} else if (newLayout == vk::ImageLayout::eShaderReadOnlyOptimal) {
		dstAccessMask = vk::AccessFlagBits::eShaderRead;
		*dstStageFlags |= vk::PipelineStageFlagBits::eFragmentShader;
	} else {
		assert (false);
	}
	return vk::ImageMemoryBarrier (
	           srcAccessMask, dstAccessMask,
	           oldLayout, newLayout,
	           0, 0,
	           image,
	           vk::ImageSubresourceRange (aspectFlags, miprange.min, miprange.max - miprange.min, arrayrange.min, arrayrange.max - arrayrange.min)
	       );
}
void VulkanImageWrapper::transition_image_layout (vk::ImageLayout newLayout, Range<u32> mip_range, Range<u32> array_range, vk::CommandBuffer commandBuffer) {

	if (mip_range.max == 0)
		mip_range.max = mipMapLevels;
	if (array_range.max == 0)
		array_range.max = arraySize;

	vk::PipelineStageFlags sourceStage, destinationStage;
	u32 different_layout_count = layouts[array_range.min] == newLayout ? 0 : 1;
	for (u32 i = array_range.min + 1; i < array_range.max; ++i) {
		if (layouts[array_range.min + i] != layouts[array_range.min + i - 1] && layouts[array_range.min + i] != newLayout) {
			++different_layout_count;
		}
	}
	vk::ImageMemoryBarrier barriers[different_layout_count];

	{
		u32 different_layout_index = 0;
		u32 startid = array_range.min;
		vk::ImageLayout last_layout = layouts[0];
		for (u32 i = array_range.min + 1; i < array_range.max; ++i) {
			if (layouts[array_range.min + i] != last_layout) {
				if (last_layout != newLayout)
					barriers[different_layout_index++] = transition_image_layout_impl (last_layout, newLayout, mip_range, {startid, i - startid}, &sourceStage, &destinationStage);
				startid = i;
				last_layout = layouts[array_range.min + i];
			}
		}
		if (last_layout != newLayout)
			barriers[different_layout_index] = transition_image_layout_impl (last_layout, newLayout, mip_range, {startid, array_range.max - startid}, &sourceStage, &destinationStage);
	}

	for (u32 i = 0; i < different_layout_count; ++i) {
		barriers[i].srcQueueFamilyIndex = 0;
		barriers[i].dstQueueFamilyIndex = 0;
	}

	commandBuffer.pipelineBarrier (
	    sourceStage, destinationStage,
	    vk::DependencyFlags(),
	    {},//memoryBarriers
	    {},//bufferBarriers
	    vk::ArrayProxy<const vk::ImageMemoryBarrier> (different_layout_count, barriers) //imageBarriers
	);

	for (size_t i = array_range.min; i < array_range.max; ++i) {
		layouts[array_range.min + i] = newLayout;
	}
}

void VulkanImageWrapper::transition_image_layout (vk::ImageLayout newLayout, Range<u32> mip_range, Range<u32> array_range, vk::CommandPool commandPool, vk::Queue submitQueue) {

	vk::CommandBuffer commandBuffer = instance->createCommandBuffer (commandPool, vk::CommandBufferLevel::ePrimary);

	commandBuffer.begin (vk::CommandBufferBeginInfo (vk::CommandBufferUsageFlagBits::eOneTimeSubmit));

	transition_image_layout (newLayout, mip_range, array_range, commandBuffer);

	commandBuffer.end();
	vk::SubmitInfo submitInfo (
	    0, nullptr,//waitSemaphores
	    nullptr,//pWaitDstStageMask
	    1, &commandBuffer,
	    0, nullptr//signalsemaphores
	);
	submitQueue.submit ({submitInfo}, vk::Fence());
}
void VulkanImageWrapper::generate_mipmaps (Range<u32> mip_range, Range<u32> array_range, vk::ImageLayout targetLayout, vk::CommandBuffer commandBuffer) {

	if (mip_range.max == 0)
		mip_range.max = mipMapLevels;
	if (array_range.max == 0)
		array_range.max = arraySize;

	transition_image_layout (vk::ImageLayout::eTransferSrcOptimal, {mip_range.min, mip_range.min + 1}, array_range, commandBuffer);
	transition_image_layout (vk::ImageLayout::eTransferDstOptimal, {mip_range.min + 1, mip_range.max}, array_range, commandBuffer);

	for (u32 index = mip_range.min + 1; index < mip_range.max; index++) {
		// Set up a blit from previous mip-level to the next.
		vk::ImageBlit imageBlit (vk::ImageSubresourceLayers (aspectFlags, index - 1, array_range.min, array_range.max - array_range.min), {
			vk::Offset3D (0, 0, 0), vk::Offset3D (std::max (int (extent.width >> (index - 1)), 1), std::max (int (extent.height >> (index - 1)), 1), std::max (int (extent.depth >> (index - 1)), 1))
		}, vk::ImageSubresourceLayers (aspectFlags, index, array_range.min, array_range.max - array_range.min), {
			vk::Offset3D (0, 0, 0), vk::Offset3D (std::max (int (extent.width >> index), 1), std::max (int (extent.height >> index), 1), std::max (int (extent.depth >> index), 1))
		});
		commandBuffer.blitImage (image, vk::ImageLayout::eTransferSrcOptimal, image, vk::ImageLayout::eTransferDstOptimal, {imageBlit}, vk::Filter::eLinear);
	}
	transition_image_layout (targetLayout, mip_range, array_range, commandBuffer);

}
void VulkanImageWrapper::generate_mipmaps (Range<u32> mip_range, Range<u32> array_range, vk::ImageLayout targetLayout, vk::CommandPool commandPool, vk::Queue submitQueue) {

	vk::CommandBuffer commandBuffer = instance->createCommandBuffer (commandPool, vk::CommandBufferLevel::ePrimary);

	commandBuffer.begin (vk::CommandBufferBeginInfo (vk::CommandBufferUsageFlagBits::eOneTimeSubmit));

	generate_mipmaps (mip_range, array_range, targetLayout, commandBuffer);

	commandBuffer.end();
	vk::SubmitInfo submitInfo (
	    0, nullptr,//waitSemaphores
	    nullptr,//pWaitDstStageMask
	    1, &commandBuffer,
	    0, nullptr//signalsemaphores
	);
	submitQueue.submit ({submitInfo}, vk::Fence());

}
