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
                        int                padToAdd) {
    size_t pos = data.find(toSearch);

    while (pos != std::string::npos) {
        pos++;  // Skip the \n
        data.insert(pos, std::string(padToAdd, ' '));
        pos += padToAdd;  // Skip the padding
        pos = data.find(toSearch, pos);
    }
}


void Logger::log(LogLevel logLevel, string owner, string message) {
    if (logLevel >= targetLevel) {
        string             output;
        std::ostringstream oss;

        if (enableColor) {
            switch (logLevel) {
                case LogLevel::DEBUG:
                case LogLevel::VERBOSE:
                    oss << COLOR_FG_LIGHT_GREY;
                    break;
                case LogLevel::SUCCESS:
                    oss << COLOR_FG_LIGHT_GREEN;
                    break;
                case LogLevel::WARNING:
                    oss << COLOR_FG_LIGHT_YELLOW;
                    break;
                case LogLevel::ERROR:
                case LogLevel::FATAL:
                    oss << COLOR_FG_LIGHT_RED;
                    break;
                case LogLevel::INFO:
                    break;
            }
        }

        int padToAdd = 0;
        if (longPrefix) {
            oss << encloseInBrackets(levelMapLong.at(logLevel),
                                     padPrefixInternalLongLength);
            padToAdd += padPrefixExternalLongLength;
        } else {
            oss << encloseInBrackets(levelMapShort.at(logLevel),
                                     padPrefixInternalShortLength);
            padToAdd += padPrefixExternalShortLength;
        }
        padToAdd += printTimestamp ? padTimestampExternalLength : 0;
        padToAdd += padOwnerExternalLength;

        addPadAfterEndline(message, "\n", padToAdd);

        if (enableColor) {
            // restore color
            switch (logLevel) {
                case LogLevel::VERBOSE:
                case LogLevel::SUCCESS:
                case LogLevel::WARNING:
                case LogLevel::ERROR:
                    oss << COLOR_FG_DEFAULT;
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
            auto in_time_t = std::chrono::system_clock::to_time_t(now);

            std::ostringstream timestamp_oss;
            timestamp_oss << std::put_time(std::localtime(&in_time_t),
                                           "%H:%M:%S")
                          << '.' << std::setfill('0') << std::setw(3)
                          << ms.count();

            oss << encloseInBrackets(timestamp_oss.str(),
                                     padTimestampInternalLength);
        }

        oss << encloseInBrackets(owner, padOwnerInternalLength);
        oss << message;

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
                    oss << COLOR_FG_DEFAULT;
                    break;
            }
        }

        mutex.lock();
        output = oss.str();
        cout << output << endl;
        mutex.unlock();
    }
}

string Logger::encloseInBrackets(const string& message, int padLenght) {
    std::ostringstream oss;
    oss << "[" << left << setw(padLenght) << message << "] ";
    return oss.str();
}