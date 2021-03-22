#include "Logger.h"

#include <chrono>
#include <iostream>
#include <string>

shared_ptr<Logger> Logger::logger = nullptr;

shared_ptr<Logger> Logger::getInstance() {
    if (logger == nullptr) {
        logger = shared_ptr<Logger>(new Logger());
    }
    return logger;
}

void Logger::setTargetLogLevel(LogLevel newTargetLevel) {
    targetLevel = newTargetLevel;
}

void Logger::setOutputMode(bool newLongOutput) { longOutput = newLongOutput; }

void addAfterAll(std::string&       data,
                 const std::string& toSearch,
                 const std::string& replaceStr) {
    size_t pos = data.find(toSearch);

    while (pos != std::string::npos) {
        data.insert(pos + 1, replaceStr);
        pos = data.find(toSearch, pos + 1 + replaceStr.size());
    }
}

void Logger::log(LogLevel level, string message) {
    if (level >= targetLevel) {
        string output;

        switch (level) {
            case LogLevel::DEBUG:
                output += COLOR_F_L_GREY;
                break;
            case LogLevel::VERBOSE:
                output += COLOR_F_L_GREY;
                break;
            case LogLevel::SUCCESS:
                output += COLOR_F_L_GREEN;
                break;
            case LogLevel::WARNING:
                output += COLOR_F_L_YELLOW;
                break;
            case LogLevel::ERROR:
            case LogLevel::FATAL:
                output += COLOR_F_L_RED;
                break;
        }

        if (longOutput) {
            output += levelMapLong.at(level);
            addAfterAll(message, "\n", padLong);
        } else {
            output += levelMapShort.at(level);
            addAfterAll(message, "\n", padShort);
        }

        switch (level) {
            case LogLevel::VERBOSE:
            case LogLevel::SUCCESS:
            case LogLevel::WARNING:
            case LogLevel::ERROR:
                output += COLOR_F_DEFAULT;
                output += message;
                break;
            case LogLevel::DEBUG:
            case LogLevel::FATAL:
                output += message;
                output += COLOR_F_DEFAULT;
                break;
            case LogLevel::INFO:
                output += message;
                break;
        }


        switch (level) {}

        cout << output << endl;
        //        cout << chrono::steady_clock::now().time_since_epoch().count()
        //        << endl;
    }
}
