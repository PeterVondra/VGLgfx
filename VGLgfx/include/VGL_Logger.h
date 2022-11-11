#pragma once

#pragma warning(push, 0)
#include <spdlog/spdlog.h>
#include <spdlog/fmt/bundled/printf.h>
#pragma warning(pop)

#include "../libs/PPK_ASSERT/ppk_assert.h"

#include <memory>

namespace vgl
{
	enum class Level
	{
		Off			= spdlog::level::off,
		Fatal		= spdlog::level::critical,
		Error		= spdlog::level::err,
		Warning		= spdlog::level::warn,
		Info		= spdlog::level::info,
		Debug		= spdlog::level::debug,
		Trace		= spdlog::level::trace
	};
	class Logger
	{
		public:
			inline static void init();
			inline static std::shared_ptr<spdlog::logger>& getInternalLogger() { return s_InternalLogger; }
			inline static std::shared_ptr<spdlog::logger>& getClientLogger() { return s_ClientLogger; }

			inline static void setInternalLogLevel(spdlog::level::level_enum p_Level) { s_InternalLogger->set_level(p_Level); }
			inline static void setClientLogLevel(spdlog::level::level_enum p_Level) { s_ClientLogger->set_level(p_Level); }

		private:
			static std::shared_ptr<spdlog::logger> s_InternalLogger;
			static std::shared_ptr<spdlog::logger> s_ClientLogger;
	};
	template <class LoggerPtr, class... Args>
	void spdlogCStyleFormat(
		LoggerPtr p_Logger,
		spdlog::level::level_enum p_Level,
		spdlog::source_loc p_Loc,
		const char* fmt,
		const Args&... args) noexcept
	{
		if (p_Logger && p_Logger->should_log(p_Level))
			p_Logger->log(p_Loc, p_Level, "{}", fmt::sprintf(fmt, args...));
	}
	template <class LoggerPtr, class... Args>
	void spdlogCStyleFormat(
		LoggerPtr p_Logger,
		spdlog::level::level_enum p_Level,
		spdlog::source_loc p_Loc,
		std::string fmt,
		const Args&... args) noexcept
	{
		if (p_Logger && p_Logger->should_log(p_Level))
			p_Logger->log(p_Loc, p_Level, "{}", fmt::sprintf(fmt.c_str(), args...));
	}

#define SPDLOG_LOGGER_PRINTF(p_Logger, p_Level, ...) \
    spdlogCStyleFormat(p_Logger, p_Level, spdlog::source_loc{__FILE__, __LINE__, SPDLOG_FUNCTION}, __VA_ARGS__)
}

#define NOP ((void)0)
#define VGL_INTERNAL_LOG_LEVEL ::vgl::Level::Off
#define VGL_CLIENT_LOG_LEVEL ::vgl::Level::Off

#ifdef VGL_INTERNAL_LOGGING_ENABLED
// Internal logging macros
#ifdef VGL_INTERNAL_LOG_LEVEL_FATAL
#define VGL_INTERNAL_LOG_LEVEL ::vgl::Level::Fatal
#endif
#ifdef VGL_INTERNAL_LOG_LEVEL_ERROR
#define VGL_INTERNAL_LOG_LEVEL ::vgl::Level::Error
#endif
#ifdef VGL_INTERNAL_LOG_LEVEL_WARNING
#define VGL_INTERNAL_LOG_LEVEL ::vgl::Level::Warning
#endif
#ifdef VGL_INTERNAL_LOG_LEVEL_INFO
#define VGL_INTERNAL_LOG_LEVEL ::vgl::Level::Info
#endif
#ifdef VGL_INTERNAL_LOG_LEVEL_DEBUG
#define VGL_INTERNAL_LOG_LEVEL ::vgl::Level::Debug
#endif
#ifdef VGL_INTERNAL_LOG_LEVEL_TRACE
#define VGL_INTERNAL_LOG_LEVEL ::vgl::Level::Trace
#endif
#ifdef VGL_INTERNAL_LOG_LEVEL_OFF
#define VGL_INTERNAL_LOG_LEVEL ::vgl::Level::Off
#endif
#endif

#ifdef VGL_INTERNAL_LOGGING_ENABLED
#define VGL_INTERNAL_FATAL(...)		::vgl::SPDLOG_LOGGER_PRINTF(::vgl::Logger::getInternalLogger(), spdlog::level::critical, __VA_ARGS__)
#define VGL_INTERNAL_ERROR(...)		::vgl::SPDLOG_LOGGER_PRINTF(::vgl::Logger::getInternalLogger(), spdlog::level::err, __VA_ARGS__)
#define VGL_INTERNAL_WARNING(...)	::vgl::SPDLOG_LOGGER_PRINTF(::vgl::Logger::getInternalLogger(), spdlog::level::warn, __VA_ARGS__)
#define VGL_INTERNAL_INFO(...)		::vgl::SPDLOG_LOGGER_PRINTF(::vgl::Logger::getInternalLogger(), spdlog::level::info, __VA_ARGS__)
#define VGL_INTERNAL_DEBUG(...)		::vgl::SPDLOG_LOGGER_PRINTF(::vgl::Logger::getInternalLogger(), spdlog::level::debug, __VA_ARGS__)
#define VGL_INTERNAL_TRACE(...)		::vgl::SPDLOG_LOGGER_PRINTF(::vgl::Logger::getInternalLogger(), spdlog::level::trace, __VA_ARGS__)
#else
#define VGL_INTERNAL_FATAL(...)		NOP
#define VGL_INTERNAL_ERROR(...)		NOP
#define VGL_INTERNAL_WARNING(...)	NOP
#define VGL_INTERNAL_INFO(...)		NOP
#define VGL_INTERNAL_DEBUG(...)		NOP
#define VGL_INTERNAL_TRACE(...)		NOP
#endif

// Client logging macros
#ifdef VGL_LOGGING_ENABLED
// Internal logging macros
#ifdef VGL_INTERNAL_LOG_LEVEL_FATAL
#define VGL_LOG_LEVEL ::vgl::Level::Fatal
#endif
#ifdef VGL_LOG_LEVEL_ERROR
#define VGL_LOG_LEVEL ::vgl::Level::Error
#endif
#ifdef VGL_LOG_LEVEL_WARNING
#define VGL_LOG_LEVEL ::vgl::Level::Warning
#endif
#ifdef VGL_LOG_LEVEL_INFO
#define VGL_LOG_LEVEL ::vgl::Level::Info
#endif
#ifdef VGL_LOG_LEVEL_DEBUG
#define VGL_LOG_LEVEL ::vgl::Level::Debug
#endif
#ifdef VGL_LOG_LEVEL_TRACE
#define VGL_LOG_LEVEL ::vgl::Level::Trace
#endif
#ifdef VGL_LOG_LEVEL_OFF
#define VGL_LOG_LEVEL ::vgl::Level::Off
#endif
#endif

#ifdef VGL_LOGGING_ENABLED
#define VGL_FATAL(...)		::vgl::SPDLOG_LOGGER_PRINTF(::vgl::Logger::getClientLogger(), spdlog::level::critical, __VA_ARGS__)
#define VGL_ERROR(...)		::vgl::SPDLOG_LOGGER_PRINTF(::vgl::Logger::getClientLogger(), spdlog::level::err, __VA_ARGS__)
#define VGL_WARNING(...)	::vgl::SPDLOG_LOGGER_PRINTF(::vgl::Logger::getClientLogger(), spdlog::level::warn, __VA_ARGS__)
#define VGL_INFO(...)		::vgl::SPDLOG_LOGGER_PRINTF(::vgl::Logger::getClientLogger(), spdlog::level::info, __VA_ARGS__)
#define VGL_DEBUG(...)		::vgl::SPDLOG_LOGGER_PRINTF(::vgl::Logger::getClientLogger(), spdlog::level::debug, __VA_ARGS__)
#define VGL_TRACE(...)		::vgl::SPDLOG_LOGGER_PRINTF(::vgl::Logger::getClientLogger(), spdlog::level::trace, __VA_ARGS__)
#else
#define VGL_FATAL(...)		NOP
#define VGL_ERROR(...)		NOP
#define VGL_WARNING(...)	NOP
#define VGL_INFO(...)		NOP
#define VGL_DEBUG(...)		NOP
#define VGL_TRACE(...)		NOP
#endif

#ifdef VGL_INTERNAL_ASSERT_ENABLED
#define PPK_ASSERT_ENABLED 1
#ifdef VGL_INTERNAL_ASSERT_LEVEL_FATAL
#define VGL_INTERNAL_ASSERT_FATAL(...)		PPK_ASSERT_FATAL(__VA_ARGS__)
#else VGL_INTERNAL_ASSERT_FATAL(...)		NOP
#endif
#ifdef VGL_INTERNAL_ASSERT_LEVEL_ERROR
#define VGL_INTERNAL_ASSERT_ERROR(...)		PPK_ASSERT_ERROR(__VA_ARGS__)
#else VGL_INTERNAL_ASSERT_ERROR(...)		NOP
#endif
#ifdef VGL_INTERNAL_ASSERT_LEVEL_WARNING
#define VGL_INTERNAL_ASSERT_WARNING(...)	PPK_ASSERT_WARNING(__VA_ARGS__)
#else VGL_INTERNAL_ASSERT_WARNING(...)		NOP
#endif
#ifdef VGL_INTERNAL_ASSERT_LEVEL_DEBUG
#define VGL_INTERNAL_ASSERT_DEBUG(...)		PPK_ASSERT_DEBUG(__VA_ARGS__)
#else VGL_INTERNAL_ASSERT_DEBUG(...)		NOP
#endif
#else
#define PEMPEK_ASSERT_ENABLED 0
#define VGL_INTERNAL_ASSERT_FATAL(...)		NOP
#define VGL_INTERNAL_ASSERT_ERROR(...)		NOP
#define VGL_INTERNAL_ASSERT_WARNING(...)	NOP
#define VGL_INTERNAL_ASSERT_DEBUG(...)		NOP
#endif

#ifdef VGL_ASSERT_ENABLED
#define PEMPEK_ASSERT_ENABLED 1
#ifdef VGL_ASSERT_LEVEL_FATAL
#define VGL_ASSERT_FATAL(...)		PPK_ASSERT_FATAL(__VA_ARGS__)
#else VGL_ASSERT_FATAL(...)			NOP
#endif
#ifdef VGL_ASSERT_LEVEL_ERROR
#define VGL_ASSERT_ERROR(...)		PPK_ASSERT_ERROR(__VA_ARGS__)
#else VGL_ASSERT_ERROR(...)			NOP
#endif
#ifdef VGL_ASSERT_LEVEL_WARNING
#define VGL_ASSERT_WARNING(...)		PPK_ASSERT_WARNING(__VA_ARGS__)
#else VGL_ASSERT_WARNING(...)		NOP
#endif
#ifdef VGL_ASSERT_LEVEL_DEBUG
#define VGL_ASSERT_DEBUG(...)		PPK_ASSERT_DEBUG(__VA_ARGS__)
#else VGL_ASSERT_DEBUG(...)			NOP
#endif
#else
#define PEMPEK_ASSERT_ENABLED 0
#define VGL_ASSERT_FATAL(...)		NOP
#define VGL_ASSERT_ERROR(...)		NOP
#define VGL_ASSERT_WARNING(...)		NOP
#define VGL_ASSERT_DEBUG(...)		NOP
#endif