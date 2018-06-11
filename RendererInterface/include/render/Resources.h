#pragma once

#include "Header.h"

//contiguous GPU memory

class Resource{
	virtual bool is_ready() = 0;
};
//Image
//	bufferbacked

//Buffer
//	shaderinput

//IndexVertexData
//	writable(feedback loop)

//ShaderInputData

struct ResourceManager{
	
	template<typename T>
	T* get_resource(const char*);
	template<typename T>
	T* get_resource(ResourceId);
	
	
};