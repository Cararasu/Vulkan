#pragma once
#include "Dimensions.h"
#include "Header.h"

struct VideoMode{
	Extend2D<s32> extend;
	u32 red_bits, green_bits, blue_bits;
	u32 refresh_rate;
};

struct Monitor{
	const char* name;
	Extend2D<s32> extend;
	Offset2D<s32> offset;
	Array<VideoMode> videomodes;
	
	virtual ~Monitor(){}
	
	//gamma stuff
};
