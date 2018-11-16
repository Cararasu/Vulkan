#include "VInstance.h"
#include "VBuffer.h"
#include <limits>


VBuffer* VInstance::request_staging_buffer ( u64 size ) {
	//TODO cache staging buffers
	return new VBuffer (
	           this,
	           size,
	           vk::BufferUsageFlagBits::eTransferSrc,
	           vk::MemoryPropertyFlags() | vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent );
}
void VInstance::free_staging_buffer ( VBuffer* buffer ) {
	//TODO put them into a staging store and clean it at the end of the frame
	vassert ( buffer );
	delete buffer;
}

//high priority transfer
vk::Semaphore VInstance::schedule_transfer_data ( Array<VSimpleTransferJob>& jobs ) {
	for ( VSimpleTransferJob& transfer_job : jobs ) {

	}
	return vk::Semaphore();
}
//a fence for optional cpu-synchronization and cpu-side check for completion
//maybe do it in a queue that has lower priority
vk::Fence VInstance::do_transfer_data_asynch ( Array<VSimpleTransferJob>& jobs ) {

	return vk::Fence();
}

//immediate transfer
void VInstance::transfer_data ( Array<VSimpleTransferJob>& jobs ) {
	vk::Fence fence = do_transfer_data_asynch ( jobs );
	if ( fence ) vk_device().waitForFences ( {fence}, true, std::numeric_limits<u64>::max() );
}

void VInstance::wait_for_frame ( u64 frame_index ) {
	if(frame_index) {
		v_logger.log<LogLevel::eWarn> ( "Waiting for Frame %d", frame_index );
		last_completed_frame_index = frame_index;
	}
}
