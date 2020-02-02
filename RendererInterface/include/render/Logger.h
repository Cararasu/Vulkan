#pragma once

#include <cstdio>
#include <cstdarg>
#include "String.h"

enum class LogLevel {
	Trace = 0x1,
	Debug = 0x2,
	Info  = 0x3,
	Warn  = 0x4,
	Error = 0x5,
};

struct Logger {
	const String module_name;
	LogLevel level;
	FILE * log_output;

	Logger ( const String& module_name, LogLevel level = LogLevel::Info, FILE * log_output = stdout ) : module_name ( module_name ), level ( level ), log_output(log_output) {

	}
	Logger ( const String&& module_name, LogLevel level = LogLevel::Info, FILE * log_output = stdout ) : module_name ( module_name ), level ( level ), log_output(log_output) {

	}
	~Logger() = default;
	
	template<LogLevel LL>
	bool is_enabled() const {
		return LL >= level;
	}
	
	template<LogLevel LL>
	void log ( const char* fstring, ... ) const {
		if ( LL >= level ) {
			printf ( "%s - ", module_name.cstr );
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
		}
	}
};