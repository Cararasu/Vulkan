#include "VInstance.h"
#include "VBuffer.h"



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
	vassert(buffer)
	delete buffer;
}

//immediate transfer
void VInstance::transfer_data ( Array<VSimpleTransferJob>& jobs ) {

}
//high priority transfer
vk::Semaphore VInstance::schedule_transfer_data ( Array<VSimpleTransferJob>& jobs ) {
	for(VSimpleTransferJob& transfer_job : jobs){
		
	}
}
void VInstance::transfer_data() {

}
//a fence for optional cpu-synchronization and cpu-side check for completion
//maybe do it in a queue that has lower priority
vk::Fence VInstance::do_transfer_data_asynch ( Array<VSimpleTransferJob>& jobs ) {

}
