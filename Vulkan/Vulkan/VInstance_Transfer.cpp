#include "VInstance.h"
#include "VBuffer.h"
#include <limits>


VThinBuffer VInstance::request_staging_buffer ( u64 size ) {
	if ( !current_staging_buffer_store ) {
		if ( ready_staging_buffer_queue.size() == 0 ) {
			current_staging_buffer_store = new VDividableBufferStore ( this, vk::BufferUsageFlagBits::eTransferSrc,
			        vk::MemoryPropertyFlags() | vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent );
		} else {
			current_staging_buffer_store = ready_staging_buffer_queue.front();
			ready_staging_buffer_queue.pop();
		}
	}
	return current_staging_buffer_store->acquire_buffer ( size );
}
vk::Fence VInstance::request_fence () {
	if ( free_fences.size() != 0 ) {
		vk::Fence fence = free_fences.back();
		free_fences.pop_back();
		return fence;
	}
	return vk_device().createFence ( vk::FenceCreateInfo ( vk::FenceCreateFlags() ) );
}
void VInstance::free_fence ( vk::Fence fence ) {
	v_logger.log<LogLevel::eDebug> ( "Freeing Fence 0x%" PRIx64, fence );
	free_fence_queue.push ( std::make_pair ( frame_index, fence ) );
}
vk::CommandBuffer VInstance::request_transfer_command_buffer() {
	if ( free_command_buffers.size() != 0 ) {
		vk::CommandBuffer commandbuffer = free_command_buffers.back();
		free_command_buffers.pop_back();
		commandbuffer.reset ( vk::CommandBufferResetFlags() );
		return commandbuffer;
	}
	vk::CommandBufferAllocateInfo allocateInfo ( transfer_commandpool, vk::CommandBufferLevel::ePrimary, 1 );

	vk::CommandBuffer commandBuffer;
	m_device.allocateCommandBuffers ( &allocateInfo, &commandBuffer );
	return commandBuffer;
}
void VInstance::free_transfer_command_buffer ( vk::CommandBuffer buffer ) {
	v_logger.log<LogLevel::eDebug> ( "Freeing CommandBuffer 0x%" PRIx64, buffer );
	free_command_buffer_queue.push ( std::make_pair ( frame_index, buffer ) );
}

//high priority transfer
vk::Semaphore VInstance::schedule_transfer_data ( Array<VSimpleTransferJob>& jobs ) {
	vk::CommandBuffer cmdbuffer = request_transfer_command_buffer ( );
	//vk::CommandBuffer t_buffer = request_transfer_command_buffer();
	for ( VSimpleTransferJob& transfer_job : jobs ) {

	}

	free_transfer_command_buffer ( cmdbuffer );
	return vk::Semaphore();
}
//a fence for optional cpu-synchronization and cpu-side check for completion
//maybe do it in a queue that has lower priority
vk::Fence VInstance::do_transfer_data_asynch ( Array<VSimpleTransferJob>& jobs ) {
	vk::CommandBuffer cmdbuffer = request_transfer_command_buffer ( );

	cmdbuffer.begin ( vk::CommandBufferBeginInfo ( vk::CommandBufferUsageFlags() | vk::CommandBufferUsageFlagBits::eOneTimeSubmit ) );

	for ( VSimpleTransferJob& transfer_job : jobs ) {
		cmdbuffer.copyBuffer ( transfer_job.source_buffer.buffer, transfer_job.target_buffer->buffer, 1, &transfer_job.sections );
	}
	cmdbuffer.end();
	QueueWrapper* queue_wrapper_ptr = queue_wrapper();

	vk::PipelineStageFlags waitDstStageMask ( vk::PipelineStageFlagBits::eHost );
	vk::SubmitInfo submitinfo (
	    0, nullptr, //waitSem
	    &waitDstStageMask,
	    1, &cmdbuffer,//commandbuffers
	    0, nullptr//signalSem
	);
	vk::Fence fence = request_fence();
	if ( queue_wrapper_ptr->dedicated_transfer_queue ) {
		queue_wrapper_ptr->transfer_queue.submit ( 1, &submitinfo, fence );
	} else {
		queue_wrapper_ptr->graphics_queue.submit ( 1, &submitinfo, fence );
	}
	return fence;
}

//immediate transfer
void VInstance::transfer_data ( Array<VSimpleTransferJob>& jobs ) {
	vk::Fence fence = do_transfer_data_asynch ( jobs );
	if ( fence ) {
		vk_device().waitForFences ( {fence}, true, std::numeric_limits<u64>::max() );
		vk_device().resetFences ( {fence} );
		free_fences.push_back ( fence );
	}
}

void VInstance::wait_for_frame ( u64 frame_index ) {
	v_logger.log<LogLevel::eInfo> ( "Waiting for Frame %" PRId64, frame_index );
	//TODO "... < frame_index" -> "... <= frame_index"
	free_staging_buffer_queue.push(std::make_pair(this->frame_index, current_staging_buffer_store));
	current_staging_buffer_store = nullptr;
	while ( !free_staging_buffer_queue.empty() && free_staging_buffer_queue.front().first < frame_index ) {
		free_staging_buffer_queue.front().second->free_buffers();
		ready_staging_buffer_queue.push(free_staging_buffer_queue.front().second);
		free_staging_buffer_queue.pop();
	}
	while ( !free_fence_queue.empty() && free_fence_queue.front().first < frame_index ) {
		free_fences.push_back ( free_fence_queue.front().second );
		free_fence_queue.pop();
	}
	while ( !free_command_buffer_queue.empty() && free_command_buffer_queue.front().first < frame_index ) {
		free_command_buffers.push_back ( free_command_buffer_queue.front().second );
		free_command_buffer_queue.pop();
	}
	if ( frame_index ) {
		v_logger.log<LogLevel::eWarn> ( "Waiting for Frame %d", frame_index );
		last_completed_frame_index = frame_index;
	}
}
