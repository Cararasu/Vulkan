#include "VulkanWindowSection.h"
#include "VulkanQuadRenderer.h"
#include "VulkanWindow.h"


VulkanWindowSection::VulkanWindowSection ( WindowSectionType type, VulkanInstance* instance ) : WindowSection ( type ), v_instance ( instance ),
	m_viewport(), m_parent_window ( nullptr ), m_parent ( nullptr ), v_quad_renderer ( nullptr ), finish_sem ( create_semaphore ( instance ) ) {

}
VulkanWindowSection::~VulkanWindowSection() {
	delete v_quad_renderer;
	destroy_semaphore ( v_instance, finish_sem );
}
void UIVulkanWindowSection::render_frame ( u32 index ) {
	//printf ( "Render-UI Section\n" );
	vk::CommandBuffer quadbuffer = v_quad_renderer->render_quads ( index );

	vk::PipelineStageFlags waitDstStageMask = vk::PipelineStageFlags() | vk::PipelineStageFlagBits::eColorAttachmentOutput;
	vk::SubmitInfo submitInfo (
	    m_parent_window->active_sems.size(), m_parent_window->active_sems.size() ? m_parent_window->active_sems.data() : nullptr,//waitSemaphores
	    &waitDstStageMask,//pWaitDstStageMask
	    1, &quadbuffer,
	    1, &finish_sem//signalsemaphores
	);
	v_instance->vulkan_pgc_queue ( m_parent_window->queue_index )->graphics_queue.submit ( {submitInfo}, vk::Fence() );
	m_parent_window->active_sems.clear();
	m_parent_window->active_sems.push_back ( finish_sem );
}
void UIVulkanWindowSection::resize ( Viewport<f32> viewport, VulkanRenderTarget* target_wrapper ) {
	if(target_wrapper == nullptr){
		reset();
	}else{
		if(!v_quad_renderer)
			v_quad_renderer = new VulkanQuadRenderer(v_instance);
		v_quad_renderer->update_extend ( viewport, target_wrapper );
	}

}
void UIVulkanWindowSection::reset ( ) {
	if(v_quad_renderer){
		VulkanQuadRenderer* quadRend = v_quad_renderer;
		m_parent_window->current_framelocal_data()->deferred_calls.push_back ( [quadRend] ( u32 index ) {
			delete quadRend;
			return RendResult::eSuccess;
		});
		v_quad_renderer = new VulkanQuadRenderer(v_quad_renderer);
	}
}
