
#ifndef PIPELINE_MODULE_H
#define PIPELINE_MODULE_H

#include <vulkan/vulkan.hpp>
#include <vector>

struct VInstance;

struct PipelineModuleLayout {
	std::vector<vk::DescriptorSetLayout> descriptorSetLayouts;
	std::vector<vk::PushConstantRange> pushConstRanges;
	vk::PipelineLayout pipelineLayout;
};
struct PipelineModule {
	PipelineModuleLayout* modulelayout;
	vk::RenderPass renderPass;
	vk::Pipeline pipeline;
};

struct PipelineModuleBuilderTemplate {
	std::vector<const char*>  shader_files = {};//or shader_names

	std::vector<vk::DescriptorSetLayout> createDescriptorSetLayouts();
	std::vector<vk::PushConstantRange> createPushConstantRanges();
	vk::PipelineLayout createPipelineLayout (std::vector<vk::DescriptorSetLayout>& descriptorSetLayouts, std::vector<vk::PushConstantRange>& pushConstRanges);

	void destroyDescriptorSetLayouts (std::vector<vk::DescriptorSetLayout> descriptorSetLayouts);
	void destroyPipelineLayout (vk::Pipeline pipeline);

	vk::RenderPass createRenderPass (vk::Format format);
	vk::Pipeline createPipeline (vk::Extent2D extent, vk::RenderPass renderPass, vk::PipelineLayout layout);

	void destroyRenderPass (vk::RenderPass renderPass);
	void destroyPipeline (vk::Pipeline pipeline);
};
template<typename BUILDER>
PipelineModuleLayout createPipelineModuleLayout (BUILDER* builder) {
	PipelineModuleLayout pML;
	pML.descriptorSetLayouts = builder->createDescriptorSetLayouts();
	pML.pushConstRanges = builder->createPushConstantRanges();
	pML.pipelineLayout = builder->createPipelineLayout (pML.descriptorSetLayouts, pML.pushConstRanges);
	return pML;
}
template<typename BUILDER>
PipelineModule createPipelineModule (BUILDER* builder, PipelineModuleLayout* moduleLayout, vk::Format format, vk::Extent2D extent2D) {
	PipelineModule pM;
	pM.modulelayout = moduleLayout;
	pM.renderPass = builder->createRenderPass (format);
	pM.pipeline = builder->createPipeline (extent2D, pM.renderPass, moduleLayout->pipelineLayout);
	return pM;
}
template<typename BUILDER>
void updateExtent (BUILDER* builder, PipelineModule* module, vk::Extent2D extent2D) {
	if (module->pipeline)
		builder->destroyPipeline (module->pipeline);
	module->pipeline = builder->createPipeline (extent2D, module->renderPass, module->modulelayout->pipelineLayout);
}
template<typename BUILDER>
void deletePipelineModule (BUILDER* builder, PipelineModule module) {
	if (module.pipeline)
		builder->destroyPipeline (module.pipeline);
	if (module.renderPass)
		builder->destroyRenderPass (module.renderPass);
}
template<typename BUILDER>
void deletePipelineModuleLayout (BUILDER* builder, PipelineModuleLayout moduleLayout) {
	if (moduleLayout.descriptorSetLayouts.size() != 0)
		builder->destroyDescriptorSetLayouts (moduleLayout.descriptorSetLayouts);
	if (moduleLayout.pipelineLayout)
		builder->destroyPipelineLayout (moduleLayout.pipelineLayout);
}



//ShaderStore
//	for every vk::ShaderStageFlagBits


struct StandardPipelineModuleBuilder {
	VInstance* instance;
	std::vector<const char*> shader_files = {"../workingdir/shader/tri.vert", "../workingdir/shader/tri.frag"};//or shader_names

	std::vector<vk::DescriptorSetLayout> createDescriptorSetLayouts();
	std::vector<vk::PushConstantRange> createPushConstantRanges();
	vk::PipelineLayout createPipelineLayout (std::vector<vk::DescriptorSetLayout>& descriptorSetLayouts, std::vector<vk::PushConstantRange>& pushConstRanges);

	void destroyDescriptorSetLayouts (std::vector<vk::DescriptorSetLayout> descriptorSetLayouts);
	void destroyPipelineLayout (vk::PipelineLayout pipelineLayout);

	vk::RenderPass createRenderPass (vk::Format format);
	vk::Pipeline createPipeline (vk::Extent2D extent, vk::RenderPass renderPass, vk::PipelineLayout layout);

	void destroyRenderPass (vk::RenderPass renderPass);
	void destroyPipeline (vk::Pipeline pipeline);
};


#endif // PIPELINE_MODULE_H