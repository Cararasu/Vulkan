#ifndef MAINBUNDLE_H
#define MAINBUNDLE_H

#include <render/Model.h>
#include <render/Resources.h>

#include "../VHeader.h"
#include "../VRenderer.h"
#include <render/Dimensions.h>
#include "../VWindow.h"

struct VInstanceGroup;
struct VContextGroup;
struct VBaseImage;
struct VInstance;

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
struct SubPassInput {
	vk::DescriptorSetLayout ds_layout;
	vk::DescriptorSet ds_set;
};

void gen_pipeline_layout ( VInstance* v_instance, SubPassInput* subpass_input, PipelineStruct* p_struct );

void gen_tex_pipeline ( VInstance* v_instance, PipelineStruct* p_struct, Viewport<f32> viewport, vk::RenderPass renderpass );
void gen_flat_pipeline ( VInstance* v_instance, PipelineStruct* p_struct, Viewport<f32> viewport, vk::RenderPass renderpass );
void gen_skybox_pipeline ( VInstance* v_instance, PipelineStruct* p_struct, Viewport<f32> viewport, vk::RenderPass renderpass );
void gen_shot_pipeline ( VInstance* v_instance, PipelineStruct* p_struct, Viewport<f32> viewport, vk::RenderPass renderpass );
void gen_engine_pipeline ( VInstance* v_instance, PipelineStruct* p_struct, Viewport<f32> viewport, vk::RenderPass renderpass );

void gen_lightless_pipeline ( VInstance* v_instance, PipelineStruct* p_struct, Viewport<f32> viewport, vk::RenderPass renderpass );
void gen_dirlight_pipeline ( VInstance* v_instance, PipelineStruct* p_struct, Viewport<f32> viewport, vk::RenderPass renderpass );

void destroy_pipeline ( VInstance* v_instance, PipelineStruct* p_struct );
void destroy_pipeline_layout ( VInstance* v_instance, PipelineStruct* p_struct );


void render_pipeline ( VInstance* v_instance, VInstanceGroup* igroup, VContextGroup* cgroup, PipelineStruct* p_struct, SubPassInput* renderpass_struct, vk::CommandBuffer cmdbuffer );

struct VMainRenderStage : public VRenderStage {
	VInstance* v_instance;
	
	VInstanceGroup* v_igroup;
	VContextGroup* v_cgroup;

	PipelineStruct skybox_pipeline;
	PipelineStruct tex_pipeline;
	PipelineStruct flat_pipeline;
	PipelineStruct shot_pipeline;
	PipelineStruct engine_pipeline;
	
	PipelineStruct lightless_pipeline;
	PipelineStruct dirlight_pipeline;
	
	Viewport<f32> viewport;
	
	vk::CommandPool commandpool;
	Array<PerFrameMainBundleRenderObj> v_per_frame_data;
	vk::RenderPass v_renderpass;
	
	vk::DescriptorPool input_ds_pool;
	Array<SubPassInput> subpass_inputs;

	u64 last_frame_index_pipeline_built = 0;
	
	VMainRenderStage ( VInstance* v_instance, InstanceGroup* igroup, ContextGroup* cgroup );
	virtual ~VMainRenderStage();
	
	void v_destroy_pipeline_layouts();
	void v_destroy_pipelines();
	void v_destroy_renderpasses();
	void v_destroy_framebuffers();

	void v_check_rebuild();
	void v_rebuild_pipelines();
	void v_rebuild_commandbuffer ( u32 index );
	
	virtual void set_rendertarget ( u32 index, Image* image ) override;
	
	virtual void v_dispatch ( vk::CommandBuffer buffer, u32 index ) override;
};
struct VScaleDownRenderStage : public VRenderStage {
	VInstance* v_instance;
	
	VScaleDownRenderStage ( VInstance* v_instance );
	virtual ~VScaleDownRenderStage();
	
	virtual void set_rendertarget ( u32 index, Image* image ) override;
	
	virtual void v_dispatch ( vk::CommandBuffer buffer, u32 index ) override;
};
struct VMainBundle : public RenderBundle {
	Array<VRenderStage*> stages;
	DynArray<std::pair<u32, u32>> dependencies;
	VWindow* window_dependency;
	
	VInstance* v_instance;

	vk::CommandPool commandpool;
	Array<PerFrameMainBundleRenderObj> v_per_frame_data;
	
	u64 last_used = 0;

	VMainBundle ( VInstance* v_instance, InstanceGroup* igroup, ContextGroup* cgroup );
	virtual ~VMainBundle();

	virtual void add_dependency( u32 src_index, u32 dst_index ) override;
	virtual void remove_dependency( u32 src_index, u32 dst_index ) override;
	
	virtual void set_window_dependency( Window* window ) override;
	virtual void clear_window_dependency( ) override;

	virtual void set_renderstage ( u32 index, RenderStage* renderstage ) override;
	virtual RenderStage* get_renderstage ( u32 index ) override;
	virtual RenderStage* remove_renderstage ( u32 index ) override;

	void v_dispatch ( );
};

#endif // MAINBUNDLE_H
