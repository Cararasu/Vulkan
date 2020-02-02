
#include "VHeader.h"
#include "VInstance.h"


const Logger v_logger ( "vulkan", LogLevel::Info );

void printError ( VkResult res ) {
	switch ( res ) {
	case VK_SUCCESS:
		v_logger.log<LogLevel::Error> ( "Success!" );
		break;
	case VK_NOT_READY:
		v_logger.log<LogLevel::Error> ( "Not Ready!" );
		break;
	case VK_TIMEOUT:
		v_logger.log<LogLevel::Error> ( "Timeout!" );
		break;
	case VK_EVENT_SET:
		v_logger.log<LogLevel::Error> ( "Event Set!" );
		break;
	case VK_EVENT_RESET:
		v_logger.log<LogLevel::Error> ( "Event Reset!" );
		break;
	case VK_RESULT_END_RANGE:
		v_logger.log<LogLevel::Error> ( "End Range / Incomplete!" );
		break;
	case VK_SUBOPTIMAL_KHR:
		v_logger.log<LogLevel::Error> ( "Suboptimal!" );
		break;
	case VK_RESULT_RANGE_SIZE:
		v_logger.log<LogLevel::Error> ( "Range Size!" );
		break;
	case VK_RESULT_MAX_ENUM:
		v_logger.log<LogLevel::Error> ( "Max Enum!" );
		break;
	case VK_ERROR_OUT_OF_HOST_MEMORY:
		v_logger.log<LogLevel::Error> ( "OOM Host!" );
		break;
	case VK_ERROR_OUT_OF_DEVICE_MEMORY:
		v_logger.log<LogLevel::Error> ( "OOM Device!" );
		break;
	case VK_ERROR_INITIALIZATION_FAILED:
		v_logger.log<LogLevel::Error> ( "Init failed!" );
		break;
	case VK_ERROR_DEVICE_LOST:
		v_logger.log<LogLevel::Error> ( "Device lost!" );
		break;
	case VK_ERROR_MEMORY_MAP_FAILED:
		v_logger.log<LogLevel::Error> ( "Memory map failed!" );
		break;
	case VK_ERROR_LAYER_NOT_PRESENT:
		v_logger.log<LogLevel::Error> ( "Layer not Ppresent!" );
		break;
	case VK_ERROR_EXTENSION_NOT_PRESENT:
		v_logger.log<LogLevel::Error> ( "Extension not present!" );
		break;
	case VK_ERROR_FEATURE_NOT_PRESENT:
		v_logger.log<LogLevel::Error> ( "Feature not present!" );
		break;
	case VK_ERROR_INCOMPATIBLE_DRIVER:
		v_logger.log<LogLevel::Error> ( "Incompatible Driver!" );
		break;
	case VK_ERROR_TOO_MANY_OBJECTS:
		v_logger.log<LogLevel::Error> ( "Too many Objects!" );
		break;
	case VK_ERROR_FORMAT_NOT_SUPPORTED:
		v_logger.log<LogLevel::Error> ( "Format not supported!" );
		break;
	case VK_ERROR_FRAGMENTED_POOL:
		v_logger.log<LogLevel::Error> ( "Fragmented pool/Begin Range!" );
		break;
	case VK_ERROR_SURFACE_LOST_KHR:
		v_logger.log<LogLevel::Error> ( "Surface lost!" );
		break;
	case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
		v_logger.log<LogLevel::Error> ( "Native window in use!" );
		break;
	case VK_ERROR_OUT_OF_DATE_KHR:
		v_logger.log<LogLevel::Error> ( "Incomplete!" );
		break;
	case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR:
		v_logger.log<LogLevel::Error> ( "Incompatible display!" );
		break;
	case VK_ERROR_VALIDATION_FAILED_EXT:
		v_logger.log<LogLevel::Error> ( "Validation failed!" );
		break;
	case VK_ERROR_INVALID_SHADER_NV:
		v_logger.log<LogLevel::Error> ( "Invalid Shader!" );
		break;
	case VK_ERROR_OUT_OF_POOL_MEMORY_KHR:
		v_logger.log<LogLevel::Error> ( "OOM pool!" );
		break;
	case VK_ERROR_INVALID_EXTERNAL_HANDLE_KHR:
		v_logger.log<LogLevel::Error> ( "Invalid External Handle!" );
		break;
	default:
		v_logger.log<LogLevel::Error> ( "Other Error 0x%x", res );
	}
	if ( res < 0 ) {
		glfwTerminate();
		exit ( res );
	}
}
