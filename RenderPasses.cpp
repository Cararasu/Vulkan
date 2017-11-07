
#include "VHeader.h"
#include "VGlobal.h"

vk::RenderPass standardRenderPass = VK_NULL_HANDLE;

vk::RenderPass createStandardRenderPass(vk::Format targetFormat){
	
	destroyStandardRenderPass();
	
	vk::AttachmentDescription attachments[2] = {
		vk::AttachmentDescription(vk::AttachmentDescriptionFlags(),
			targetFormat, vk::SampleCountFlagBits::e1,
			vk::AttachmentLoadOp::eClear,
			vk::AttachmentStoreOp::eStore,
			vk::AttachmentLoadOp::eDontCare,
			vk::AttachmentStoreOp::eDontCare,
			vk::ImageLayout::eUndefined,
			vk::ImageLayout::ePresentSrcKHR
		),
		vk::AttachmentDescription(vk::AttachmentDescriptionFlags(),
			targetFormat, vk::SampleCountFlagBits::e1,
			vk::AttachmentLoadOp::eClear,
			vk::AttachmentStoreOp::eDontCare,
			vk::AttachmentLoadOp::eDontCare,
			vk::AttachmentStoreOp::eDontCare,
			vk::ImageLayout::eUndefined,
			vk::ImageLayout::eDepthStencilAttachmentOptimal
		)
	};
	
	
	vk::AttachmentReference colorAttachmentRef(0, vk::ImageLayout::eColorAttachmentOptimal);
	
	vk::AttachmentReference depthAttachmentRef(1, vk::ImageLayout::eDepthStencilAttachmentOptimal);
	
	vk::SubpassDescription subpass(vk::SubpassDescriptionFlags(), vk::PipelineBindPoint::eGraphics, 0, nullptr/*inputAttackment*/, 1, &colorAttachmentRef/*colorAttachment*/, &depthAttachmentRef/*depthAttackment*/);
	
	vk::RenderPassCreateInfo renderPassInfo(vk::RenderPassCreateFlags(), 2, attachments, 1, &subpass, 0, nullptr/*dependencies*/);

	V_CHECKCALL(vGlobal.deviceWrapper.device.createRenderPass(&renderPassInfo, nullptr, &standardRenderPass), printf("Creation of RenderPass failed\n"));
	
	return standardRenderPass;
}
void destroyStandardRenderPass(){
	if(standardRenderPass)
		destroyRenderPass(standardRenderPass);
	standardRenderPass = VK_NULL_HANDLE;
}
void destroyRenderPass(vk::RenderPass renderpass){
	vkDestroyRenderPass(vGlobal.deviceWrapper.device, renderpass, nullptr);
}