
#include "VHeader.h"
#include "VInstance.h"


std::string vulkan_str = "vulkan";

const Logger v_logger ( vulkan_str );

void printError(VkResult res){
	switch(res){
	case VK_SUCCESS:
		puts("Success!\n");
		break;
	case VK_NOT_READY:
		puts("Not Ready!\n");
		break;
	case VK_TIMEOUT:
		puts("Timeout!\n");
		break;
	case VK_EVENT_SET:
		puts("Event Set!\n");
		break;
	case VK_EVENT_RESET:
		puts("Event Reset!\n");
		break;
	case VK_RESULT_END_RANGE:
		puts("End Range / Incomplete!\n");
		break;
	case VK_SUBOPTIMAL_KHR:
		puts("Suboptimal!\n");
		break;
	case VK_RESULT_RANGE_SIZE:
		puts("Range Size!\n");
		break;
	case VK_RESULT_MAX_ENUM:
		puts("Max Enum!\n");
		break;
	case VK_ERROR_OUT_OF_HOST_MEMORY:
		puts("OOM Host!\n");
		break;
	case VK_ERROR_OUT_OF_DEVICE_MEMORY:
		puts("OOM Device!\n");
		break;
	case VK_ERROR_INITIALIZATION_FAILED:
		puts("Init failed!\n");
		break;
	case VK_ERROR_DEVICE_LOST:
		puts("Device lost!\n");
		break;
	case VK_ERROR_MEMORY_MAP_FAILED:
		puts("Memory map failed!\n");
		break;
	case VK_ERROR_LAYER_NOT_PRESENT:
		puts("Layer not Ppresent!\n");
		break;
	case VK_ERROR_EXTENSION_NOT_PRESENT:
		puts("Extension not present!\n");
		break;
	case VK_ERROR_FEATURE_NOT_PRESENT:
		puts("Feature not present!\n");
		break;
	case VK_ERROR_INCOMPATIBLE_DRIVER:
		puts("Incompatible Driver!\n");
		break;
	case VK_ERROR_TOO_MANY_OBJECTS:
		puts("Too many Objects!\n");
		break;
	case VK_ERROR_FORMAT_NOT_SUPPORTED:
		puts("Format not supported!\n");
		break;
	case VK_ERROR_FRAGMENTED_POOL:
		puts("Fragmented pool/Begin Range!\n");
		break;
	case VK_ERROR_SURFACE_LOST_KHR:
		puts("Surface lost!\n");
		break;
	case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
		puts("Native window in use!\n");
		break;
	case VK_ERROR_OUT_OF_DATE_KHR:
		puts("Incomplete!\n");
		break;
	case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR:
		puts("Incompatible display!\n");
		break;
	case VK_ERROR_VALIDATION_FAILED_EXT:
		puts("Validation failed!\n");
		break;
	case VK_ERROR_INVALID_SHADER_NV:
		puts("Invalid Shader!\n");
		break;
	case VK_ERROR_OUT_OF_POOL_MEMORY_KHR:
		puts("OOM pool!\n");
		break;
	case VK_ERROR_INVALID_EXTERNAL_HANDLE_KHR:
		puts("Invalid External Handle!\n");
		break;
	default:
		printf("Other Error 0x%x\n", res);
	}
	if(res < 0){
		glfwTerminate();
		exit(res);
	}
}