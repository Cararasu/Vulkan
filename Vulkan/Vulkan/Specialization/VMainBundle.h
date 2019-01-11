#ifndef MAINBUNDLE_H
#define MAINBUNDLE_H

#include "VSpecialStructs.h"

void gen_pipeline_layout ( VInstance* v_instance, SubPassInput* subpass_input, PipelineStruct* p_struct, PushConstUsed* pushconsts = nullptr );

void destroy_pipeline ( VInstance* v_instance, PipelineStruct* p_struct );
void destroy_pipeline_layout ( VInstance* v_instance, PipelineStruct* p_struct );


void update_instancegroup ( VInstance* v_instance, VInstanceGroup* igroup, vk::CommandBuffer buffer );
void update_contexts ( VInstance* v_instance, VContextGroup* cgroup, vk::CommandBuffer buffer );

void render_pipeline ( VInstance* v_instance, VInstanceGroup* igroup, VContextGroup* cgroup, PipelineStruct* p_struct, SubPassInput* renderpass_struct, vk::CommandBuffer cmdbuffer, u32 pipeline_index = 0 );
void render_pipeline_lod ( VInstance* v_instance, VInstanceGroup* igroup, VContextGroup* cgroup, PipelineStruct* p_struct, SubPassInput* renderpass_struct, vk::CommandBuffer cmdbuffer, u32 pipeline_index );

struct VShadowRenderStage : public VRenderStage {
	VInstance* v_instance;

	VInstanceGroup* v_igroup;

	PipelineStruct shadowmap_pipeline;

	Viewport<f32> viewport;

	vk::RenderPass v_renderpass;

	vk::DescriptorPool input_ds_pool;
	Array<SubPassInput> subpass_inputs;

	u64 last_frame_index_pipeline_built = 0;

	VShadowRenderStage ( VInstance* v_instance, InstanceGroup* igroup );
	virtual ~VShadowRenderStage();
	/*
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
	 */
};
struct VScaleDownRenderStage : public VRenderStage {
	VInstance* v_instance;

	VScaleDownRenderStage ( VInstance* v_instance );
	virtual ~VScaleDownRenderStage();

	virtual void set_renderimage ( u32 index, Image* image, u32 miplayer = 0, u32 arraylayer = 0 ) override;
	virtual void set_renderwindow ( u32 index, Window* window ) override;

	virtual void v_dispatch ( vk::CommandBuffer buffer, u32 index ) override;
};
struct VCopyToScreenRenderStage : public VRenderStage {
	VInstance* v_instance;
	VWindow* v_window;

	VCopyToScreenRenderStage ( VInstance* v_instance );
	virtual ~VCopyToScreenRenderStage();

	virtual void set_renderimage ( u32 index, Image* image, u32 miplayer = 0, u32 arraylayer = 0 ) override;
	virtual void set_renderwindow ( u32 index, Window* window ) override;

	virtual void v_dispatch ( vk::CommandBuffer buffer, u32 index ) override;
};
struct VMainBundle : public RenderBundle {
	Array<VRenderStage*> stages;
	DynArray<std::pair<u32, u32>> dependencies;
	VWindow* window_dependency;

	VInstance* v_instance;

	vk::CommandPool commandpool;
	Array<PerFrameRenderObj> v_per_frame_data;

	u64 last_used = 0;

	VMainBundle ( VInstance* v_instance, InstanceGroup* igroup );
	virtual ~VMainBundle();

	virtual void add_dependency ( u32 src_index, u32 dst_index ) override;
	virtual void remove_dependency ( u32 src_index, u32 dst_index ) override;

	virtual void set_window_dependency ( Window* window ) override;
	virtual void clear_window_dependency( ) override;

	virtual void set_renderstage ( u32 index, RenderStage* renderstage ) override;
	virtual RenderStage* get_renderstage ( u32 index ) override;
	virtual RenderStage* remove_renderstage ( u32 index ) override;

	void v_dispatch ( );
};

#endif // MAINBUNDLE_H
