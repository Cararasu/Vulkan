#pragma once
#include "VulkanBuffer.h"
#include "VulkanRenderer.h"


//full image
//one quad
//two quads with transition

struct PerFrameQuadRenderObj {
	vk::Framebuffer framebuffer;
	ResettableCommandBuffer command;
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

struct VulkanQuadRenderer : public VulkanRenderer {
	Viewport<f32> viewport;

	vk::ShaderModule vertex_shader;
	vk::ShaderModule fragment_shader;

	vk::CommandPool g_commandpool;
	vk::CommandPool t_commandpool;

	VulkanBuffer* vertex_buffer = nullptr;
	VulkanBuffer* staging_vertex_buffer = nullptr;
	
	VulkanRenderTarget* render_target;

	DynArray<PerFrameQuadRenderObj> per_target_data;

	VulkanQuadRenderer ( VulkanInstance* instance );
	~VulkanQuadRenderer();

	void init();
	void v_inherit ( VulkanQuadRenderer* oldquadrenderer );
	void destroy_framebuffers();
	
	void destroy ( );
	
	virtual void inherit ( VulkanRenderer* oldrenderer );
	
	virtual void use_commandpool ( vk::CommandPool commandpool ){}

	virtual RendResult update_extend ( Viewport<f32> viewport, VulkanRenderTarget* target_wrapper );

	virtual RendResult render ( u32 frame_index, SubmitStore* state, u32 wait_sem_index, u32* final_sem_index );
	
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