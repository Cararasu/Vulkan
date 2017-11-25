
#include "VHeader.h"
#include "VGlobal.h"


/*
Light: B10G11R11_UFLOAT_PACK32
Albedo: RGBA8
Normal: RGB10A2
PBR/Misc: RGBA8
 * */

vk::RenderPass createStandardRenderPass(vk::Format targetFormat){
	
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
	
	return global.deviceWrapper.device.createRenderPass(renderPassInfo, nullptr);
}
void destroyRenderPass(vk::RenderPass renderpass){
	vkDestroyRenderPass(global.deviceWrapper.device, renderpass, nullptr);
}