#pragma once
#include "Dimensions.h"
#include "Header.h"

struct VideoMode{
	Extend2D<s32> extend;
	u32 red_bits, green_bits, blue_bits;
	u32 refresh_rate;
};

inline bool operator==(VideoMode lmode, VideoMode rmode){
	return lmode.extend == rmode.extend &&
	lmode.red_bits == rmode.red_bits &&
	lmode.green_bits == rmode.green_bits &&
	lmode.blue_bits == rmode.blue_bits &&
	lmode.refresh_rate == rmode.refresh_rate;
}

struct Monitor{
	const char* name;
	Extend2D<s32> extend;
	Offset2D<s32> offset;
	Array<VideoMode> videomodes;
	
	virtual ~Monitor(){}
	
	virtual VideoMode current_mode() = 0;
	
	//gamma stuff
};
