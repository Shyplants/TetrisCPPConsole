#pragma once

#include <string_view>

#define TETRIS_LOG(x) Logger::Log(x);
#define TETRIS_ERROR(x) Logger::Error(x, __FILE__, __FUNCTION__, __LINE__);

class Logger
{
public:
	Logger() {};
	~Logger() = default;

	static void Log(const std::string_view message);
	static void Error(const std::string& message, const char* file, const char* function, int line);

private:
	static std::string CurrentDate();
};