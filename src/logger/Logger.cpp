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

void Logger::log(LogLevel level, const string& message) {
    if (level >= targetLevel) {
        string output;
        if (longOutput) {
            output += levelMapLong.at(level);
        } else {
            output += levelMapShort.at(level);
        }
        output += " ";
        output += message;

        cout << output << endl;
        //        cout << chrono::steady_clock::now().time_since_epoch().count()
        //        << endl;
    }
}

void Logger::shutdown() { logger = nullptr; }
