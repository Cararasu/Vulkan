#pragma once

#include "VSpecialStructs.h"

void gen_hbloom_pipeline ( VInstance* v_instance, PipelineStruct* p_struct, Viewport<f32> viewport, vk::RenderPass renderpass );
void gen_vbloom_pipeline ( VInstance* v_instance, PipelineStruct* p_struct, Viewport<f32> viewport, vk::RenderPass renderpass );

struct VBloomRenderStage : public VRenderStage {
	VInstance* v_instance;
	
	VInstanceGroup* v_igroup;
	VContextGroup* v_cgroup;

	PipelineStruct bloom_pipeline;
	
	Viewport<f32> viewport;
	
	Array<PerFrameRenderObj> v_per_frame_data;
	vk::RenderPass v_renderpass;

	Array<SubPassInput> subpass_inputs;
	
	u64 last_frame_index_pipeline_built = 0;
	
	VBloomRenderStage ( VInstance* v_instance, InstanceGroup* igroup, ContextGroup* cgroup );
	virtual ~VBloomRenderStage();
	
	void v_destroy_pipeline_layouts();
	void v_destroy_pipelines();
	void v_destroy_renderpasses();
	void v_destroy_framebuffers();

	void v_check_rebuild();
	void v_rebuild_pipelines();
	void v_rebuild_commandbuffer ( u32 index );
	
	virtual void set_renderimage ( u32 index, Image* image, Range<u32> miprange = {0, 1}, Range<u32> layers = {0, 1} ) override;
	virtual void set_renderwindow ( u32 index, Window* window ) override;
	
	virtual void v_dispatch ( vk::CommandBuffer buffer, u32 index ) override;
};

struct HBloomRenderStage : public VRenderStage {
	VInstance* v_instance;
	
	VInstanceGroup* v_igroup;
	VContextGroup* v_cgroup;

	PipelineStruct bloom_pipeline;
	
	Viewport<f32> viewport;
	
	Array<PerFrameRenderObj> v_per_frame_data;
	vk::RenderPass v_renderpass;

	Array<SubPassInput> subpass_inputs;
	
	u64 last_frame_index_pipeline_built = 0;
	
	HBloomRenderStage ( VInstance* v_instance, InstanceGroup* igroup, ContextGroup* cgroup );
	virtual ~HBloomRenderStage();
	
	void v_destroy_pipeline_layouts();
	void v_destroy_pipelines();
	void v_destroy_renderpasses();
	void v_destroy_framebuffers();

	void v_check_rebuild();
	void v_rebuild_pipelines();
	void v_rebuild_commandbuffer ( u32 index );
	
	virtual void set_renderimage ( u32 index, Image* image, Range<u32> miprange = {0, 1}, Range<u32> layers = {0, 1} ) override;
	virtual void set_renderwindow ( u32 index, Window* window ) override;
	
	virtual void v_dispatch ( vk::CommandBuffer buffer, u32 index ) override;
};