#pragma once

#include <iostream>
#include <string_view>

#ifdef _WIN32
#include <windows.h>
#endif

namespace Core {

// Enable ANSI escape codes on Windows
inline void EnableConsoleColors() {
#ifdef _WIN32
  static bool initialized = false;
  if (!initialized) {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode = 0;
    GetConsoleMode(hOut, &dwMode);
    SetConsoleMode(hOut, dwMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    initialized = true;
  }
#endif
}

namespace LogColors {
constexpr const char *Reset = "\033[0m";
constexpr const char *Green = "\033[32m";
constexpr const char *Yellow = "\033[33m";
constexpr const char *Red = "\033[31m";
constexpr const char *Cyan = "\033[36m";
} // namespace LogColors

inline void LogInfo(std::string_view msg) {
  EnableConsoleColors();
  std::cout << LogColors::Green << "[INFO] " << LogColors::Reset << msg << '\n';
}

inline void LogWarn(std::string_view msg) {
  EnableConsoleColors();
  std::cout << LogColors::Yellow << "[WARN] " << LogColors::Reset << msg
            << '\n';
}

inline void LogError(std::string_view msg) {
  EnableConsoleColors();
  std::cerr << LogColors::Red << "[ERROR] " << LogColors::Reset << msg << '\n';
}

inline void LogDebug(std::string_view msg) {
  EnableConsoleColors();
  std::cout << LogColors::Cyan << "[DEBUG] " << LogColors::Reset << msg << '\n';
}

} // namespace Core

// Convenience macros
#define LOG_INFO(msg) Core::LogInfo(msg)
#define LOG_WARN(msg) Core::LogWarn(msg)
#define LOG_ERROR(msg) Core::LogError(msg)
#define LOG_DEBUG(msg) Core::LogDebug(msg)
