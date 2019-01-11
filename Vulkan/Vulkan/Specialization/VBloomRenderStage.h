#pragma once

#include "VSpecialStructs.h"

void gen_brightness_pipeline ( VInstance* v_instance, PipelineStruct* p_struct, u32 pipeline_index, Viewport<f32> viewport, vk::RenderPass renderpass );
void gen_hbloom_pipeline ( VInstance* v_instance, PipelineStruct* p_struct, u32 pipeline_index, Viewport<f32> viewport, vk::RenderPass renderpass );
void gen_vbloom_pipeline ( VInstance* v_instance, PipelineStruct* p_struct, u32 pipeline_index, Viewport<f32> viewport, vk::RenderPass renderpass);

struct VBrightnessRenderStage : public VRenderStage {
	VInstance* v_instance;
	
	VInstanceGroup* v_igroup;

	PipelineStruct brightness_pipeline;
	
	Viewport<f32> viewport;
	
	Array<RenderPassWrapper> renderpasses;

	Array<SubPassInput> subpass_inputs;
	
	u64 last_frame_index_pipeline_built = 0;
	
	VBrightnessRenderStage ( VInstance* v_instance, InstanceGroup* igroup );
	virtual ~VBrightnessRenderStage();
	
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

struct VBloomRenderStage : public VRenderStage {
	VInstance* v_instance;
	
	VInstanceGroup* v_igroup;

	PipelineStruct bloom_pipeline;
	
	Viewport<f32> viewport;
	
	Array<RenderPassWrapper> renderpasses;

	Array<SubPassInput> subpass_inputs;
	
	u64 last_frame_index_pipeline_built = 0;
	
	VBloomRenderStage ( VInstance* v_instance, InstanceGroup* igroup );
	virtual ~VBloomRenderStage();
	
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

struct HBloomRenderStage : public VRenderStage {
	VInstance* v_instance;
	
	VInstanceGroup* v_igroup;

	PipelineStruct bloom_pipeline;
	
	Viewport<f32> viewport;
	
	Array<RenderPassWrapper> renderpasses;

	Array<SubPassInput> subpass_inputs;
	
	u64 last_frame_index_pipeline_built = 0;
	
	HBloomRenderStage ( VInstance* v_instance, InstanceGroup* igroup );
	virtual ~HBloomRenderStage();
	
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