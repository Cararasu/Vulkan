#pragma once
#include "Dimensions.h"
#include "Header.h"

struct VideoMode{
	Extend2D<s32> extend;
	u32 refresh_rate;
};

inline bool operator==(VideoMode lmode, VideoMode rmode){
	return lmode.extend == rmode.extend && lmode.refresh_rate == rmode.refresh_rate;
}
inline bool operator!=(VideoMode lmode, VideoMode rmode){
	return !operator==(lmode, rmode);
}

struct Monitor{
	const char* name;
	Extend2D<s32> extend;
	Offset2D<s32> offset;
	Array<VideoMode> videomodes;
	
	virtual ~Monitor(){}
	
	VideoMode find_best_videomode(Extend2D<s32> size, u32 refreshrate){
		if(size.x <= 0 || size.y <= 0 || refreshrate <= 0)
			return current_mode();
		VideoMode best_mode;
		bool mode_is_possible_for_monitor = false;
		for (VideoMode& mode : videomodes) { //select the best mode
			if (mode.extend.x >= size.x && mode.extend.y >= size.y && mode.refresh_rate >= refreshrate) {
				if (!mode_is_possible_for_monitor) {
					best_mode = mode;
					mode_is_possible_for_monitor = true;
				} else if (mode.extend.x <= best_mode.extend.x && mode.extend.y <= best_mode.extend.y && mode.refresh_rate <= best_mode.refresh_rate) {
					best_mode = mode;
				}
			}
		}
		if (mode_is_possible_for_monitor) {
			return best_mode;
		}
		return current_mode();
	}
	virtual VideoMode current_mode() = 0;
	
	//gamma stuff
};
