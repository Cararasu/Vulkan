#include "VTransformEnums.h"

VFormatData valuetypemap[] = {
	{ vk::Format::eUndefined, 0, 0},
	
	{vk::Format::eR8Uint, 1, 1},
	{vk::Format::eR16Uint, 2, 1},
	{vk::Format::eR32Uint, 4, 1},
	{vk::Format::eR64Uint, 8, 1},
	{vk::Format::eR8Sint, 1, 1},
	{vk::Format::eR16Sint, 2, 1},
	{vk::Format::eR32Sint, 4, 1},
	{vk::Format::eR64Sint, 8, 1},
	{vk::Format::eR32Sfloat, 4, 1},
	{vk::Format::eR64Sfloat, 8, 1},
	
	{vk::Format::eR8G8Uint, 2, 1},
	{vk::Format::eR16G16Uint, 4, 1},
	{vk::Format::eR32G32Uint, 8, 1},
	{vk::Format::eR64G64Uint, 16, 1},
	{vk::Format::eR8G8Sint, 2, 1},
	{vk::Format::eR16G16Sint, 4, 1},
	{vk::Format::eR32G32Sint, 8, 1},
	{vk::Format::eR64G64Sint, 16, 1},
	{vk::Format::eR32G32Sfloat, 8, 1},
	{vk::Format::eR64G64Sfloat, 16, 1},
	
	{vk::Format::eR8G8B8Uint, 3, 1},
	{vk::Format::eR16G16B16Uint, 6, 1},
	{vk::Format::eR32G32B32Uint, 12, 1},
	{vk::Format::eR64G64B64Uint, 24, 1},
	{vk::Format::eR8G8B8Sint, 3, 1},
	{vk::Format::eR16G16B16Sint, 6, 1},
	{vk::Format::eR32G32B32Sint, 12, 1},
	{vk::Format::eR64G64B64Sint, 24, 1},
	{vk::Format::eR32G32B32Sfloat, 12, 1},
	{vk::Format::eR64G64B64Sfloat, 24, 1},
	
	{vk::Format::eR8G8B8A8Uint, 4, 1},
	{vk::Format::eR16G16B16A16Uint, 8, 1},
	{vk::Format::eR32G32B32A32Uint, 16, 1},
	{vk::Format::eR64G64B64A64Uint, 32, 1},
	{vk::Format::eR8G8B8A8Sint, 4, 1},
	{vk::Format::eR16G16B16A16Sint, 8, 1},
	{vk::Format::eR32G32B32A32Sint, 16, 1},
	{vk::Format::eR64G64B64A64Sint, 32, 1},
	{vk::Format::eR32G32B32A32Sfloat, 16, 1},
	{vk::Format::eR64G64B64A64Sfloat, 32, 1},
	
	{vk::Format::eR8G8Uint, 2, 2},
	{vk::Format::eR16G16Uint, 4, 2},
	{vk::Format::eR32G32Uint, 8, 2},
	{vk::Format::eR64G64Uint, 16, 2},
	{vk::Format::eR8G8Sint, 2, 2},
	{vk::Format::eR16G16Sint, 4, 2},
	{vk::Format::eR32G32Sint, 8, 2},
	{vk::Format::eR64G64Sint, 16, 2},
	{vk::Format::eR32G32Sfloat, 8, 2},
	{vk::Format::eR64G64Sfloat, 16, 2},
	
	{vk::Format::eR8G8B8Uint, 3, 3},
	{vk::Format::eR16G16B16Uint, 6, 3},
	{vk::Format::eR32G32B32Uint, 12, 3},
	{vk::Format::eR64G64B64Uint, 24, 3},
	{vk::Format::eR8G8B8Sint, 3, 3},
	{vk::Format::eR16G16B16Sint, 6, 3},
	{vk::Format::eR32G32B32Sint, 12, 3},
	{vk::Format::eR64G64B64Sint, 16, 3},
	{vk::Format::eR32G32B32Sfloat, 12, 3},
	{vk::Format::eR64G64B64Sfloat, 24, 3},
	
	{vk::Format::eR8G8B8A8Uint, 4, 4},
	{vk::Format::eR16G16B16A16Uint, 8, 4},
	{vk::Format::eR32G32B32A32Uint, 16, 4},
	{vk::Format::eR64G64B64A64Uint, 32, 4},
	{vk::Format::eR8G8B8A8Sint, 4, 4},
	{vk::Format::eR16G16B16A16Sint, 8, 4},
	{vk::Format::eR32G32B32A32Sint, 16, 4},
	{vk::Format::eR64G64B64A64Sint, 32, 4},
	{vk::Format::eR32G32B32A32Sfloat, 16, 4},
	{vk::Format::eR64G64B64A64Sfloat, 32, 4},
};
VFormatData to_v_format(ValueType type){
	return valuetypemap[(int)type];
}