
#ifndef VHEADER_H
#define VHEADER_H

#include <vulkan/vulkan.h>
#include <stdio.h>

void printError(VkResult res);


#define VCHECKCALL(call, errorMessage) {\
	if(VkResult res = call){\
		printf(errorMessage);\
		printError(res);\
	}\
}


#endif