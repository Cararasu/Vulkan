#include "VulkanBuffer.h"


VulkanBuffer::VulkanBuffer ( VulkanInstance* instance, vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags needed, vk::MemoryPropertyFlags recommended ) :
	v_instance ( instance ), memory ( {
	vk::DeviceMemory(), size
} ), buffer ( vk::Buffer() ) {
	vk::Device device = vulkan_device ( v_instance );
	vk::BufferCreateInfo bufferInfo ( vk::BufferCreateFlags(), size, usage, vk::SharingMode::eExclusive );

	V_CHECKCALL ( device.createBuffer ( &bufferInfo, nullptr, &buffer ), printf ( "Failed To Create Buffer\n" ) );

	printf ( "Create Buffer of size %d\n", size );

	vk::MemoryRequirements memRequirements;
	device.getBufferMemoryRequirements ( buffer, &memRequirements );
	memory.memory = v_instance->allocateMemory ( memRequirements, needed | recommended );
	memory.size = memRequirements.size;
	if ( !memory.memory )
		memory.memory = v_instance->allocateMemory ( memRequirements, needed );

	device.bindBufferMemory ( buffer, memory.memory, 0 );
}
void VulkanBuffer::destroy() {
	//instance->destroyBuffer (buffer, memory.memory);
	vulkan_device ( v_instance ).destroyBuffer ( buffer, nullptr );
	vulkan_device ( v_instance ).freeMemory ( memory.memory, nullptr );
}
VulkanBuffer::~VulkanBuffer() {
	destroy();
}
