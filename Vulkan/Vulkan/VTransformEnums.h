#pragma once

#include "VHeader.h"


struct VFormatData {
	vk::Format format;
	u32 bytesize;
	u32 count;
};

VFormatData to_v_format ( ValueType type );
