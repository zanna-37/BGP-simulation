#include "Logger.h"

#include <chrono>
#include <ctime>    // localtime
#include <iomanip>  // put_time
#include <iostream>
#include <sstream>  // stringstream
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

void Logger::setPrintTimestamp(bool printTimestamp) {
    this->printTimestamp = printTimestamp;
}

void Logger::setLongPrefix(bool longPrefix) { this->longPrefix = longPrefix; }

void addPadAfterEndline(std::string&       data,
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

        string padToAdd = printTimestamp ? padTimestamp : "";
        if (longPrefix) {
            output += levelMapLong.at(logLevel);
            padToAdd += padLong;
        } else {
            output += levelMapShort.at(logLevel);
            padToAdd += padShort;
        }

        addPadAfterEndline(message, "\n", padToAdd);

        if (enableColor) {
            // restore color
            switch (logLevel) {
                case LogLevel::VERBOSE:
                case LogLevel::SUCCESS:
                case LogLevel::WARNING:
                case LogLevel::ERROR:
                    output += COLOR_FG_DEFAULT;
                    break;
                case LogLevel::DEBUG:
                case LogLevel::FATAL:
                case LogLevel::INFO:
                    // restore color after the message
                    break;
            }
        }

        if (printTimestamp) {
            auto now = std::chrono::high_resolution_clock::now();
            auto ms  = std::chrono::duration_cast<std::chrono::milliseconds>(
                          now.time_since_epoch()) %
                      1000;
            //            auto us =
            //            std::chrono::duration_cast<std::chrono::microseconds>(
            //                          now.time_since_epoch()) %
            //                      1000;
            auto in_time_t = std::chrono::system_clock::to_time_t(now);

            std::stringstream ss;
            ss << std::put_time(std::localtime(&in_time_t), "[%H:%M:%S");
            ss << '.' << std::setfill('0') << std::setw(3)
               << ms.count()
               /* << "." << std::setw(3) << us.count() */
               << "] ";

            output += ss.str();
        }

        output += message;

        if (enableColor) {
            // restore color
            switch (logLevel) {
                case LogLevel::VERBOSE:
                case LogLevel::SUCCESS:
                case LogLevel::WARNING:
                case LogLevel::ERROR:
                case LogLevel::INFO:
                    // color already restored
                    break;
                case LogLevel::DEBUG:
                case LogLevel::FATAL:
                    output += COLOR_FG_DEFAULT;
                    break;
            }
        }

        mutex.lock();
        cout << output << endl;
        mutex.unlock();
    }
}
