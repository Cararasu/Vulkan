#include "VBuffer.h"
#include "VInstance.h"

VBuffer::VBuffer ( VInstance* instance ) :
	v_instance ( instance ), size ( 0 ) {
}
VBuffer::VBuffer ( VInstance* instance, vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags needed, vk::MemoryPropertyFlags recommended ) :
	v_instance ( instance ) {
	init ( size, usage, needed, recommended );
}
RendResult VBuffer::init ( vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags needed, vk::MemoryPropertyFlags recommended ) {
	if ( buffer ) destroy();
	this->memory = GPUMemory ( size, needed, recommended );
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

	V_CHECKCALL ( device.createBuffer ( &bufferInfo, nullptr, &buffer ), v_logger.log<LogLevel::eError> ( "Failed To Create Buffer" ) );

	v_logger.log<LogLevel::eDebug> ( "Create Buffer of size %" PRId64 " with usage %s with buffer 0x%" PRIx64, size, to_string ( usage ).c_str(), reinterpret_cast<u64> ( static_cast<VkBuffer> ( buffer ) ) );

	vk::MemoryRequirements mem_req;
	device.getBufferMemoryRequirements ( buffer, &mem_req );
	v_instance->allocate_gpu_memory ( mem_req, &memory );
	device.bindBufferMemory ( buffer, memory.memory, 0 );
	last_build_frame_index = v_instance->frame_index;
	if ( memory.memory ) {
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
	if ( mapped_ptr ) {
		return RendResult::eSuccess;
	}
	vk::Result res = v_instance->vk_device ().mapMemory ( memory.memory, ( vk::DeviceSize ) 0L, memory.size, vk::MemoryMapFlags(), &mapped_ptr );
	return mapped_ptr ? RendResult::eSuccess : RendResult::eFail;
}
RendResult VBuffer::unmap_mem() {
	v_instance->vk_device ().unmapMemory ( memory.memory );
	mapped_ptr = nullptr;
	return RendResult::eSuccess;
}
void VBuffer::destroy() {
	if ( mapped_ptr )
		unmap_mem();
	if ( buffer ) {
		v_logger.log<LogLevel::eDebug> ( "Destroying Buffer 0x%" PRIx64, static_cast<VkBuffer>(buffer) );
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

VThinBuffer::VThinBuffer ( VInstance* instance, vk::Buffer buffer, vk::DeviceSize size, void* mapped_ptr) : v_instance(v_instance), buffer(buffer), size(size), mapped_ptr(mapped_ptr) {
	
}
VThinBuffer::~VThinBuffer() {
	
}