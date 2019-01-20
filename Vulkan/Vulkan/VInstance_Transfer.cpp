#include "VInstance.h"
#include "VBuffer.h"
#include <limits>
#include <render/Timing.h>


VThinBuffer VInstance::request_staging_buffer ( u64 size ) {
	if ( !current_staging_buffer_store ) {
		if ( free_data.staging_buffer_stores.empty() ) {
			current_staging_buffer_store = new VDividableBufferStore ( this, vk::BufferUsageFlagBits::eTransferSrc,
			        vk::MemoryPropertyFlags() | vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent );
		} else {
			current_staging_buffer_store = free_data.staging_buffer_stores.back();
			free_data.staging_buffer_stores.pop_back();
		}
	}
	return current_staging_buffer_store->acquire_buffer ( size );
}
vk::Fence VInstance::request_fence () {
	if ( !free_data.fences.empty() ) {
		vk::Fence fence = free_data.fences.back();
		free_data.fences.pop_back();
		return fence;
	}
	return vk_device().createFence ( vk::FenceCreateInfo ( vk::FenceCreateFlags() ) );
}
void VInstance::free_fence ( vk::Fence fence ) {
	v_logger.log<LogLevel::eDebug> ( "Freeing Fence 0x%" PRIx64, fence );
	current_free_data->fences.push_back ( fence );
}
vk::CommandBuffer VInstance::request_transfer_command_buffer() {
	if ( !free_data.command_buffers.empty() ) {
		vk::CommandBuffer commandbuffer = free_data.command_buffers.back();
		free_data.command_buffers.pop_back();
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
	current_free_data->command_buffers.push_back ( buffer );
}

//high priority transfer
vk::Semaphore VInstance::schedule_transfer_data ( Array<VSimpleTransferJob>& jobs ) {
	assert ( false );
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
		free_data.fences.push_back ( fence );
	}
}

void VInstance::wait_for_frame ( u64 wait_index ) {
	if ( wait_index <= last_completed_frame_index ) return;
	v_logger.log<LogLevel::eInfo> ( "Waiting for Frame %" PRId64, wait_index );
	while ( !per_frame_queue.empty() ) {
		PerFrameData* data = per_frame_queue.front();
		if ( data->frame_index > wait_index ) {
			break;
		}
		if ( !data->fences.empty() ) {
			vk_device().waitForFences ( data->fences, true, std::numeric_limits<u64>::max() );
			vk_device().resetFences ( data->fences );
		}
		for ( VDividableBufferStore* buffer_store : data->staging_buffer_stores ) {
			buffer_store->free_buffers();
			free_data.staging_buffer_stores.push_back ( buffer_store );
		}
		data->staging_buffer_stores.clear();
		for ( vk::Fence fence : data->fences ) {
			free_data.fences.push_back ( fence );
		}
		data->fences.clear();
		for ( vk::CommandBuffer buffer : data->command_buffers ) {
			free_data.command_buffers.push_back ( buffer );
		}
		data->command_buffers.clear();
		per_frame_datas.push_back(data);
		per_frame_queue.pop();
	}
	last_completed_frame_index = wait_index;
}

void VInstance::prepare_frame () {
	//TODO "... < wait_index" -> "... <= wait_index"
	if (current_staging_buffer_store) {
		current_free_data->staging_buffer_stores.push_back ( current_staging_buffer_store );
		if ( free_data.staging_buffer_stores.empty() ) {
			current_staging_buffer_store = new VDividableBufferStore ( this, vk::BufferUsageFlagBits::eTransferSrc,
			        vk::MemoryPropertyFlags() | vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent );
		} else {
			current_staging_buffer_store = free_data.staging_buffer_stores.back();
			free_data.staging_buffer_stores.pop_back();
		}
	}
	if(current_free_data) {
		current_free_data->frame_index = this->frame_index;
		per_frame_queue.push ( current_free_data );
		current_free_data = nullptr;
	}
	if ( !current_free_data ) {
		if(per_frame_datas.empty()){
			current_free_data = new PerFrameData();
		} else {
			current_free_data = per_frame_datas.back();
			per_frame_datas.pop_back();
		}
		current_free_data->frame_index = frame_index;
	}
}