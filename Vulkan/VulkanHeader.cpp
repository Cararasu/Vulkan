
#include "VulkanHeader.h"
#include "VulkanInstance.h"

RendResult submit_command( std::function<RendResult ( vk::CommandBuffer ) > do_command, VulkanInstance* instance, vk::CommandPool commandPool, vk::Queue submitQueue) {
	vk::CommandBuffer commandBuffer = instance->createCommandBuffer ( commandPool, vk::CommandBufferLevel::ePrimary );

	commandBuffer.begin ( vk::CommandBufferBeginInfo ( vk::CommandBufferUsageFlagBits::eOneTimeSubmit ) );

	RendResult res = do_command ( commandBuffer );

	commandBuffer.end();
	vk::SubmitInfo submitInfo (
	    0, nullptr,//waitSemaphores
	    nullptr,//pWaitDstStageMask
	    1, &commandBuffer,
	    0, nullptr//signalsemaphores
	);
	submitQueue.submit ( {submitInfo}, vk::Fence() );
	return res;
}
