#pragma once

#include <cstdlib>


struct PerFrameAllocatorChunk {
	void* data = nullptr;
	size_t size = 0;
	size_t bytesleft = 0;
};

#define ROUND_UP(val, mod) ((((val - 1) / mod) + 1) * mod)

const size_t BYTE_LEFT_THRESHOLD = 32;
const size_t BYTE_ALIGNMENT = 8;
const size_t ALLOCATED_CHUNK_SIZE = (32 * 1024 * 1024);
const size_t EFFECTIVELY_USABLE_CHUNK_SIZE = (32 * 1024 * 1024) - ROUND_UP(sizeof(PerFrameAllocatorChunk), BYTE_ALIGNMENT);

struct PerFrameAllocator {
	PerFrameAllocatorChunk first_chunk;
	PerFrameAllocatorChunk* first_free_chunk;
	
	PerFrameAllocator();
	~PerFrameAllocator();
	
	void* init_chunk(PerFrameAllocatorChunk* basechunk) {
		basechunk->data = malloc(ALLOCATED_CHUNK_SIZE);
		basechunk->bytesleft = EFFECTIVELY_USABLE_CHUNK_SIZE;
		basechunk->size = ALLOCATED_CHUNK_SIZE;
		first_free_chunk = &first_chunk;
	}
	void* allocate(size_t bytes) {
		assert(bytes <= EFFECTIVELY_USABLE_CHUNK_SIZE);
		
		bytes = ROUND_UP(bytes, BYTE_ALIGNMENT);
		PerFrameAllocatorChunk* chunk = first_free_chunk;
		while(chunk->data) {
			if(bytes <= chunk->bytesleft) {
				void* ptr = chunk->data + ALLOCATED_CHUNK_SIZE - chunk->bytesleft;
				chunk->bytesleft -= bytes;
				return ptr;
			}
			if(bytes < BYTE_LEFT_THRESHOLD) first_free_chunk = reinterpret_cast<PerFrameAllocatorChunk*>(chunk->data);
			chunk = reinterpret_cast<PerFrameAllocatorChunk*>(chunk->data);
		}
		init_chunk(chunk);
		void* ptr = chunk->data + ALLOCATED_CHUNK_SIZE - chunk->bytesleft;
		chunk->bytesleft -= bytes;
		return ptr;
	}
	void free(void* data) {
		//no-op
	}
	void clear() {
		int i = 0;
		PerFrameAllocatorChunk* chunk = &first_chunk;
		while(chunk->data) {
			i++;
			chunk->bytesleft = EFFECTIVELY_USABLE_CHUNK_SIZE;
			chunk = reinterpret_cast<PerFrameAllocatorChunk*>(chunk->data);
		}
		printf("Clearing %d chunks\n", i);
		first_free_chunk = &first_chunk;
	}
};
PerFrameAllocator::PerFrameAllocator() : first_chunk() {
	init_chunk(&first_chunk);
}
PerFrameAllocator::~PerFrameAllocator() {
	PerFrameAllocatorChunk* chunk = reinterpret_cast<PerFrameAllocatorChunk*>(first_chunk.data);
	while(chunk) {
		PerFrameAllocatorChunk* t_chunk = reinterpret_cast<PerFrameAllocatorChunk*>(chunk->data);
		free(chunk);
		chunk = t_chunk;
	}
}