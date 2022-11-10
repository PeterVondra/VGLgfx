#pragma once

#include <spdlog/spdlog.h>
#include <memory>

namespace vgl
{
	class Logger
	{
		public:
			inline static void init();
			inline static std::shared_ptr<spdlog::logger>& getInternalLogger() { return s_InternalLogger; }
			inline static std::shared_ptr<spdlog::logger>& getClientLogger() { return s_ClientLogger; }

		private:
			static std::shared_ptr<spdlog::logger> s_InternalLogger;
			static std::shared_ptr<spdlog::logger> s_ClientLogger;
	};
}