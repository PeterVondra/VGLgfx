#include "VGL_Logger.h"
#include <spdlog/sinks/stdout_color_sinks.h>

namespace vgl
{
	std::shared_ptr<spdlog::logger> Logger::s_InternalLogger;
	std::shared_ptr<spdlog::logger> Logger::s_ClientLogger;

	void Logger::init()
	{
		spdlog::set_pattern("%^^[%T] %n: %v%$");
		spdlog::set_pattern("*** [%H:%M:%S %z] [thread %t] %l %v ***");
		
		s_InternalLogger = spdlog::stdout_color_mt("VGLgfx");
		s_InternalLogger->set_level(spdlog::level::trace);
		
		s_ClientLogger = spdlog::stdout_color_mt("VGLgfx_APP");
		s_ClientLogger->set_level(spdlog::level::trace);

	}
}

// Internal logging macros
#define VGL_INTERNAL_FATAL(...)		::vgl::Logger::getInternalLogger()->fatal(__VA_ARG__)
#define VGL_INTERNAL_ERROR(...)		::vgl::Logger::getInternalLogger()->error(__VA_ARG__)
#define VGL_INTERNAL_WARNING(...)	::vgl::Logger::getInternalLogger()->warn(__VA_ARG__)
#define VGL_INTERNAL_INFO(...)		::vgl::Logger::getInternalLogger()->info(__VA_ARG__)
#define VGL_INTERNAL_TRACE(...)		::vgl::Logger::getInternalLogger()->trace(__VA_ARG__)

// Client logging macros
#define VGL_FATAL(...)		::vgl::Logger::getClientLogger()->fatal(__VA_ARG__)
#define VGL_ERROR(...)		::vgl::Logger::getClientLogger()->error(__VA_ARG__)
#define VGL_WARNING(...)	::vgl::Logger::getClientLogger()->warn(__VA_ARG__)
#define VGL_INFO(...)		::vgl::Logger::getClientLogger()->info(__VA_ARG__)
#define VGL_TRACE(...)		::vgl::Logger::getClientLogger()->trace(__VA_ARG__)