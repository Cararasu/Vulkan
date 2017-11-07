
#include "VHeader.h"
#include "VGlobal.h"

vk::RenderPass standardRenderPass = vk::RenderPass();

vk::RenderPass createStandardRenderPass(vk::Format targetFormat){
	
	destroyStandardRenderPass();
	
	vk::AttachmentDescription attachments[2] = {
		vk::AttachmentDescription(vk::AttachmentDescriptionFlags(),
			targetFormat, vk::SampleCountFlagBits::e1,//format, samples
			vk::AttachmentLoadOp::eClear,//loadOp
			vk::AttachmentStoreOp::eStore,//storeOp
			vk::AttachmentLoadOp::eDontCare,//stencilLoadOp
			vk::AttachmentStoreOp::eDontCare,//stencilLoadOp
			vk::ImageLayout::eUndefined,//initialLaylout
			vk::ImageLayout::ePresentSrcKHR//finalLayout
		),
		vk::AttachmentDescription(vk::AttachmentDescriptionFlags(),
			findDepthFormat(), vk::SampleCountFlagBits::e1,//format, samples
			vk::AttachmentLoadOp::eClear,//loadOp
			vk::AttachmentStoreOp::eDontCare,//storeOp
			vk::AttachmentLoadOp::eDontCare,//stencilLoadOp
			vk::AttachmentStoreOp::eDontCare,//stencilLoadOp
			vk::ImageLayout::eUndefined,//initialLaylout
			vk::ImageLayout::eDepthStencilAttachmentOptimal//finalLayout
		)
	};
	
	vk::AttachmentReference colorAttachmentRef(0, vk::ImageLayout::eColorAttachmentOptimal);
	vk::AttachmentReference depthAttachmentRef(1, vk::ImageLayout::eDepthStencilAttachmentOptimal);
	
	vk::SubpassDescription subpass(
			vk::SubpassDescriptionFlags(), vk::PipelineBindPoint::eGraphics,
			0, nullptr/*inputAttackments*/, 
			1, &colorAttachmentRef/*colorAttachments*/, 
			nullptr,/*resolveAttachments*/
			&depthAttachmentRef,/*depthAttackment*/
			0, nullptr/*preserveAttachments*/
		);
	
	vk::RenderPassCreateInfo renderPassInfo(vk::RenderPassCreateFlags(), 2, attachments, 1, &subpass, 0, nullptr/*dependencies*/);
	
	return standardRenderPass = vGlobal.deviceWrapper.device.createRenderPass(renderPassInfo, nullptr);
}
void destroyStandardRenderPass(){
	if(standardRenderPass)
		destroyRenderPass(standardRenderPass);
	standardRenderPass = vk::RenderPass();
}
void destroyRenderPass(vk::RenderPass renderpass){
	vkDestroyRenderPass(vGlobal.deviceWrapper.device, renderpass, nullptr);
}