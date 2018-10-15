#include "VContext.h"
#include "VBuffer.h"
#include "VInstance.h"
#include "VTransferOperator.h"


VGPUDataStore::VGPUDataStore ( VInstance* instance, const DataGroupDef* datagroup, vk::BufferUsageFlags usage ) :
	instance ( instance ),
	datagroup ( datagroup->id ),
	buffer ( instance ),
	datastore ( datagroup->size ) {
	buffer.init ( datastore.size * 20,
	              usage | vk::BufferUsageFlagBits::eTransferSrc,
	              vk::MemoryPropertyFlags() | vk::MemoryPropertyFlagBits::eDeviceLocal
	            );
}
VGPUDataStore::~VGPUDataStore() {

}
RId VGPUDataStore::allocate_block() {
	return datastore.create_chunk();
}
void VGPUDataStore::remove_block ( RId index ) {
	return datastore.delete_chunk ( index );
}
void VGPUDataStore::update_data() {
	std::pair<void*, vk::Semaphore> data = instance->transfer_control->request_transfer_memory ( &buffer, datastore.size, 0 );
	memcpy(datastore.data, data.first, datastore.size);
}
