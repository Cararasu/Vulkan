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

/*struct PerRenderInstanceData {
	vk::Framebuffer framebuffer;
	ResettableCommandBuffer command;
};
struct VPipelineLayoutObject {
	InstanceBaseId mib_id;
	Array<ContextBaseId> contextbase_list;

	vk::PipelineLayout v_pipeline_layout;

	Array<vk::DescriptorSetLayout> v_descriptor_set_layouts;

	VPipelineObject ( InstanceBaseId mib_id, Array<ContextBaseId>& contextbase_list );
	~VPipelineObject ( );

	void destroy_pipeline_layouts();
	void destroy_pipelines();
	void destroy_commandbuffers();
	void destroy_all();

	void rebuild ( vk::RenderPass renderpass, Array<vk::Framebuffer>& framebuffers, Viewport<f32> viewport );

	vk::CommandBuffer get_command_buffer ( u32 index );
}
struct VPipelineObject {
	InstanceBaseId mib_id;
	Array<ContextBaseId> contextbase_list;

	vk::PipelineLayout v_pipeline_layout;
	vk::Pipeline v_pipeline;
	vk::RenderPass current_renderpass;
	Viewport<f32> current_viewport;
	
	vk::CommandPool commandpool;
	Array<PerFrameMainBundleRenderObj> per_instance_data;

	Array<vk::DescriptorSetLayout> v_descriptor_set_layouts;

	VPipelineObject ( InstanceBaseId mib_id, Array<ContextBaseId>& contextbase_list );
	~VPipelineObject ( );

	void destroy_pipeline_layouts();
	void destroy_pipelines();
	void destroy_commandbuffers();
	void destroy_all();

	void rebuild ( vk::RenderPass renderpass, Array<vk::Framebuffer>& framebuffers, Viewport<f32> viewport );

	vk::CommandBuffer get_command_buffer ( u32 index );
}*/

struct VBundleImageState {
	vk::Format current_format = vk::Format::eUndefined;
	VBaseImage* actual_image = nullptr;
};
struct PerFrameMainBundleRenderObj {
	vk::Framebuffer framebuffer;
	ResettableCommandBuffer command;
};

struct VMainBundle : public RenderBundle {
	VInstance* v_instance;
	VInstanceGroup* v_igroup;
	VContextGroup* v_cgroup;
	Array<VBundleImageState> v_bundleStates;

	Viewport<f32> viewport;

	vk::PipelineLayout v_object_pipeline_layout;
	vk::Pipeline v_object_pipeline;

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
	void v_rebuild_commandbuffers(u32 index);
	void v_dispatch();
};

#endif // MAINBUNDLE_H
