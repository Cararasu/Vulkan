#pragma once

#include "VSpecialStructs.h"

void gen_tex_pipeline ( VInstance* v_instance, PipelineStruct* p_struct, Viewport<f32> viewport, vk::RenderPass renderpass, u32 pipeline_index = 0 );
void gen_flat_pipeline ( VInstance* v_instance, PipelineStruct* p_struct, Viewport<f32> viewport, vk::RenderPass renderpass, u32 pipeline_index = 0 );
void gen_skybox_pipeline ( VInstance* v_instance, PipelineStruct* p_struct, Viewport<f32> viewport, vk::RenderPass renderpass, u32 pipeline_index = 0 );

void gen_lightless_pipeline ( VInstance* v_instance, PipelineStruct* p_struct, Viewport<f32> viewport, vk::RenderPass renderpass, u32 pipeline_index = 0 );
void gen_dirlight_pipeline ( VInstance* v_instance, PipelineStruct* p_struct, Viewport<f32> viewport, vk::RenderPass renderpass, u32 pipeline_index = 0 );
void gen_shotlight_pipeline ( VInstance* v_instance, PipelineStruct* p_struct, Viewport<f32> viewport, vk::RenderPass renderpass, u32 pipeline_index = 0 );
void gen_shot_pipeline ( VInstance* v_instance, PipelineStruct* p_struct, Viewport<f32> viewport, vk::RenderPass renderpass, u32 pipeline_index = 0 );
void gen_billboard_pipeline ( VInstance* v_instance, PipelineStruct* p_struct, Viewport<f32> viewport, vk::RenderPass renderpass, u32 pipeline_index = 0 );

struct VMainRenderStage : public VRenderStage {
	VInstance* v_instance;
	
	VInstanceGroup* v_igroup;

	PipelineStruct skybox_pipeline;
	PipelineStruct tex_pipeline;
	PipelineStruct flat_pipeline;
	
	PipelineStruct lightless_pipeline;
	PipelineStruct dirlight_pipeline;
	PipelineStruct shotlight_pipeline;
	PipelineStruct shot_pipeline;
	PipelineStruct billboard_pipeline;
	
	Viewport<f32> viewport;
	
	vk::CommandPool commandpool;
	Array<PerFrameRenderObj> v_per_frame_data;
	vk::RenderPass v_renderpass;
	
	vk::DescriptorPool input_ds_pool;
	Array<SubPassInput> subpass_inputs;

	u64 last_frame_index_pipeline_built = 0;
	
	VMainRenderStage ( VInstance* v_instance, InstanceGroup* igroup );
	virtual ~VMainRenderStage();
	
	void v_destroy_pipeline_layouts();
	void v_destroy_pipelines();
	void v_destroy_renderpasses();
	void v_destroy_framebuffers();

	void v_check_rebuild();
	void v_rebuild_pipelines();
	void v_rebuild_commandbuffer ( u32 index );
	
	virtual void set_renderimage ( u32 index, Image* image, u32 miplayer = 0, u32 arraylayer = 0 ) override;
	virtual void set_renderwindow ( u32 index, Window* window ) override;
	
	virtual void v_dispatch ( vk::CommandBuffer buffer, u32 index ) override;
};
