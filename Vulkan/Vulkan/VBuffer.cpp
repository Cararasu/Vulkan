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

VDividableBufferStore::VDividableBufferStore ( VInstance* v_instance, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags needed, vk::MemoryPropertyFlags recommended ) :
	v_instance(v_instance), usage(usage), needed(needed), recommended(recommended) {
}
VDividableBufferStore::~VDividableBufferStore() {
	destroy();
}
VThinBuffer VDividableBufferStore::acquire_buffer(u64 size) {
	last_frame_index_acquired = v_instance->frame_index;
	vk::Device device = v_instance->vk_device ();
	vk::Buffer buffer;
	vk::BufferCreateInfo bufferInfo ( vk::BufferCreateFlags(), size, usage, vk::SharingMode::eExclusive );
	V_CHECKCALL ( device.createBuffer ( &bufferInfo, nullptr, &buffer ), v_logger.log<LogLevel::eError> ( "Failed To Create Buffer" ) );
	
	vk::MemoryRequirements mem_req;
	device.getBufferMemoryRequirements ( buffer, &mem_req );
	
	for(VDividableMemory& memory : memory_chunks) {
		u64 possible_offset = (((memory.offset - 1) / mem_req.alignment) + 1) * mem_req.alignment;
		
		if ( possible_offset < memory.memory.size && mem_req.size < memory.memory.size - possible_offset) {
			device.bindBufferMemory ( buffer, memory.memory.memory, possible_offset );
			memory.offset = possible_offset + size;
			buffers.push_back(buffer);
			return VThinBuffer ( v_instance, buffer, size, memory.mapped_ptr + possible_offset);
		}
	}
	vk::MemoryRequirements new_mem_req = mem_req;
	new_mem_req.size = new_mem_req.size > MAX_MEMORY_CUNK_SIZE ? new_mem_req.size : MAX_MEMORY_CUNK_SIZE;
	
	GPUMemory memory( new_mem_req.size, needed, recommended );
	v_logger.log<LogLevel::eDebug>("Allocating Memory for transfer of size %" PRId64, new_mem_req.size);
	v_instance->allocate_gpu_memory ( new_mem_req, &memory );
	void* mapped_ptr;
	vk::Result res = v_instance->vk_device ().mapMemory ( memory.memory, ( vk::DeviceSize ) 0L, memory.size, vk::MemoryMapFlags(), &mapped_ptr );
	
	device.bindBufferMemory ( buffer, memory.memory, 0 );
	if(new_mem_req.size > MAX_MEMORY_CUNK_SIZE) {
		special_memory_chunks.push_back ( {memory, mem_req.size, mapped_ptr} );
	}else{
		memory_chunks.push_back ( {memory, mem_req.size, mapped_ptr} );
	}
	buffers.push_back(buffer);
	return VThinBuffer ( v_instance, buffer, size, mapped_ptr);
}
void VDividableBufferStore::free_buffers() {
	for(vk::Buffer buffer : buffers) {
		v_logger.log<LogLevel::eDebug> ( "Destroying Buffer 0x%" PRIx64, static_cast<VkBuffer>(buffer) );
		v_instance->vk_device ().destroyBuffer ( buffer, nullptr );
	}
	for(VDividableMemory& chunk : memory_chunks) {
		chunk.offset = 0;
	}
	for(VDividableMemory& chunk : special_memory_chunks) {
		v_instance->vk_device ().unmapMemory ( chunk.memory.memory );
		v_instance->free_gpu_memory ( chunk.memory );
	}
	special_memory_chunks.clear();
	buffers.clear();
}
void VDividableBufferStore::destroy() {
	free_buffers();
	for(VDividableMemory& chunk : memory_chunks) {
		v_instance->vk_device ().unmapMemory ( chunk.memory.memory );
		v_instance->free_gpu_memory ( chunk.memory );
	}
	memory_chunks.clear();
}