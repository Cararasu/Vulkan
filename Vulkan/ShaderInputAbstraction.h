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
template<typename BASETYPE, u32 BITSIZE, u32 HEIGHT, u32 WIDTH, u32 DIMENSION, bool ARRAY>
class IDSampledImage : public InputDefinition{
	
};
template<typename BASETYPE, u32 BITSIZE, u32 HEIGHT, u32 WIDTH, u32 DIMENSION, bool ARRAY, bool WRITABLE>
class IDImage : public InputDefinition{
	
};
template<typename BASETYPE, u32 BITSIZE, u32 HEIGHT, u32 WIDTH, bool WRITABLE>
class IDBuffer : public InputDefinition{
	
};
template<typename BASETYPE, u32 BITSIZE, u32 HEIGHT, u32 WIDTH, bool WRITABLE>
class PixelBuffer : public InputDefinition{
	
};


class InputBLob{
	std::vector<InputDefinition*> inputs;
};


#endif // SHADERINPUTABSTRACTION_H
