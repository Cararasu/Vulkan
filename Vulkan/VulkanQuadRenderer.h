#pragma once
#include "VulkanInstance.h"
#include "VulkanWindowSection.h"
#include "render/Dimensions.h"

struct PerFrameQuadRenderObj{
	vk::Framebuffer framebuffer;
	vk::CommandBuffer commandbuffer;
};

struct VulkanQuadRenderer {
	VulkanInstance* v_instance;
	
	std::array<vk::DescriptorSetLayout, 2> descriptor_set_layouts;
	vk::PipelineLayout pipeline_layout;
	vk::ShaderModule vertex_shader;
	vk::ShaderModule fragment_shader;
	vk::RenderPass renderpass;
	vk::Pipeline pipeline;
	vk::CommandPool commandpool;
	
	Array<PerFrameQuadRenderObj> per_target_data;

	VulkanQuadRenderer ( VulkanInstance* instance ) : v_instance ( instance ) {}
	~VulkanQuadRenderer();

	void destroy_framebuffers();
	
	RendResult update_extend(Viewport<f32> viewport, VulkanRenderTarget* target_wrapper);
	RendResult render_quads(u32 index);
};
