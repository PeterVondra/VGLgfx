#include "Logger.h"

#include <iomanip>

namespace Utils
{
	//===============================================================================
	//TEXT ATTRIBUTE DEFINITIONS
	//===============================================================================

	//Define the text attributes
#ifdef __unix__
	const TextAttrib Attribute::Default = TextAttrib("\u001b[0m");
	const TextAttrib Attribute::Bold = TextAttrib("\e[1m");
	const TextAttrib Attribute::Dim = TextAttrib("\e[2m");
	const TextAttrib Attribute::Underlined = TextAttrib("\e[4m");
	const TextAttrib Attribute::Blink = TextAttrib("\e[5m");
	const TextAttrib Attribute::Inverted = TextAttrib("\e[7m");
	const TextAttrib Attribute::Hidden = TextAttrib("\e[8m");

	//Define the text colors
	const TextAttrib Foreground_Color::Red = TextAttrib("\e[41m");
	const TextAttrib Foreground_Color::Green = TextAttrib("\e[42m");
	const TextAttrib Foreground_Color::Yellow = TextAttrib("\e[43m");
	const TextAttrib Foreground_Color::Blue = TextAttrib("\e[44m");
	const TextAttrib Foreground_Color::Magenta = TextAttrib("\e[45m");
	const TextAttrib Foreground_Color::Cyan = TextAttrib("\e[46m");
	const TextAttrib Foreground_Color::White = TextAttrib("\e[107m");
	const TextAttrib Foreground_Color::Black = TextAttrib("\e[40m");

	//Define the background colors
	const TextAttrib Background_Color::Red = TextAttrib("\e[41m");
	const TextAttrib Background_Color::Green = TextAttrib("\e[42m");
	const TextAttrib Background_Color::Yellow = TextAttrib("\e[43m");
	const TextAttrib Background_Color::Blue = TextAttrib("\e[44m");
	const TextAttrib Background_Color::Magenta = TextAttrib("\e[45m");
	const TextAttrib Background_Color::Cyan = TextAttrib("\e[46m");
	const TextAttrib Background_Color::White = TextAttrib("\e[107m");
	const TextAttrib Background_Color::Black = TextAttrib("\e[40m");
#endif //__UNIX__

#ifdef _WIN32

	namespace
	{
		//macros not defined by Windows.h
		#define FOREGROUND_YELLOW 6
		#define FOREGROUND_MAGENTA 5
		#define FOREGROUND_CYAN 11
		#define FOREGROUND_WHITE 15
		#define FOREGROUND_BLACK 0
		
		#define BACKGROUND_YELLOW 367
		#define BACKGROUND_MAGENTA 351
		#define BACKGROUND_CYAN 447
		#define BACKGROUND_WHITE 240
		#define BACKGROUND_BLACK 271
	}

	const TextAttrib Foreground_Color::Red = TextAttrib(FOREGROUND_RED);
	const TextAttrib Foreground_Color::Green = TextAttrib(FOREGROUND_GREEN);
	const TextAttrib Foreground_Color::Yellow = TextAttrib(FOREGROUND_YELLOW);
	const TextAttrib Foreground_Color::Blue = TextAttrib(FOREGROUND_BLUE);
	const TextAttrib Foreground_Color::Magenta = TextAttrib(FOREGROUND_MAGENTA);
	const TextAttrib Foreground_Color::Cyan = TextAttrib(FOREGROUND_CYAN);
	const TextAttrib Foreground_Color::White = TextAttrib(FOREGROUND_WHITE);
	const TextAttrib Foreground_Color::Black = TextAttrib(FOREGROUND_BLACK);

	const TextAttrib Attribute::Dim = TextAttrib(FOREGROUND_INTENSITY);

	const TextAttrib Background_Color::Red = TextAttrib(BACKGROUND_RED);
	const TextAttrib Background_Color::Green = TextAttrib(BACKGROUND_GREEN);
	const TextAttrib Background_Color::Yellow = TextAttrib(BACKGROUND_YELLOW);
	const TextAttrib Background_Color::Blue = TextAttrib(BACKGROUND_BLUE);
	const TextAttrib Background_Color::Magenta = TextAttrib(BACKGROUND_MAGENTA);
	const TextAttrib Background_Color::Cyan = TextAttrib(BACKGROUND_CYAN);
	const TextAttrib Background_Color::White = TextAttrib(BACKGROUND_WHITE);
	const TextAttrib Background_Color::Black = TextAttrib(BACKGROUND_BLACK);
#endif //_WIN32

	const SeverityDetails Severity::Trace = SeverityDetails(Attribute::Dim, "Trace:    ");
	const SeverityDetails Severity::Debug = SeverityDetails(Foreground_Color::Magenta, "Debug:    ");
	const SeverityDetails Severity::Info = SeverityDetails(Foreground_Color::Green, "Info:    ");
	const SeverityDetails Severity::Warning = SeverityDetails(Foreground_Color::Yellow, "Warning:    ");
	const SeverityDetails Severity::Error = SeverityDetails(Foreground_Color::Red, "Error:    ");
	const SeverityDetails Severity::Fatal = SeverityDetails(Background_Color::Red, "Fatal:    ");

	const ResultDetails Result::Success = Foreground_Color::Green;
	const ResultDetails Result::Failed = Foreground_Color::Red;

#ifdef _WIN32
	HANDLE Logger::hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
#endif

	//===============================================================================
	//LOGGER
	//===============================================================================

	std::string Logger::m_Stream;
	double Logger::m_Runtime;
	std::chrono::steady_clock::time_point Logger::startTime;

	Logger::Logger()
	{

	}
	Logger::Logger(const std::string p_FileName)
	{
		std::ofstream file;
		file.open(p_FileName);

		if (!file.is_open())
			logMSG("Failed to open: " + p_FileName + ", creating new file", "Log File", Severity::Warning);

		file << m_Stream << std::endl;

		file.close();
	}
	Logger::Logger(const char* p_FileName)
	{
		std::ofstream file;
		file.open(p_FileName);

		if (!file.is_open())
			logMSG("Failed to open: " + std::string(p_FileName) + ", creating new file", "Log File", Severity::Warning);

		file << m_Stream << std::endl;

		file.close();
	}
	Logger::~Logger()
	{

	}

	void Logger::setStartTimePoint(std::chrono::steady_clock::time_point p_StartTime)
	{
		startTime = p_StartTime;
	}

	float Logger::getTimePoint()
	{
		return std::chrono::duration<float, std::ratio<1, 1>>(std::chrono::steady_clock::now() - startTime).count();
	}

	void Logger::setLogFile(std::string p_FileName)
	{
		std::ofstream file;
		file.open(p_FileName);

		if (!file.is_open())
			logMSG("Failed to open: " + p_FileName + ", creating new file", "Log File", Severity::Warning);

		file << m_Stream << std::endl;

		file.close();
	}
	void Logger::setLogFile(const char* p_FileName)
	{
		std::ofstream file;
		file.open(p_FileName);
		if (!file.is_open())
			logMSG("Failed to open: " + std::string(p_FileName) + ", creating new file", "Log File", Severity::Warning);

		file << m_Stream << std::endl;

		file.close();
	}

	//===============================================================================
	//WRITE TO CONSOLE
	//===============================================================================
	const char* Logger::set(TextAttrib p_Attrib)
	{
#ifdef __UNIX__
		std::cout << p_Attrib;
		return "";
#endif //__UNIX__

#ifdef _WIN32
		SetConsoleTextAttribute(hConsole, p_Attrib.code);
		return "";
#endif //_WIN32
	}
	void Logger::writeToConsole(std::ostream p_Content)
	{
		std::stringstream ss;
		ss << p_Content.rdbuf();
		std::cout << ss.str();
	}
	void Logger::writeToConsole(const std::string p_Content)
	{
		std::cout << p_Content;
	}
	void Logger::writeToConsole(const char* p_Content)
	{
		std::cout << p_Content;
	}

	void Logger::writeToConsole(std::ostream p_Content, TextAttrib p_Attrib)
	{
		std::stringstream ss;
		ss << p_Content.rdbuf();
		std::cout << set(p_Attrib) << ss.str();
	}
	void Logger::writeToConsole(const std::string p_Content, TextAttrib	p_Attrib)
	{
		std::cout << set(p_Attrib) << p_Content;
	}
	void Logger::writeToConsole(const char* p_Content, TextAttrib p_Attrib)
	{
		std::cout << set(p_Attrib) << p_Content;
	}

	void Logger::logMSG(const std::string p_Content)
	{
		writeToConsole(p_Content);
		m_Stream += p_Content;
	}

	void Logger::logMSG(const char* p_Content)
	{
		writeToConsole(p_Content);
		m_Stream += p_Content;
	}

	//write to log file
	void Logger::logMSG(const std::string p_Content, const char* p_Title, SeverityDetails p_Severity)
	{
		m_Runtime = std::chrono::duration<float, std::ratio<1, 1>>(std::chrono::steady_clock::now() - startTime).count();
		std::string content = "[" + std::string(p_Title) + "]" + std::string(p_Severity.name) + p_Content;

		std::cout << set(p_Severity.code) << std::fixed << std::setprecision(5) << "[Run time : " << m_Runtime << "s]" << content << set(Foreground_Color::White) << "\n";
		m_Stream += content + "\n";
	}
	void Logger::logMSG(const char* p_Content, const char* p_Title, SeverityDetails p_Severity)
	{
		m_Runtime = std::chrono::duration<float, std::ratio<1, 1>>(std::chrono::steady_clock::now() - startTime).count();
		std::string content = "[" + std::string(p_Title) + "]" + std::string(p_Severity.name) + std::string(p_Content);

		std::cout << set(p_Severity.code) << std::fixed << std::setprecision(5) << "[Run time : " << m_Runtime << "s]" << content << set(Foreground_Color::White) << "\n";
		m_Stream += content + "\n";
	}
	void Logger::logMSG(const std::string p_Content, const char* p_Title, ResultDetails p_Result)
	{
		if (p_Result == Result::Success)
		{
			m_Runtime = std::chrono::duration<float, std::ratio<1, 1>>(std::chrono::steady_clock::now() - startTime).count();
			std::string content = "[" + std::string(p_Title) + "]" + std::string(p_Content);

			std::cout << set(Foreground_Color::White) <<
				std::fixed << std::setprecision(5) << "[Run time : " << m_Runtime << "s]" << content << "\t[" <<
				set(p_Result.code) << "SUCCESS" <<
				set(Foreground_Color::White) << "]\n";

			m_Stream += content + "\n";
		}
		else if (p_Result == Result::Failed)
		{
			m_Runtime = std::chrono::duration<float, std::ratio<1, 1>>(std::chrono::steady_clock::now() - startTime).count();
			std::string content = "[" + std::string(p_Title) + "]" + std::string(p_Content);

			std::cout << set(Foreground_Color::White) <<
				std::fixed << std::setprecision(5) << "[Run time : " << m_Runtime << "s]" << content << "\t[" <<
				set(p_Result.code) << "FAILED" <<
				set(Foreground_Color::White) << "]\n";

			m_Stream += content + "\n";
		}
	}

	//===============================================================================
	//FILE I/O
	//===============================================================================
	std::vector<char> readFile(const std::string& filename)
	{
		std::ifstream file(filename, std::ios::ate | std::ios::binary);

		if (!file.is_open())
			throw std::runtime_error("failed to open file");

		size_t fileSize = (size_t)file.tellg();
		std::vector<char> buffer(fileSize);

		file.seekg(0);
		file.read(buffer.data(), fileSize);

		file.close();

		return buffer;
	}
	void writeFile(const std::string& filename, const std::string& writeData)
	{
		std::ofstream file;
		file.open(filename);
		if (!file.is_open())
			throw std::runtime_error("failed to open file");

		file << writeData << std::endl;

		file.close();
	}

	//===============================================================================
	//PROGRESS BAR
	//===============================================================================

	ProgressBar::ProgressBar(const unsigned int whole)
	{
		m_Progress = "[";
		m_Whole = whole;
		m_Size = 50;
		i = 0;
	}
	ProgressBar::ProgressBar(const unsigned int size, const unsigned int whole)
	{
		m_Progress = "[";
		m_Whole = whole;
		m_Size = size;
		i = 0;
	}

	void ProgressBar::finish()
	{
		if (m_Current < m_Whole)
			m_Current = m_Whole;
		if (m_Current > m_Whole)
			m_Current = m_Whole;

		for (int j = i; j * (100 / m_Size) <= 100; j++)
		{
			m_Progress += "=";
		}
		Logger::writeToConsole("\rprogress: ", Foreground_Color::Green);
		Logger::writeToConsole(Utils::to_string(m_Progress) + ">");
		i++;
		fflush(stdout);
		if (i * (100 / m_Size) > 100)
			i = m_Size;
		if (i * (100 / m_Size) < 100)
			i = m_Size;
		Logger::writeToConsole("] " + Utils::to_string(i * (100 / m_Size)) + "%");
		Logger::writeToConsole(" completed\n", Foreground_Color::Green);
	}
	void ProgressBar::start(const unsigned int current)
	{
		m_Current = current;
		if (m_Current > m_Whole)
			m_Current = m_Whole;


		if (m_Current >= (double(m_Whole) / m_Size) * i)
		{
			m_Progress += "=";
			Logger::writeToConsole("progress: " + Utils::to_string(m_Progress) + ">" + Utils::to_string(i * (100 / m_Size)) + "%" + "\r", Foreground_Color::Green);
			i++;
		}
	}

	double ProgressBar::getTime()
	{
		return m_Time;
	}

}
