#pragma once

#include <cstdio>
#include <cstdarg>
#include <string>

enum class LogLevel {
	eTrace,
	eDebug,
	eInfo,
	eWarn,
	eError,
};

struct Logger {
	std::string module_name;
	LogLevel level;
	FILE * log_output;

	Logger ( std::string module_name, LogLevel level = LogLevel::eInfo, FILE * log_output = stdout ) : module_name ( module_name ), level ( level ), log_output(log_output) {

	}
	~Logger() = default;

	template<LogLevel LL>
	void log ( const char* fstring, ... ) const {
		if ( LL >= level ) {
			printf ( "%s - ", module_name.c_str() );
			va_list args;
			va_start ( args, fstring );
			vfprintf ( log_output, fstring, args );
			va_end ( args );
			puts ( "" );
		}
	}
	template<LogLevel LL>
	void log_clean ( const char* fstring, ... ) const {
		if ( LL >= level ) {
			va_list args;
			va_start ( args, fstring );
			vfprintf ( log_output, fstring, args );
			va_end ( args );
			puts ( "" );
		}
	}
};

extern const Logger g_logger;
