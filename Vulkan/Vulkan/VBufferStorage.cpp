#include "VBufferStorage.h"
#include "VInstance.h"

VTransientBufferStorage::VTransientBufferStorage ( VInstance* instance, vk::BufferUsageFlags usageflags ) :
	buffer ( instance ),
	staging_buffer(),
	needed_size ( 0 ),
	changed ( true ) {
	buffer.usage = usageflags;
}
VTransientBufferStorage::~VTransientBufferStorage () {
	
}
u64 VTransientBufferStorage::allocate_chunk ( u64 size ) {
	u64 t = max_offset;
	max_offset += size;
	return t;
}
constexpr u64 MIN_VERTEXBUFFER = 1024*1024;
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
	return staging_buffer.mapped_ptr;
}
vk::Semaphore VTransientBufferStorage::transfer_data ( vk::CommandBuffer commandbuffer ) {
	vk::BufferCopy buffercopy ( 0, 0, buffer.size );
	commandbuffer.copyBuffer ( staging_buffer.buffer, buffer.buffer, 1, &buffercopy );
	changed = false;
	return vk::Semaphore();
}

void VTransientBufferStorage::clear_transfer() {
	max_offset = 0;
	changed = true;
}
DynArray<VBuffer*> buffers;
DynArray<Chunk> freelist;
VUpdateableBufferStorage::VUpdateableBufferStorage ( VInstance* instance, vk::BufferUsageFlags usageflags ) :
	v_instance ( instance ), usageflags ( usageflags ),
	buffers ( { std::make_pair(new VBuffer ( instance, UNIFORM_BUFFER_SIZE, usageflags, vk::MemoryPropertyFlags() | vk::MemoryPropertyFlagBits::eDeviceLocal ), VThinBuffer())} ),
	freelist ( {{ 0, 0, UNIFORM_BUFFER_SIZE}} ) {

}
VUpdateableBufferStorage::~VUpdateableBufferStorage ( ) {
	for ( std::pair<VBuffer*, VThinBuffer> p : buffers ) {
		delete p.first;
	}
}

Chunk VUpdateableBufferStorage::allocate_chunk ( u64 size ) {
	u64 alignment = v_instance->v_device->vkPhysDevProps.limits.minUniformBufferOffsetAlignment;
	size = (((size - 1) / alignment) + 1) * alignment;
	for ( auto it = freelist.begin(); it != freelist.end(); it++ ) {//first check exact matches
		if ( it->size == size ) {
			Chunk c = *it;
			freelist.erase ( it );
			v_logger.log<LogLevel::eDebug> ("Allocating chunk from BufferStorage of size %" PRId64 " in Buffer %" PRId64 " Offset %" PRId64, size, c.index, c.offset);
			return c;
		}
	}
	for ( auto it = freelist.begin(); it != freelist.end(); it++ ) {//then bigger chunks
		if ( it->size > size ) {
			Chunk c = *it;
			c.size = size;
			it->offset += size;
			v_logger.log<LogLevel::eDebug> ("Allocating chunk from BufferStorage of size %" PRId64 " in Buffer %" PRId64 " Offset %" PRId64, size, c.index, c.offset);
			return c;
		}
	}
	if ( size >= UNIFORM_BUFFER_SIZE ) {
		buffers.push_back ( std::make_pair(new VBuffer ( v_instance, UNIFORM_BUFFER_SIZE, usageflags, vk::MemoryPropertyFlags() | vk::MemoryPropertyFlagBits::eDeviceLocal ), VThinBuffer()) );
		v_logger.log<LogLevel::eDebug> ("Allocating chunk from BufferStorage of size %" PRId64 " in Buffer %" PRId64 " Offset %" PRId64, buffers.size() - 1, 0, size);
		return {buffers.size() - 1, 0, size};
	}
	buffers.push_back ( std::make_pair(new VBuffer ( v_instance, UNIFORM_BUFFER_SIZE, usageflags, vk::MemoryPropertyFlags() | vk::MemoryPropertyFlagBits::eDeviceLocal ), VThinBuffer()) );
	freelist.push_back ( {buffers.size() - 1, size, UNIFORM_BUFFER_SIZE - size} );
	v_logger.log<LogLevel::eDebug> ("Allocating chunk from BufferStorage of size %" PRId64 " in Buffer %" PRId64 " Offset %" PRId64, buffers.size() - 1, 0, size);
	return {buffers.size() - 1, 0, size};
}
VBuffer* VUpdateableBufferStorage::get_buffer ( u64 index ) {
	return buffers[index].first;
}
void VUpdateableBufferStorage::fetch_transferbuffers() {
	for(std::pair<VBuffer*, VThinBuffer>& p : buffers) {
		p.second = v_instance->request_staging_buffer(p.first->size);
	}
}
void VUpdateableBufferStorage::free_transferbuffers(u64 frame_index) {
	for(std::pair<VBuffer*, VThinBuffer>& p : buffers) {
		p.second = VThinBuffer();
	}
}
std::pair<VBuffer*, VThinBuffer> VUpdateableBufferStorage::get_buffer_pair ( u64 index ) {
	return buffers[index];
}
void VUpdateableBufferStorage::free_chunk ( Chunk chunk ) {
	for ( auto it = freelist.begin(); it != freelist.end(); ) {
		if ( it->index == chunk.index ) {
			if ( it->offset + it->size == chunk.offset ) {
				chunk.offset -= it->offset;
				chunk.size += it->size;
				it = freelist.erase ( it );
				continue;
			}
			if ( chunk.offset + chunk.size == it->offset ) {
				chunk.size += it->size;
				it = freelist.erase ( it );
				continue;
			}
		}
		it++;
	}
	freelist.push_back ( chunk );
}
