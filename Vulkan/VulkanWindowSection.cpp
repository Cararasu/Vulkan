#include "VulkanWindowSection.h"
#include "VulkanQuadRenderer.h"


VulkanWindowSection::VulkanWindowSection ( WindowSectionType type, VulkanInstance* instance ) : WindowSection ( type ), v_instance ( instance ),
	m_viewport(), m_parent_window ( nullptr ), m_parent ( nullptr ), v_quad_renderer ( new VulkanQuadRenderer(instance) ) {}
VulkanWindowSection::~VulkanWindowSection() {
	delete v_quad_renderer;
}
void UIVulkanWindowSection::render_frame() {
	//printf ( "Render-UI Section\n" );
}
void UIVulkanWindowSection::resize ( Viewport<f32> viewport, VulkanRenderTarget* target_wrapper ) {

	v_quad_renderer->update_extend ( viewport, target_wrapper );

}
