#include "VulkanWindowSection.h"
#include "VulkanQuadRenderer.h"
#include "VulkanWindow.h"


VulkanWindowSection::VulkanWindowSection ( WindowSectionType type, VulkanInstance* instance ) : WindowSection ( type ), v_instance ( instance ),
	m_viewport(), m_parent_window ( nullptr ), m_parent ( nullptr ), finish_sem ( create_semaphore ( instance ) ) {

}
VulkanWindowSection::~VulkanWindowSection() {
	destroy_semaphore ( v_instance, finish_sem );
}
UIVulkanWindowSection::~UIVulkanWindowSection() {
	v_quad_renderer.remove();
}
void UIVulkanWindowSection::render_frame ( u32 index ) {
	//printf ( "Render-UI Section\n" );

	vk::CommandBuffer buffers[2];
	buffers[0] = per_frame_data[index].clearcmd;
	buffers[1] = v_quad_renderer.render_quads ( index );

	vk::PipelineStageFlags waitDstStageMask = vk::PipelineStageFlags() | vk::PipelineStageFlagBits::eColorAttachmentOutput;
	vk::SubmitInfo submitInfo (
	    m_parent_window->active_sems.size(), m_parent_window->active_sems.size() ? m_parent_window->active_sems.data() : nullptr,//waitSemaphores
	    &waitDstStageMask,//pWaitDstStageMask
	    2, buffers,
	    1, &finish_sem//signalsemaphores
	);
	v_instance->vulkan_pgc_queue ( m_parent_window->queue_index )->graphics_queue.submit ( {submitInfo}, vk::Fence() );
	m_parent_window->active_sems.clear();
	m_parent_window->active_sems.push_back ( finish_sem );
}
void UIVulkanWindowSection::resize ( Viewport<f32> viewport, VulkanRenderTarget* target_wrapper ) {
	if ( target_wrapper == nullptr ) {
		reset();
	} else {
		if ( !depth_image ) {
			vk::Format depth_format = v_instance->findDepthFormat();
			vk::Extent3D extent ( viewport.extend.width, viewport.extend.height, 0 );
			depth_image = new VulkanImageWrapper ( v_instance, extent, 1, 1, depth_format, vk::ImageTiling::eOptimal, vk::ImageUsageFlags ( vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eTransferDst ),
			                                       vk::ImageAspectFlags() | vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil, vk::MemoryPropertyFlags ( vk::MemoryPropertyFlagBits::eDeviceLocal ) );
		}
		if ( !depth_image_view ) {
			depth_image_view = v_instance->createImageView2D ( depth_image->image, 0, depth_image->mipMapLevels, depth_image->format, vk::ImageAspectFlags ( vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil ) );
		}
		if ( !commandpool ) {
			vk::CommandPoolCreateInfo createInfo ( vk::CommandPoolCreateFlags(), v_instance->vulkan_pgc_queue ( 0 )->graphics_queue_id );
			vulkan_device ( v_instance ).createCommandPool ( &createInfo, nullptr, &commandpool );
		} else {
			vulkan_device ( v_instance ).resetCommandPool ( commandpool, vk::CommandPoolResetFlags() );
		}
		for ( UIVulkanSectionFrameData& data : per_frame_data ) {
			//reset
		}
		per_frame_data.resize ( target_wrapper->images.size() );
		vk::CommandBuffer command_buffers[per_frame_data.size()];

		vk::CommandBufferAllocateInfo allocinfo ( commandpool, vk::CommandBufferLevel::ePrimary, per_frame_data.size() );
		vulkan_device ( v_instance ).allocateCommandBuffers ( &allocinfo, command_buffers );
		int i = 0;
		for ( UIVulkanSectionFrameData& data : per_frame_data ) {
			data.clearcmd = command_buffers[i++];

			data.clearcmd.begin ( vk::CommandBufferBeginInfo() );
			depth_image->transition_image_layout ( vk::ImageLayout::eTransferDstOptimal, data.clearcmd );
			data.clearcmd.clearDepthStencilImage (
			    depth_image->image,
			    vk::ImageLayout::eTransferDstOptimal,
			    vk::ClearDepthStencilValue ( 1.0f, 0 ),
			{vk::ImageSubresourceRange ( depth_image->aspectFlags, 0, 1, 0, 1 ) }
			);
			depth_image->transition_image_layout ( vk::ImageLayout::eDepthStencilAttachmentOptimal, data.clearcmd );
			data.clearcmd.end();
		}

		target_wrapper->depth_stencil_format = depth_image->format;
		target_wrapper->depthview = depth_image_view;
		v_quad_renderer.update_extend ( viewport, target_wrapper );
	}

}
void UIVulkanWindowSection::reset ( ) {
	VulkanQuadRenderer quad_rend = v_quad_renderer;
	vk::CommandPool tmp_commandpool = commandpool;
	vk::ImageView tmp_depth_image_view = depth_image_view;
	VulkanImageWrapper* tmp_depth_image = depth_image;
	VulkanInstance* tmp_instance = v_instance;

	commandpool = vk::CommandPool();
	depth_image_view = vk::ImageView();
	depth_image = nullptr;
	v_quad_renderer.inherit ( &quad_rend );

	FrameLocalData* frame_local_data = m_parent_window->current_framelocal_data();
	auto lambda = [tmp_instance, quad_rend, tmp_commandpool, tmp_depth_image_view, tmp_depth_image] ( u32 index ) mutable -> RendResult {
		if ( tmp_commandpool ) {
			vulkan_device ( tmp_instance ).destroyCommandPool ( tmp_commandpool );
		}
		if ( tmp_depth_image_view ) {
			vulkan_device ( tmp_instance ).destroyImageView ( tmp_depth_image_view );
		}
		if ( tmp_depth_image ) {
			tmp_depth_image->destroy();
			delete tmp_depth_image;
		}
		quad_rend.remove();
		return RendResult::eSuccess;
	};
	if ( frame_local_data ) {
		m_parent_window->current_framelocal_data()->deferred_calls.push_back ( lambda );
	} else {
		lambda ( 0 );
	}

}
