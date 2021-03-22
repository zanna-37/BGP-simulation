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

void Logger::setEnableColor(bool enableColor) {
    this->enableColor = enableColor;
}

void Logger::setLongPrefix(bool longPrefix) { this->longPrefix = longPrefix; }

void addAfterAll(std::string&       data,
                 const std::string& toSearch,
                 const std::string& replaceStr) {
    size_t pos = data.find(toSearch);

    while (pos != std::string::npos) {
        data.insert(pos + 1, replaceStr);
        pos = data.find(toSearch, pos + 1 + replaceStr.size());
    }
}

void Logger::log(LogLevel logLevel, string message) {
    if (logLevel >= targetLevel) {
        string output;

        if (enableColor) {
            switch (logLevel) {
                case LogLevel::DEBUG:
                case LogLevel::VERBOSE:
                    output += COLOR_FG_LIGHT_GREY;
                    break;
                case LogLevel::SUCCESS:
                    output += COLOR_FG_LIGHT_GREEN;
                    break;
                case LogLevel::WARNING:
                    output += COLOR_FG_LIGHT_YELLOW;
                    break;
                case LogLevel::ERROR:
                case LogLevel::FATAL:
                    output += COLOR_FG_LIGHT_RED;
                    break;
                case LogLevel::INFO:
                    break;
            }
        }

        if (longPrefix) {
            output += levelMapLong.at(logLevel);
            addAfterAll(message, "\n", padLong);
        } else {
            output += levelMapShort.at(logLevel);
            addAfterAll(message, "\n", padShort);
        }

        if (enableColor) {
            switch (logLevel) {
                case LogLevel::VERBOSE:
                case LogLevel::SUCCESS:
                case LogLevel::WARNING:
                case LogLevel::ERROR:
                    output += COLOR_FG_DEFAULT;
                    output += message;
                    break;
                case LogLevel::DEBUG:
                case LogLevel::FATAL:
                    output += message;
                    output += COLOR_FG_DEFAULT;
                    break;
                case LogLevel::INFO:
                    output += message;
                    break;
            }
        } else {
            output += message;
        }


        switch (logLevel) {}

        cout << output << endl;
        //        cout << chrono::steady_clock::now().time_since_epoch().count()
        //        << endl;
    }
}
