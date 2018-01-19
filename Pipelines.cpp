
#include "VHeader.h"
#include "VBuilders.h"
#include "VGlobal.h"


void destroyPipelineLayout (vk::PipelineLayout pipelineLayout) {
	vkDestroyPipelineLayout (global.deviceWrapper.device, pipelineLayout, nullptr);
}


void destroyPipeline (vk::Pipeline pipeline) {
	vkDestroyPipeline (global.deviceWrapper.device, pipeline, nullptr);
}

void destroyRenderPass(vk::RenderPass renderpass){
	vkDestroyRenderPass(global.deviceWrapper.device, renderpass, nullptr);
}