#ifndef MAINBUNDLE_H
#define MAINBUNDLE_H

#include <render/Model.h>
#include <render/Resources.h>

#include "../VHeader.h"
#include <render/Dimensions.h>

struct VInstanceGroup;
struct VContextGroup;
struct VBaseImage;
struct VInstance;

struct VBundleImageState {
	vk::Format current_format = vk::Format::eUndefined;
	VBaseImage* actual_image = nullptr;
};
struct PerFrameMainBundleRenderObj {
	vk::Framebuffer framebuffer;
	ResettableCommandBuffer command;
};

struct PipelineStruct {
	ModelBaseId modelbase_id;
	InstanceBaseId instancebase_id;
	Array<ContextBaseId> contextBaseId;
	Array<ContextBaseId> model_contextBaseId;
	vk::PipelineLayout pipeline_layout;
	vk::Pipeline pipeline;
};

void gen_pipeline_layout ( VInstance* v_instance, PipelineStruct* p_struct );

void gen_tex_pipeline ( VInstance* v_instance, PipelineStruct* p_struct, Viewport<f32> viewport, vk::RenderPass renderpass );
void gen_flat_pipeline ( VInstance* v_instance, PipelineStruct* p_struct, Viewport<f32> viewport, vk::RenderPass renderpass );
void gen_skybox_pipeline ( VInstance* v_instance, PipelineStruct* p_struct, Viewport<f32> viewport, vk::RenderPass renderpass );
void gen_shot_pipeline ( VInstance* v_instance, PipelineStruct* p_struct, Viewport<f32> viewport, vk::RenderPass renderpass );
void gen_engine_pipeline ( VInstance* v_instance, PipelineStruct* p_struct, Viewport<f32> viewport, vk::RenderPass renderpass );

void destroy_pipeline ( VInstance* v_instance, PipelineStruct* p_struct );
void destroy_pipeline_layout ( VInstance* v_instance, PipelineStruct* p_struct );

void render_pipeline ( VInstance* v_instance, VInstanceGroup* igroup, VContextGroup* cgroup, PipelineStruct* p_struct, vk::CommandBuffer cmdbuffer );

struct VMainBundle : public RenderBundle {
	VInstance* v_instance;
	VInstanceGroup* v_igroup;
	VContextGroup* v_cgroup;
	Array<VBundleImageState> v_bundleStates;

	PipelineStruct skybox_pipeline;
	PipelineStruct tex_pipeline;
	PipelineStruct flat_pipeline;
	PipelineStruct shot_pipeline;
	PipelineStruct engine_pipeline;

	Viewport<f32> viewport;

	vk::CommandPool commandpool;
	Array<PerFrameMainBundleRenderObj> v_per_frame_data;
	vk::RenderPass v_renderpass;

	u64 last_frame_index_pipeline_built = 0;
	u64 last_used = 0;

	VMainBundle ( VInstance* v_instance, InstanceGroup* igroup, ContextGroup* cgroup );
	virtual ~VMainBundle();

	virtual void set_rendertarget ( u32 index, Image* image ) override;

	void v_destroy_pipeline_layouts();
	void v_destroy_pipelines();
	void v_destroy_renderpasses();
	void v_destroy_framebuffers();

	void v_check_rebuild();
	void v_rebuild_pipelines();
	void v_rebuild_commandbuffer ( u32 index );
	void v_dispatch();
};

#endif // MAINBUNDLE_H
