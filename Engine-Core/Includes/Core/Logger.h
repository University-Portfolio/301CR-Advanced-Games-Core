#pragma once
#include "CoreDefs.h"
#include <string>

#define LOG(message, ...) { \
	char fmsg[256]; \
	snprintf(fmsg, sizeof(fmsg), message, __VA_ARGS__); \
	Logger::LogMessage(fmsg); \
}

#ifdef BUILD_DEBUG
#define LOG_WARNING(message, ...) { \
	char fmsg[256]; \
	snprintf(fmsg, sizeof(fmsg), message, __VA_ARGS__); \
	Logger::LogWarning(fmsg, __FILE__, __LINE__); \
}

#define LOG_ERROR(message, ...) { \
	char fmsg[256]; \
	snprintf(fmsg, sizeof(fmsg), message, __VA_ARGS__); \
	Logger::LogError(fmsg, __FILE__, __LINE__); \
}
#else
#define LOG_WARNING(message, ...) { \
	char fmsg[256]; \
	snprintf(fmsg, sizeof(fmsg), message, __VA_ARGS__); \
	Logger::LogWarning(fmsg); \
}

#define LOG_ERROR(message, ...) { \
	char fmsg[256]; \
	snprintf(fmsg, sizeof(fmsg), message, __VA_ARGS__); \
	Logger::LogError(fmsg); \
}
#endif

namespace EngineCore
{
	/**
	* Centralized logging class
	* NOTE: All text output should be through this class (preferably by use of LOG(_XYZ) macros)
	*/
	class CORE_API Logger
	{
	public:
		static void LogMessage(std::string msg);

#ifdef BUILD_DEBUG
		static void LogWarning(std::string msg, std::string file, int line);
		static void LogError(std::string msg, std::string file, int line);
#else
		static void LogWarning(std::string msg);
		static void LogError(std::string msg);
#endif
	};
}
