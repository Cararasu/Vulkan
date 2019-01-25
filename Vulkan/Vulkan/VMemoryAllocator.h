#pragma once

#include <cstdlib>

struct SimpleAllocationChunk {
	void* data = nullptr;
	size_t size = 0, bytesleft = 0;
};

#define ROUND_UP(val, mod) ((((val - 1) / mod) + 1) * mod)

const size_t BYTE_ALIGNMENT = sizeof ( size_t );
const size_t ALLOCATED_CHUNK_SIZE = ( 32 * 1024 * 1024 );
const size_t EFFECTIVELY_USABLE_CHUNK_SIZE = ( 32 * 1024 * 1024 ) - ROUND_UP ( sizeof ( SimpleAllocationChunk ), BYTE_ALIGNMENT );

struct AlwaysGrowAllocator {
	SimpleAllocationChunk first_chunk;
	SimpleAllocationChunk* first_free_chunk;

	void init_chunk ( SimpleAllocationChunk* basechunk ) {
		if ( !basechunk->data ) {
			basechunk->data = malloc ( ALLOCATED_CHUNK_SIZE );
			SimpleAllocationChunk* chunk = reinterpret_cast<SimpleAllocationChunk*> ( first_chunk.data );
			chunk->data = nullptr;
			chunk->size = 0;
			chunk->data = 0;
			basechunk->bytesleft = EFFECTIVELY_USABLE_CHUNK_SIZE;
			basechunk->size = ALLOCATED_CHUNK_SIZE;
		}
		first_free_chunk = basechunk;
	}
	AlwaysGrowAllocator() : first_chunk() {
		init_chunk ( &first_chunk );
	}
	~AlwaysGrowAllocator() {
		SimpleAllocationChunk* chunk = reinterpret_cast<SimpleAllocationChunk*> ( first_chunk.data );
		while ( chunk ) {
			SimpleAllocationChunk* t_chunk = reinterpret_cast<SimpleAllocationChunk*> ( chunk->data );
			free ( chunk );
			chunk = t_chunk;
		}
	}
	void* allocate ( size_t bytes, size_t alignment = BYTE_ALIGNMENT ) {
		assert ( bytes <= EFFECTIVELY_USABLE_CHUNK_SIZE );
		fflush ( stdout );

		SimpleAllocationChunk* chunk = first_free_chunk;
		while ( chunk->data ) {
			size_t next_free_ptr = reinterpret_cast<size_t> ( chunk->data ) + ALLOCATED_CHUNK_SIZE - chunk->bytesleft;
			size_t aligned_ptr = ROUND_UP ( next_free_ptr, alignment );
			size_t padding = ( aligned_ptr - next_free_ptr );
			while ( padding + bytes <= chunk->bytesleft ) {
				if ( sizeof ( size_t ) < padding ) {
					padding += alignment;
					aligned_ptr += alignment;
				} else {
					*reinterpret_cast<size_t*> ( aligned_ptr - sizeof ( size_t ) ) = bytes;
					return reinterpret_cast<void*> ( aligned_ptr );
				}
			}
			chunk = static_cast<SimpleAllocationChunk*> ( chunk->data );
			init_chunk ( chunk );
		}
		init_chunk ( chunk );
		void* ptr = chunk->data + ALLOCATED_CHUNK_SIZE - chunk->bytesleft;
		chunk->bytesleft -= bytes;
		fflush ( stdout );
		return ptr;
	}
	void* reallocate ( void* oldptr, size_t bytes, size_t alignment = BYTE_ALIGNMENT ) {
		if ( !oldptr ) return allocate ( bytes, alignment );

		size_t data_size = static_cast<size_t*> ( oldptr ) [-1];
		if ( data_size == bytes ) return oldptr;

		void* new_allocation = allocate ( bytes, alignment );
		memcpy ( new_allocation, oldptr, bytes <= data_size ? bytes : data_size );
		return new_allocation;
	}
	void free ( void* data ) {
		//no-op
	}
	void clear() {
		int i = 0;
		SimpleAllocationChunk* chunk = &first_chunk;
		while ( chunk->data ) {
			chunk->bytesleft = EFFECTIVELY_USABLE_CHUNK_SIZE;
			chunk = reinterpret_cast<SimpleAllocationChunk*> ( chunk->data );
			i++;
		}
		printf ( "Clearing %d chunks\n", i );
		first_free_chunk = &first_chunk;
	}
};