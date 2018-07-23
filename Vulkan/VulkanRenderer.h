#pragma once

#include "VulkanInstance.h"
#include "VulkanHeader.h"
#include "VulkanWindowSection.h"
#include "render/Dimensions.h"
#include "VulkanWindow.h"

//RenderStage
//    setup_sections
//    render_sections
//    finalize_sections
//RenderSection
//    List of renderers
//Renderer
//    models

struct VulkanRenderer {
	VulkanInstance* const v_instance;
	bool rebuild_on_instance_change;
	bool rebuild_every_time;
	
	Viewport<f32> viewport;

	Array<vk::DescriptorSetLayout> descriptor_set_layouts;
	vk::DescriptorPool descriptorPool;

	vk::PipelineLayout pipeline_layout;
	vk::RenderPass renderpass;
	vk::Pipeline pipeline;

	vk::CommandPool commandpool;

	VulkanRenderer ( VulkanInstance* instance ) : v_instance ( instance ) {}
	virtual ~VulkanRenderer() {}

	//virtual RendResult render(u32 frame_index, vk::CommandBuffer commandbuffer) = 0;
	//virtual RendResult render(u32 frame_index, vk::CommandBuffer commandbuffer) = 0;

	virtual void inherit ( VulkanRenderer* oldrenderer ) = 0;

	virtual RendResult update_extend ( Viewport<f32> viewport ) = 0;
	virtual RendResult update_extend ( Viewport<f32> viewport, VulkanRenderTarget* target_wrapper ) = 0;

	virtual RendResult render ( u32 frame_index, SubmitStore* state, u32 wait_sem_index, u32* final_sem_index ) = 0;

};
