#include "VBufferStorage.h"
#include "VInstance.h"

VTransientBufferStorage::VTransientBufferStorage ( VInstance* instance, vk::BufferUsageFlags usageflags ) :
	buffer ( instance ),
	needed_size ( 0 ),
	changed ( true ) {
	buffer.usage = usageflags;
}
VTransientBufferStorage::~VTransientBufferStorage () {
	if ( staging_buffer ) buffer.v_instance->free_staging_buffer ( staging_buffer );
}
u64 VTransientBufferStorage::allocate_chunk ( u64 size ) {
	u64 t = max_offset;
	max_offset += size;
	return t;
}
constexpr u64 MIN_VERTEXBUFFER = 4192;
void* VTransientBufferStorage::allocate_transfer_buffer ( ) {
	needed_size = max_offset > MIN_VERTEXBUFFER ? max_offset : MIN_VERTEXBUFFER;

	//buffer is not yet initialized
	if ( buffer.size == 0 ) {
		u32 calcsize = MIN_VERTEXBUFFER;
		while ( calcsize < needed_size ) calcsize *= 2;
		buffer.init ( calcsize, buffer.usage, vk::MemoryPropertyFlagBits::eDeviceLocal );
	}
	//if the buffer is too small we make it smaller
	//or it is at least 4 times too big we make it smaller
	else if ( ( buffer.size > needed_size ) || ( needed_size >= MIN_VERTEXBUFFER && needed_size < buffer.size / 4 ) ) {
		buffer.destroy();
		while ( buffer.size < needed_size ) buffer.size *= 2;
		while ( needed_size >= MIN_VERTEXBUFFER && buffer.size / 4 > needed_size ) buffer.size /= 2;
		buffer.init();
	}

	staging_buffer = buffer.v_instance->request_staging_buffer ( needed_size );
	staging_buffer->map_mem();
	return staging_buffer->mapped_ptr;
}
vk::Semaphore VTransientBufferStorage::transfer_data ( vk::CommandBuffer commandbuffer ) {
	vk::BufferCopy buffercopy ( 0, 0, buffer.size );
	commandbuffer.copyBuffer ( staging_buffer->buffer, buffer.buffer, 1, &buffercopy );
	buffer.v_instance->free_staging_buffer ( staging_buffer );
	staging_buffer = nullptr;
	changed = false;
	return vk::Semaphore();
}

void VTransientBufferStorage::clear_transfer() {
	max_offset = 0;
	changed = true;
}
VUpdateableBufferStorage::VUpdateableBufferStorage ( VInstance* instance, vk::BufferUsageFlags usageflags ) :
	buffer ( instance ),
	needed_size ( 0 ) {
	buffer.usage = usageflags;

}
VUpdateableBufferStorage::~VUpdateableBufferStorage ( ) {

}
