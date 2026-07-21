#include "logger.hpp"
#include <iostream>

void Logger::printError(const char* name, const char* description) {
    std::cerr << "Snake_OpenGL::ERROR::" << name << "::" << description << "\n";
}

void Logger::printInfo(const char* name, const char* description) {
    std::cout << "Snake_OpenGL::INFO::" << name << "::" << description << "\n";
}