#include "VTransformEnums.h"

VFormatData valuetypemap[] = {
	{ vk::Format::eUndefined, 0},
	
	{vk::Format::eR8Uint, 1},
	{vk::Format::eR16Uint, 1},
	{vk::Format::eR32Uint, 1},
	{vk::Format::eR64Uint, 1},
	{vk::Format::eR8Sint, 1},
	{vk::Format::eR16Sint, 1},
	{vk::Format::eR32Sint, 1},
	{vk::Format::eR64Sint, 1},
	{vk::Format::eR32Sfloat, 1},
	{vk::Format::eR64Sfloat, 1},
	
	{vk::Format::eR8G8Uint, 1},
	{vk::Format::eR16G16Uint, 1},
	{vk::Format::eR32G32Uint, 1},
	{vk::Format::eR64G64Uint, 1},
	{vk::Format::eR8G8Sint, 1},
	{vk::Format::eR16G16Sint, 1},
	{vk::Format::eR32G32Sint, 1},
	{vk::Format::eR64G64Sint, 1},
	{vk::Format::eR32G32Sfloat, 1},
	{vk::Format::eR64G64Sfloat, 1},
	
	{vk::Format::eR8G8B8Uint, 1},
	{vk::Format::eR16G16B16Uint, 1},
	{vk::Format::eR32G32B32Uint, 1},
	{vk::Format::eR64G64B64Uint, 1},
	{vk::Format::eR8G8B8Sint, 1},
	{vk::Format::eR16G16B16Sint, 1},
	{vk::Format::eR32G32B32Sint, 1},
	{vk::Format::eR64G64B64Sint, 1},
	{vk::Format::eR32G32B32Sfloat, 1},
	{vk::Format::eR64G64B64Sfloat, 1},
	
	{vk::Format::eR8G8B8A8Uint, 1},
	{vk::Format::eR16G16B16A16Uint, 1},
	{vk::Format::eR32G32B32A32Uint, 1},
	{vk::Format::eR64G64B64A64Uint, 1},
	{vk::Format::eR8G8B8A8Sint, 1},
	{vk::Format::eR16G16B16A16Sint, 1},
	{vk::Format::eR32G32B32A32Sint, 1},
	{vk::Format::eR64G64B64A64Sint, 1},
	{vk::Format::eR32G32B32A32Sfloat, 1},
	{vk::Format::eR64G64B64A64Sfloat, 1},
	
	{vk::Format::eR8G8Uint, 2},
	{vk::Format::eR16G16Uint, 2},
	{vk::Format::eR32G32Uint, 2},
	{vk::Format::eR64G64Uint, 2},
	{vk::Format::eR8G8Sint, 2},
	{vk::Format::eR16G16Sint, 2},
	{vk::Format::eR32G32Sint, 2},
	{vk::Format::eR64G64Sint, 2},
	{vk::Format::eR32G32Sfloat, 2},
	{vk::Format::eR64G64Sfloat, 2},
	
	{vk::Format::eR8G8B8Uint, 3},
	{vk::Format::eR16G16B16Uint, 3},
	{vk::Format::eR32G32B32Uint, 3},
	{vk::Format::eR64G64B64Uint, 3},
	{vk::Format::eR8G8B8Sint, 3},
	{vk::Format::eR16G16B16Sint, 3},
	{vk::Format::eR32G32B32Sint, 3},
	{vk::Format::eR64G64B64Sint, 3},
	{vk::Format::eR32G32B32Sfloat, 3},
	{vk::Format::eR64G64B64Sfloat, 3},
	
	{vk::Format::eR8G8B8A8Uint, 4},
	{vk::Format::eR16G16B16A16Uint, 4},
	{vk::Format::eR32G32B32A32Uint, 4},
	{vk::Format::eR64G64B64A64Uint, 4},
	{vk::Format::eR8G8B8A8Sint, 4},
	{vk::Format::eR16G16B16A16Sint, 4},
	{vk::Format::eR32G32B32A32Sint, 4},
	{vk::Format::eR64G64B64A64Sint, 4},
	{vk::Format::eR32G32B32A32Sfloat, 4},
	{vk::Format::eR64G64B64A64Sfloat, 4},
};
VFormatData to_v_format(ValueType type){
	return valuetypemap[(int)type];
}