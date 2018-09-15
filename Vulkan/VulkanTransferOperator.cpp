#include "VulkanTransferOperator.h"
#include "VulkanInstance.h"

VulkanBufferStorage::VulkanBufferStorage ( VulkanInstance* instance, vk::BufferUsageFlags usageflags ) :
	buffer ( instance ),
	sections(),
	mapped_transfer_buffer ( nullptr ),
	needed_size ( 0 ) {

}
RId VulkanBufferStorage::allocate_chunk ( u32 size ) {
	VulkanBufferSection section = {0, 0, size};
	return sections.insert ( section );
}
void VulkanBufferStorage::free_chunk ( RId index ) {
	sections.remove ( index );
}
void VulkanBufferStorage::reorganize ( ) {
	if ( buffer.size > needed_size ) {
		buffer.destroy();
		while ( buffer.size < needed_size ) buffer.size *= 2;
		buffer.init();
	}
	u32 offset = 0;
	for ( VulkanBufferSection& section : sections ) {
		if ( section.id = 0 ) continue;
		section.offset = offset;
		offset += section.size;
	}
	needed_size = offset;
	if ( needed_size >= 1024 && needed_size < buffer.size / 4 ) {
		buffer.destroy();
		while ( needed_size >= 1024 && buffer.size / 4 > needed_size ) buffer.size /= 2;
		buffer.init();
	}
	std::pair<void*, vk::Semaphore> data = buffer.v_instance->transfer_control->request_transfer_memory(&buffer, buffer.size, 0);
	mapped_transfer_buffer = data.first;
	semaphore = data.second;
}
u32 VulkanBufferStorage::get_offset ( RId index ) {
	return sections[index].offset;
}
void* VulkanBufferStorage::get_ptr ( RId index ) {
	return mapped_transfer_buffer + sections[index].offset;
}
void VulkanBufferStorage::update_data ( ) {

}

void VulkanPerFrameTransferData::init ( VulkanInstance* v_instance, vk::CommandPool commandpool ) {
	vk::CommandBufferAllocateInfo allocateInfo ( commandpool, vk::CommandBufferLevel::ePrimary, 1 );
	vulkan_device ( v_instance ).allocateCommandBuffers ( &allocateInfo, &commandbuffer );

	vk::FenceCreateInfo create_info;
	vulkan_device ( v_instance ).createFence ( &create_info, nullptr, &fence );
}
void VulkanPerFrameTransferData::destroy ( VulkanInstance* v_instance ) {
	vulkan_device ( v_instance ).waitForFences ( 1, &fence, true, std::numeric_limits<u64>::max() );
	vulkan_device ( v_instance ).destroyFence ( fence );
	for ( VulkanBuffer* buffer : used_buffers ) {
		delete buffer;
	}
}

VulkanTransferBuffer::VulkanTransferBuffer ( VulkanInstance* instance, vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags needed, vk::MemoryPropertyFlags recommended ) :
	buffer ( instance, size, usage, needed, recommended ) {
	vk::SemaphoreCreateInfo cinfo;
	vulkan_device ( instance ).createSemaphore ( &cinfo, nullptr, &semaphore );
}
VulkanTransferBuffer::~VulkanTransferBuffer () {
	vulkan_device ( buffer.v_instance ).destroySemaphore ( semaphore );
}
VulkanTransferController::VulkanTransferController ( VulkanInstance* instance ) : v_instance ( instance ), transfer_buffers(), commandpool(), synchron_commandbuffer() {
	vk::CommandPoolCreateInfo createInfo ( vk::CommandPoolCreateFlagBits::eResetCommandBuffer );
	vulkan_device ( v_instance ).createCommandPool ( &createInfo, nullptr, &commandpool );
}
VulkanTransferController::~VulkanTransferController() {
	for ( VulkanPerFrameTransferData* data : perframe_data ) {
		data->destroy ( v_instance );
		delete data;
	}
	perframe_data.clear();
	for ( VulkanPerFrameTransferData* data : available_perframe_data ) {
		data->destroy ( v_instance );
		delete data;
	}
	available_perframe_data.clear();
	for ( VulkanTransferJob data : jobs ) {
		delete data.used_transfer_buffer;
	}
	jobs.clear();
	if ( synchron_commandbuffer ) {
		vulkan_device ( v_instance ).freeCommandBuffers ( commandpool, 1, &synchron_commandbuffer );
	}
	if ( commandpool ) {
		vulkan_device ( v_instance ).destroyCommandPool ( commandpool );
	}
}
std::pair<void*, vk::Semaphore> VulkanTransferController::request_transfer_memory ( VulkanBuffer* target_buffer, u32 size, u32 offset ) {
	VulkanTransferBuffer* transfer_buffer = nullptr;
	for ( auto it = transfer_buffers.begin(); it != transfer_buffers.end(); it++ ) {
		VulkanTransferBuffer* buff = *it;
		if ( buff->buffer.size == size ) {
			transfer_buffer = buff;
			transfer_buffers.erase ( it );
			break;
		}
	}
	if ( !transfer_buffer ) {
		transfer_buffer = new VulkanTransferBuffer ( v_instance, size, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible );
		transfer_buffer->buffer.map_mem();
	}
	jobs.push_back ( {transfer_buffer, target_buffer, offset, size} );
	return std::make_pair ( ( void* ) transfer_buffer->buffer.mapped_ptr, transfer_buffer->semaphore );
}
void VulkanTransferController::do_transfers ( ) {
	VulkanPerFrameTransferData* data;
	for ( VulkanTransferBuffer* buffer : transfer_buffers ) {
		delete buffer;
	}
	if ( perframe_data.empty() ) {
		data = new VulkanPerFrameTransferData ();
		data->init ( v_instance, commandpool );
	} else {
		data = perframe_data.back();
		perframe_data.pop_back();
	}
	data->commandbuffer.reset ( vk::CommandBufferResetFlags() );
	data->commandbuffer.begin ( vk::CommandBufferBeginInfo ( vk::CommandBufferUsageFlagBits::eOneTimeSubmit ) );
	for ( VulkanTransferJob& job : jobs ) {
		job.used_transfer_buffer->buffer.transfer_to ( job.dst_buffer, job.final_offset, job.byte_size, data->commandbuffer );
		transfer_buffers.push_back ( job.used_transfer_buffer );
	}
	data->commandbuffer.end();
	vk::PipelineStageFlags waitflags = vk::PipelineStageFlagBits::eHost;
	vk::SubmitInfo submitInfo ( 0, nullptr, &waitflags,
	                            1, &data->commandbuffer,
	                            0, nullptr );
	//should be transfer queue
	v_instance->queues.pgc[0].graphics_queue.submit ( 1, &submitInfo, vk::Fence() );
	v_instance->queues.pgc[0].graphics_queue.waitIdle();
}
void VulkanTransferController::check_free ( ) {
	auto it = perframe_data.begin();
	for ( ; it != perframe_data.end(); it++ ) {
		VulkanPerFrameTransferData* data = *it;
		if ( vulkan_device ( v_instance ).getFenceStatus ( data->fence ) == vk::Result::eSuccess ) {
			vulkan_device ( v_instance ).resetFences ( 1, &data->fence );
			available_perframe_data.push_back ( data );
			it = perframe_data.erase ( it );
		} else {
			break;
		}
	}
	if(it != perframe_data.end())
		perframe_data.erase ( it );
}
