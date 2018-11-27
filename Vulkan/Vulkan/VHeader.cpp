
#include "VHeader.h"
#include "VInstance.h"


const Logger v_logger ( "vulkan", LogLevel::eInfo );

void printError ( VkResult res ) {
	switch ( res ) {
	case VK_SUCCESS:
		v_logger.log<LogLevel::eError> ( "Success!" );
		break;
	case VK_NOT_READY:
		v_logger.log<LogLevel::eError> ( "Not Ready!" );
		break;
	case VK_TIMEOUT:
		v_logger.log<LogLevel::eError> ( "Timeout!" );
		break;
	case VK_EVENT_SET:
		v_logger.log<LogLevel::eError> ( "Event Set!" );
		break;
	case VK_EVENT_RESET:
		v_logger.log<LogLevel::eError> ( "Event Reset!" );
		break;
	case VK_RESULT_END_RANGE:
		v_logger.log<LogLevel::eError> ( "End Range / Incomplete!" );
		break;
	case VK_SUBOPTIMAL_KHR:
		v_logger.log<LogLevel::eError> ( "Suboptimal!" );
		break;
	case VK_RESULT_RANGE_SIZE:
		v_logger.log<LogLevel::eError> ( "Range Size!" );
		break;
	case VK_RESULT_MAX_ENUM:
		v_logger.log<LogLevel::eError> ( "Max Enum!" );
		break;
	case VK_ERROR_OUT_OF_HOST_MEMORY:
		v_logger.log<LogLevel::eError> ( "OOM Host!" );
		break;
	case VK_ERROR_OUT_OF_DEVICE_MEMORY:
		v_logger.log<LogLevel::eError> ( "OOM Device!" );
		break;
	case VK_ERROR_INITIALIZATION_FAILED:
		v_logger.log<LogLevel::eError> ( "Init failed!" );
		break;
	case VK_ERROR_DEVICE_LOST:
		v_logger.log<LogLevel::eError> ( "Device lost!" );
		break;
	case VK_ERROR_MEMORY_MAP_FAILED:
		v_logger.log<LogLevel::eError> ( "Memory map failed!" );
		break;
	case VK_ERROR_LAYER_NOT_PRESENT:
		v_logger.log<LogLevel::eError> ( "Layer not Ppresent!" );
		break;
	case VK_ERROR_EXTENSION_NOT_PRESENT:
		v_logger.log<LogLevel::eError> ( "Extension not present!" );
		break;
	case VK_ERROR_FEATURE_NOT_PRESENT:
		v_logger.log<LogLevel::eError> ( "Feature not present!" );
		break;
	case VK_ERROR_INCOMPATIBLE_DRIVER:
		v_logger.log<LogLevel::eError> ( "Incompatible Driver!" );
		break;
	case VK_ERROR_TOO_MANY_OBJECTS:
		v_logger.log<LogLevel::eError> ( "Too many Objects!" );
		break;
	case VK_ERROR_FORMAT_NOT_SUPPORTED:
		v_logger.log<LogLevel::eError> ( "Format not supported!" );
		break;
	case VK_ERROR_FRAGMENTED_POOL:
		v_logger.log<LogLevel::eError> ( "Fragmented pool/Begin Range!" );
		break;
	case VK_ERROR_SURFACE_LOST_KHR:
		v_logger.log<LogLevel::eError> ( "Surface lost!" );
		break;
	case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
		v_logger.log<LogLevel::eError> ( "Native window in use!" );
		break;
	case VK_ERROR_OUT_OF_DATE_KHR:
		v_logger.log<LogLevel::eError> ( "Incomplete!" );
		break;
	case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR:
		v_logger.log<LogLevel::eError> ( "Incompatible display!" );
		break;
	case VK_ERROR_VALIDATION_FAILED_EXT:
		v_logger.log<LogLevel::eError> ( "Validation failed!" );
		break;
	case VK_ERROR_INVALID_SHADER_NV:
		v_logger.log<LogLevel::eError> ( "Invalid Shader!" );
		break;
	case VK_ERROR_OUT_OF_POOL_MEMORY_KHR:
		v_logger.log<LogLevel::eError> ( "OOM pool!" );
		break;
	case VK_ERROR_INVALID_EXTERNAL_HANDLE_KHR:
		v_logger.log<LogLevel::eError> ( "Invalid External Handle!" );
		break;
	default:
		v_logger.log<LogLevel::eError> ( "Other Error 0x%x", res );
	}
	if ( res < 0 ) {
		glfwTerminate();
		exit ( res );
	}
}
