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
	vk::Device device = v_instance->vk_device ();
	vk::BufferCreateInfo bufferInfo ( vk::BufferCreateFlags(), size, usage, vk::SharingMode::eExclusive );

	V_CHECKCALL ( device.createBuffer ( &bufferInfo, nullptr, &buffer ), printf ( "Failed To Create Buffer\n" ) );

	printf ( "Create Buffer of size %d with usage %s with buffer 0x%" PRIx64 "\n", size, to_string(usage).c_str(), reinterpret_cast<u64>(static_cast<VkBuffer>(buffer)) );

	vk::MemoryRequirements mem_req;
	device.getBufferMemoryRequirements ( buffer, &mem_req );
	memory = v_instance->allocate_gpu_memory ( mem_req, needed, recommended );
	device.bindBufferMemory ( buffer, memory.memory, 0 );
	if(memory.memory) {
		return RendResult::eFail;
	}
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
	v_instance->vk_device ().mapMemory ( memory.memory, ( vk::DeviceSize ) 0L, memory.size, vk::MemoryMapFlags(), &mapped_ptr );
	return mapped_ptr ? RendResult::eSuccess : RendResult::eFail;
}
RendResult VBuffer::unmap_mem() {
	v_instance->vk_device ().unmapMemory ( memory.memory );
	mapped_ptr = nullptr;
}
void VBuffer::destroy() {
	if ( mapped_ptr )
		unmap_mem();
	if ( buffer ) {
		printf("Destroying Buffer 0x%" PRIx64 "\n", buffer);
		v_instance->vk_device ().destroyBuffer ( buffer, nullptr );
		buffer = vk::Buffer();
	}
	if ( memory.memory ) {
		v_instance->free_gpu_memory ( memory );
	}
}
VBuffer::~VBuffer() {
	destroy();
}

RendResult transfer_buffer_data(VSimpleTransferJob& job, vk::CommandBuffer commandBuffer){
	commandBuffer.copyBuffer ( 
		job.source_buffer->buffer, 
		job.target_buffer->buffer, 
		job.sections );
}