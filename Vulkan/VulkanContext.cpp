#include "VulkanContext.h"
#include "VulkanBuffer.h"
#include "VulkanInstance.h"
#include "VulkanTransferOperator.h"


VulkanGPUDataStore::VulkanGPUDataStore ( VulkanInstance* instance, const DataGroupDef* datagroup, vk::BufferUsageFlags usage ) :
	instance ( instance ),
	datagroup ( datagroup->id ),
	buffer ( instance ),
	datastore ( datagroup->size ) {
	buffer.init ( datastore.size * 20,
	              usage | vk::BufferUsageFlagBits::eTransferSrc,
	              vk::MemoryPropertyFlags() | vk::MemoryPropertyFlagBits::eDeviceLocal
	            );
}
VulkanGPUDataStore::~VulkanGPUDataStore() {

}
RId VulkanGPUDataStore::allocate_block() {
	return datastore.create_chunk();
}
void VulkanGPUDataStore::remove_block ( RId index ) {
	return datastore.delete_chunk ( index );
}
void VulkanGPUDataStore::update_data() {
	std::pair<void*, vk::Semaphore> data = instance->transfer_control->request_transfer_memory ( &buffer, datastore.size, 0 );
	memcpy(datastore.data, data.first, datastore.size);
}
