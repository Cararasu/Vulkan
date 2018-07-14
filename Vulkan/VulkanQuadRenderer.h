#pragma once
#include "VulkanInstance.h"
#include "VulkanBuffer.h"
#include "render/Dimensions.h"

struct PerFrameQuadRenderObj {
	vk::Framebuffer framebuffer;
	vk::CommandBuffer commandbuffer;
};

struct VulkanRenderTarget;

struct VulkanQuadRenderer {
	VulkanInstance* const v_instance;
	Viewport<f32> viewport;

	std::array<vk::DescriptorSetLayout, 0> descriptor_set_layouts;
	vk::PipelineLayout pipeline_layout;
	vk::ShaderModule vertex_shader;
	vk::ShaderModule fragment_shader;
	vk::RenderPass renderpass;
	vk::Pipeline pipeline;

	vk::CommandPool g_commandpool;
	vk::CommandPool t_commandpool;

	VulkanBuffer* vertex_buffer = nullptr;
	VulkanBuffer* staging_vertex_buffer = nullptr;

	Array<PerFrameQuadRenderObj> per_target_data;

	VulkanQuadRenderer ( VulkanInstance* instance );
	VulkanQuadRenderer ( VulkanQuadRenderer* old_quad_renderer );
	~VulkanQuadRenderer();

	void inherit ( VulkanQuadRenderer* oldquadrenderer );
	void destroy_framebuffers();

	RendResult update_extend ( Viewport<f32> viewport, VulkanRenderTarget* target_wrapper );
	vk::CommandBuffer render_quads ( u32 index );
	void remove ( );
};
