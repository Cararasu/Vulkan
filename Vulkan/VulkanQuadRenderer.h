#pragma once
#include "VulkanInstance.h"
#include "VulkanBuffer.h"
#include "render/Dimensions.h"

struct PerFrameQuadRenderObj {
	vk::Framebuffer framebuffer;
	vk::CommandBuffer commandbuffer;
};

struct QuadVertex {
	glm::vec2 pos;
};
struct QuadInstance {
	glm::vec4 dim;
	glm::vec4 uvdim;
	glm::vec4 data;
	glm::vec4 color;
};
enum class QuadType{
	eInactive,
	eFilled,
	eTextured
};
struct QuadRef{
	u32 quadid;
};
struct Quads{
	QuadType type;
};

struct VulkanRenderTarget;
struct SubmitStore;

struct VulkanQuadRenderer {
	VulkanInstance* const v_instance;
	Viewport<f32> viewport;

	std::array<vk::DescriptorSetLayout, 0> descriptor_set_layouts;
	vk::PipelineLayout pipeline_layout;
	vk::ShaderModule vertex_shader;
	vk::ShaderModule fragment_shader;
	vk::RenderPass renderpass;
	vk::Pipeline pipeline;

	vk::CommandPool g_commandpool;
	vk::CommandPool t_commandpool;

	VulkanBuffer* vertex_buffer = nullptr;
	VulkanBuffer* staging_vertex_buffer = nullptr;
	
	VulkanRenderTarget* render_target;

	Array<PerFrameQuadRenderObj> per_target_data;

	VulkanQuadRenderer ( VulkanInstance* instance );
	VulkanQuadRenderer ( VulkanQuadRenderer* old_quad_renderer );
	~VulkanQuadRenderer();

	void init();
	void inherit ( VulkanQuadRenderer* oldquadrenderer );
	void destroy_framebuffers();

	RendResult update_extend ( Viewport<f32> viewport, VulkanRenderTarget* target_wrapper );
	RendResult render (u32 frame_index, SubmitStore* state, u32 wait_sem_index, u32* final_sem_index);
	void destroy ( );
};

template<typename T>
void shrink_array(Array<T>* array_to_shrink){
	auto forward_it = array_to_shrink->begin();
	auto backwards_it = array_to_shrink->rbegin();
	while(forward_it <= backwards_it){
		for(; forward_it != array_to_shrink->end() && !forward_it->is_active(); forward_it++);
		for(; backwards_it != array_to_shrink->rend() && backwards_it->is_active(); backwards_it++);
		if(forward_it <= backwards_it){
			std::swap(*forward_it, *backwards_it);
		}
	}
	
}