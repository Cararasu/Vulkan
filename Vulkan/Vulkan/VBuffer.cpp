#include "VBuffer.h"
#include "VInstance.h"

VBuffer::VBuffer ( VInstance* instance ) :
	v_instance ( instance ), size(0) {
}
VBuffer::VBuffer ( VInstance* instance, vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags needed, vk::MemoryPropertyFlags recommended ) :
	v_instance ( instance ) {
	init ( size, usage, needed, recommended );
}
RendResult VBuffer::init ( vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags needed, vk::MemoryPropertyFlags recommended ) {
	this->memory = { vk::DeviceMemory(), size };
	this->buffer = vk::Buffer(),
	this->size = size;
	this->usage = usage;
	this->needed = needed;
	this->recommended = recommended;
	return init();
}
RendResult VBuffer::init() {
	vk::Device device = vulkan_device ( v_instance );
	vk::BufferCreateInfo bufferInfo ( vk::BufferCreateFlags(), size, usage, vk::SharingMode::eExclusive );

	V_CHECKCALL ( device.createBuffer ( &bufferInfo, nullptr, &buffer ), printf ( "Failed To Create Buffer\n" ) );

	printf ( "Create Buffer of size %d with usage %s\n", size, to_string(usage).c_str() );

	vk::MemoryRequirements mem_req;
	device.getBufferMemoryRequirements ( buffer, &mem_req );
	memory = v_instance->allocate_gpu_memory ( mem_req, needed, recommended );
	device.bindBufferMemory ( buffer, memory.memory, 0 );
	if(memory.memory)
		return RendResult::eFail;
	return RendResult::eSuccess;
}
RendResult VBuffer::map_mem() {
	if ( !memory.memory ) {
		return RendResult::eUninitialized;
	}
	if ( ! ( memory.property_flags & vk::MemoryPropertyFlagBits::eHostVisible ) ) {
		return RendResult::eUninitialized;
	}
	if ( mapped_ptr )
		return RendResult::eSuccess;
	vulkan_device ( v_instance ).mapMemory ( memory.memory, ( vk::DeviceSize ) 0L, memory.size, vk::MemoryMapFlags(), &mapped_ptr );
	return mapped_ptr ? RendResult::eSuccess : RendResult::eFail;
}
RendResult VBuffer::unmap_mem() {
	vulkan_device ( v_instance ).unmapMemory ( memory.memory );
	mapped_ptr = nullptr;
}
RendResult VBuffer::transfer_to ( VBuffer* dst, vk::DeviceSize offset, vk::DeviceSize size, vk::CommandBuffer commandBuffer ) {
	commandBuffer.copyBuffer ( buffer, dst->buffer, {vk::BufferCopy ( offset, offset, size ) } );
}
void VBuffer::destroy() {
	if ( mapped_ptr )
		unmap_mem();
	if ( buffer ) {
		vulkan_device ( v_instance ).destroyBuffer ( buffer, nullptr );
		buffer = vk::Buffer();
	}
	if ( memory.memory ) {
		v_instance->free_gpu_memory ( memory );
	}
}
VBuffer::~VBuffer() {
	destroy();
}
