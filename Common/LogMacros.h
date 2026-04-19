#pragma once

#include "Logger.h"

#define LOG_TRACE(...)   Logger::getInstance().trace(__FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)
#define LOG_DEBUG(...)   Logger::getInstance().debug(__FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)
#define LOG_INFO(...)    Logger::getInstance().info(__FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)
#define LOG_WARN(...)    Logger::getInstance().warn(__FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)
#define LOG_ERROR(...)   Logger::getInstance().error(__FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)
#define LOG_CRITICAL(...) Logger::getInstance().critical(__FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)

#ifdef ENABLE_LOGGING
#define LOG_IF_TRACE(condition, ...)   if(condition) LOG_TRACE(__VA_ARGS__)
#define LOG_IF_DEBUG(condition, ...)   if(condition) LOG_DEBUG(__VA_ARGS__)
#define LOG_IF_INFO(condition, ...)    if(condition) LOG_INFO(__VA_ARGS__)
#else
#define LOG_IF_TRACE(condition, ...)   ((void)0)
#define LOG_IF_DEBUG(condition, ...)   ((void)0)
#define LOG_IF_INFO(condition, ...)    ((void)0)
#endif