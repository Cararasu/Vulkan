#pragma once

struct Quad{
	uint32_t imageindex;
	glm::fvec4 image_dimensions;
	glm::fvec4 dimensions;
};

struct QuadRenderer{
	ImageWrapper* image_wrapper = nullptr;
	
	BufferWrapper* staging_buffer = nullptr;
	MappedBufferWrapper* mapped_staging_buffer = nullptr;
	
	
	
};

struct WorldRenderer{
	ImageWrapper* image_wrapper = nullptr;
	
	BufferWrapper* staging_buffer = nullptr;
	MappedBufferWrapper* mapped_staging_buffer = nullptr;
	
	
	
};
