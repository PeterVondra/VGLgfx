#include "VGL_Logger.h"
#include <spdlog/sinks/stdout_color_sinks.h>

namespace vgl
{
	std::shared_ptr<spdlog::logger> Logger::s_InternalLogger;
	std::shared_ptr<spdlog::logger> Logger::s_ClientLogger;

	void Logger::init()
	{
		spdlog::set_pattern("%^[%T] [thread %t] %l %n: %v%$");
		
		s_InternalLogger = spdlog::stdout_color_mt("VGLgfx");
		s_InternalLogger->set_level((spdlog::level::level_enum)VGL_INTERNAL_LOG_LEVEL);
		
		s_ClientLogger = spdlog::stdout_color_mt("VGLgfx_APP");
		s_ClientLogger->set_level((spdlog::level::level_enum)VGL_CLIENT_LOG_LEVEL);
	}
}