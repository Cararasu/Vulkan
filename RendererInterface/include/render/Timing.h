#pragma once

#include "Logger.h"
#include <chrono>

struct Timing {
	std::chrono::time_point<std::chrono::high_resolution_clock> starttime;
	const Logger* logger;
	const char* message;
	
	Timing(const Logger* logger, const char* message) : starttime(std::chrono::high_resolution_clock::now()), logger(logger), message(message) {
		logger->log<LogLevel::eInfo>("%s - Starting", message);
	}
	~Timing(){
		double time = std::chrono::duration_cast<std::chrono::nanoseconds> ( std::chrono::high_resolution_clock::now() - starttime ).count() / 1000000.0;
		logger->log<LogLevel::eInfo>("%s - Finished - Elapsed: %lfms", message, time);
	}
	
	void timestamp(const char* sub_message) {
		double time = std::chrono::duration_cast<std::chrono::nanoseconds> ( std::chrono::high_resolution_clock::now() - starttime ).count() / 1000000.0;
		logger->log<LogLevel::eInfo>("%s - %s - Elapsed: %lfms", message, sub_message, time);
	}
};
