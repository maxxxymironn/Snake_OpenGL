#pragma once

class Logger {
    Logger() = default;
    ~Logger() = default;
public:
    Logger(const Logger& other) = delete;
    Logger& operator=(Logger& other) = delete;

    static Logger& getInstance() {
        static Logger instance;
        return instance;
    }

    void printError(const char* name, const char* description);
    void printInfo(const char* name, const char* description);
};
