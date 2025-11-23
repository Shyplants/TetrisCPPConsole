#define _CRT_SECURE_NO_WARNINGS

#include "Logger.h"
#include <chrono>
#include <ctime>
#include <Windows.h>
#include <iostream>
#include "Colors.h"

void Logger::Log(const std::string_view message)
{
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hConsole, GREEN);
	std::cout << "LOG: " << CurrentDate() << " - " << message << "\n";
	SetConsoleTextAttribute(hConsole, WHITE);
}

void Logger::Error(const std::string& message, const char* file, const char* function, int line)
{
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hConsole, RED);
	std::cout << "ERROR: " << CurrentDate() << " - " << message << "\nFILE: " << file << "\nFUNC: " << function << "\nLINE: " << line << "\n\n";
	SetConsoleTextAttribute(hConsole, WHITE);
}

std::string Logger::CurrentDate()
{
	std::time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

	std::string output(30, '\0');
	std::strftime(&output[0], output.size(), "%y-%m-%d %H:%M:%S", std::localtime(&now));

	return output;
}
