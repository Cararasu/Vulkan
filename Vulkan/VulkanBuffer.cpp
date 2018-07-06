#include "VulkanBuffer.h"


VulkanBuffer::VulkanBuffer ( VulkanInstance* instance, vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags needed, vk::MemoryPropertyFlags recommended ) :
	v_instance ( instance ), memory ( {
	vk::DeviceMemory(), size
} ), buffer ( vk::Buffer() ) {
	vk::Device device = vulkan_device ( v_instance );
	vk::BufferCreateInfo bufferInfo ( vk::BufferCreateFlags(), size, usage, vk::SharingMode::eExclusive );

	V_CHECKCALL ( device.createBuffer ( &bufferInfo, nullptr, &buffer ), printf ( "Failed To Create Buffer\n" ) );

	printf ( "Create Buffer of size %d\n", size );

	vk::MemoryRequirements mem_req;
	device.getBufferMemoryRequirements ( buffer, &mem_req );
	memory = v_instance->allocate_gpu_memory ( mem_req, needed, recommended );
	device.bindBufferMemory ( buffer, memory.memory, 0 );
}
RendResult VulkanBuffer::map_mem() {
	if ( !memory.memory ) {
		return RendResult::eUninitialized;
	}
	if ( ! ( memory.property_flags & vk::MemoryPropertyFlagBits::eHostVisible ) ) {
		return RendResult::eUninitialized;
	}
	if ( mapped_ptr )
		return RendResult::eSuccess;
	vulkan_device(v_instance).mapMemory(memory.memory, (vk::DeviceSize) 0L, memory.size, vk::MemoryMapFlags(), &mapped_ptr);
}
RendResult VulkanBuffer::unmap_mem() {
	vulkan_device(v_instance).unmapMemory(memory.memory);
}
RendResult VulkanBuffer::transfer_to(VulkanBuffer* dst, vk::DeviceSize offset, vk::DeviceSize size, vk::CommandBuffer commandBuffer){
	
}
void VulkanBuffer::destroy() {
	if(mapped_ptr)
		unmap_mem();
	vulkan_device ( v_instance ).destroyBuffer ( buffer, nullptr );
	v_instance->free_gpu_memory ( memory );
}
VulkanBuffer::~VulkanBuffer() {
	destroy();
}
