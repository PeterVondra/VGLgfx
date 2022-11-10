#pragma once

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <mutex>

#ifdef _WIN32
	#include<Windows.h>
#endif //_WIN32

namespace Utils
{
	template<typename T>
	std::string to_string(T val)
	{
		std::stringstream ss;
		ss << val;
		std::string str = ss.str();
		return str;
	}

	//===============================================================================
	//PROGRESS BAR
	//===============================================================================
	class ProgressBar
	{
		public:
			ProgressBar(const unsigned int whole);
			ProgressBar(const unsigned int size, const unsigned int whole);

			void finish();
			void start(const unsigned int current);

			double getTime();

		protected:
		private:
			unsigned int       m_Size;
			unsigned int      m_Whole;
			unsigned int    m_Current;
			unsigned int       m_Time;
			unsigned int            i;
			std::string    m_Progress;
		};

		class TextAttrib
		{
			//If OS is unix we use escape sequences for text color, thats why std::string is used.
#ifdef __unix__
		public:
			TextAttrib() : code("") {};
			TextAttrib(const std::string p_Code) : code(p_Code) {};
		private:
			friend class Logger;
			friend class ResultDetails;

			std::string code;
#endif //__UNIX__

			//If OS is windows we have to use a different method for text color, thats why integer code is needed.
#ifdef _WIN32
		public:
			TextAttrib() : code(0) {};
			TextAttrib(const int p_Code) : code(p_Code) {};
		private:
			friend class Logger;
			friend class ResultDetails;

			int code;
#endif //_WIN32
		};
		struct SeverityDetails
		{
		public:
			SeverityDetails(TextAttrib p_Code, const char* p_Name) : code(p_Code), name(p_Name) {};

		private:
			friend class Logger;

			const TextAttrib code;
			const char* name;
		};

		struct Foreground_Color
		{
			//Pre declared colors 
			static const TextAttrib Red;
			static const TextAttrib Green;
			static const TextAttrib Yellow;
			static const TextAttrib Blue;
			static const TextAttrib Magenta;
			static const TextAttrib Cyan;
			static const TextAttrib White;
			static const TextAttrib Black;

		};
		struct Background_Color
		{
			//Pre declared colors
			static const TextAttrib Red;
			static const TextAttrib Green;
			static const TextAttrib Yellow;
			static const TextAttrib Blue;
			static const TextAttrib Magenta;
			static const TextAttrib Cyan;
			static const TextAttrib White;
			static const TextAttrib Black;

		};
		struct Attribute
		{
			//Pre declared text attributes
			static const TextAttrib Dim;

#ifdef __unix__
			static const TextAttrib Default;
			static const TextAttrib Bold;
			static const TextAttrib Underlined;
			static const TextAttrib Blink;
			static const TextAttrib Inverted;
			static const TextAttrib Hidden;
#endif //__UNIX__
		};

	struct Severity
	{
		static const SeverityDetails Trace;
		static const SeverityDetails Debug;
		static const SeverityDetails Info;
		static const SeverityDetails Warning;
		static const SeverityDetails Error;
		static const SeverityDetails Fatal;
	};

	class ResultDetails : public TextAttrib
	{
		public:
			ResultDetails(TextAttrib p_Attrib) { *this = p_Attrib; }

			void operator=(TextAttrib& p_Attrib)
			{
				code = p_Attrib.code;
			}

			bool operator==(ResultDetails& p_Details)
			{
				return code == p_Details.code;
			}
			bool operator==(const ResultDetails& p_Details)
			{
				return code == p_Details.code;
			}
	};

	struct Result
	{
		static const ResultDetails Success;
		static const ResultDetails Failed;
	};

	//===============================================================================
	//LOGGER
	//===============================================================================
	class Logger
	{
		public:
			Logger();
			Logger(std::string p_FileName);
			Logger(const char* p_FileName);
			~Logger();

			static void setStartTimePoint(std::chrono::steady_clock::time_point p_StartTime);
			static float getTimePoint();

			static void setLogFile(std::string p_FileName);
			static void setLogFile(const char* p_FileName);

			//write to log file
			static void logMSG(const std::string p_Message);
			static void logMSG(const char* p_Message);
			static void logMSG(const std::string p_Message, const char* p_MessageTitle, SeverityDetails p_Severity);
			static void logMSG(const char* p_Message, const char* p_MessageTitle, SeverityDetails p_Severity);
			static void logMSG(const std::string p_Messagge, const char* p_MessageTitle, ResultDetails p_Result);

			static const char* set(TextAttrib p_Attrib);

			static void writeToConsole(std::ostream	p_Content);
			static void writeToConsole(const std::string p_Content);
			static void writeToConsole(const char* p_Content);

			static void writeToConsole(std::ostream	p_Content, TextAttrib p_Attrib);
			static void writeToConsole(const std::string p_Content, TextAttrib p_Attrib);
			static void writeToConsole(const char* p_Content, TextAttrib p_Attrib);

		protected:
		private:
#ifdef _WIN32
			static HANDLE hConsole;
#endif

			static double m_Runtime;
			static std::chrono::steady_clock::time_point startTime;

			static std::string m_Stream;
			static std::mutex m_Mutex;
	};
	class LoggerSingleton
	{
		public:
			LoggerSingleton(LoggerSingleton const&) = delete;
			LoggerSingleton(LoggerSingleton&&) = delete;
			LoggerSingleton& operator=(LoggerSingleton const&) = delete;
			LoggerSingleton& operator=(LoggerSingleton&&) = delete;

			static Logger& getInstance()
			{
				static Logger instance("log.txt");

				return instance;
			}

		private:
			LoggerSingleton() {};
			~LoggerSingleton() {};

	};
}
