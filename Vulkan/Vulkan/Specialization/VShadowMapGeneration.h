#pragma once

#include "VSpecialStructs.h"

void gen_model1_pipeline ( VInstance* v_instance, PipelineStruct* p_struct, u32 pipeline_index, Viewport<f32> viewport, vk::RenderPass renderpass );
void gen_model2_pipeline ( VInstance* v_instance, PipelineStruct* p_struct, u32 pipeline_index, Viewport<f32> viewport, vk::RenderPass renderpass );

struct VShadowMapGeneration : public VRenderStage {
	VInstance* v_instance;

	PipelineStruct model1_pipeline;
	PipelineStruct model2_pipeline;
	
	Viewport<f32> viewport;
	
	Array<RenderPassWrapper> renderpasses;

	Array<SubPassInput> subpass_inputs;
	
	u64 last_frame_index_pipeline_built = 0;

	VShadowMapGeneration ( VInstance* v_instance );
	virtual ~VShadowMapGeneration();
	
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