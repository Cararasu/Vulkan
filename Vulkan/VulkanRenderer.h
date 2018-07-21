#pragma once

/*
struct VulkanRenderer {
	VulkanInstance* const v_instance;
	Viewport<f32> viewport;

	vk::PipelineLayout pipeline_layout;
	vk::RenderPass renderpass;
	vk::Pipeline pipeline;

	vk::CommandPool g_commandpool;
	vk::CommandPool t_commandpool;

	VulkanRenderer();
	~VulkanRenderer();


	virtual void init() = 0;
	virtual void inherit ( VulkanRenderer* oldrenderer ) = 0;

	virtual RendResult update_extend ( Viewport<f32> viewport, VulkanRenderTarget* target_wrapper ) = 0;
	virtual vk::CommandBuffer render_quads ( u32 index ) = 0;
	virtual RendResult render ( u32 frame_index, SubmitStore* state, u32 wait_sem_index, u32* final_sem_index ) = 0;
	virtual void destroy ( ) = 0;

};
*/