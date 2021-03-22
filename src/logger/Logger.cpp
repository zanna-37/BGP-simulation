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
        if (longOutput) {
            output += levelMapLong.at(level);
            addAfterAll(message, "\n", padLong);
        } else {
            output += levelMapShort.at(level);
            addAfterAll(message, "\n", padShort);
        }
        output += message;

        cout << output << endl;
        //        cout << chrono::steady_clock::now().time_since_epoch().count()
        //        << endl;
    }
}

void Logger::shutdown() { logger = nullptr; }
