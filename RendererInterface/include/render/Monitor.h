#pragma once
#include "Dimensions.h"
#include "Header.h"

struct VideoMode{
	Extend2D extend;
	u32 red_bits, green_bits, blue_bits;
	u32 refresh_rate;
};

struct Monitor{
	char* name;
	Extend2D extend;
	Offset2D offset;
	std::vector<VideoMode> videomodes;
	
	virtual ~Monitor(){}
	//gamma stuff
};
