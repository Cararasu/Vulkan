#ifndef SHADERINPUTABSTRACTION_H
#define SHADERINPUTABSTRACTION_H

/*
InputBlob
	InputDefinition

InputDefinition
	Sampler
	SampledImage(f/u/i)(nxm)(32/64)(1/2/3)D
	SampledImageArray(f/u/i)(nxm)(32/64)(1/2/3)D
	(R/RW)Image(f/u/i)(nxm)(32/64)(1/2/3)D
	(R/RW)ImageArray(f/u/i)(nxm)(32/64)(1/2/3)D
	(R/RW)Buffer(f/u/i)(nxm)(32/64)
	PixelInput(f/u/i)(nxm)(32/64)

BuiltIns
	FragCoord
	InstanceIndex
*/
#include "VHeader.h"
#include <vector>
class InputDefinition{
	
};
class IDSampler : public InputDefinition{
	
};
template<typename BASETYPE, uint32_t BITSIZE, uint32_t HEIGHT, uint32_t WIDTH, uint32_t DIMENSION, bool ARRAY>
class IDSampledImage : public InputDefinition{
	
};
template<typename BASETYPE, uint32_t BITSIZE, uint32_t HEIGHT, uint32_t WIDTH, uint32_t DIMENSION, bool ARRAY, bool WRITABLE>
class IDImage : public InputDefinition{
	
};
template<typename BASETYPE, uint32_t BITSIZE, uint32_t HEIGHT, uint32_t WIDTH, bool WRITABLE>
class IDBuffer : public InputDefinition{
	
};
template<typename BASETYPE, uint32_t BITSIZE, uint32_t HEIGHT, uint32_t WIDTH, bool WRITABLE>
class PixelBuffer : public InputDefinition{
	
};


class InputBLob{
	std::vector<InputDefinition*> inputs;
};


#endif // SHADERINPUTABSTRACTION_H
