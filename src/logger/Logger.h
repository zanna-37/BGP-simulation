#ifndef BGP_SIMULATION_LOGGER_LOGGER_H
#define BGP_SIMULATION_LOGGER_LOGGER_H

#include <map>
#include <memory>
#include <string>

using namespace std;

enum class LogLevel : short { DEBUG, VERBOSE, INFO, WARNING, ERROR, FATAL };

class Logger {
   public:
    map<LogLevel, string> levelMapLong = {{LogLevel::DEBUG, "[DEBUG  ]"},
                                          {LogLevel::VERBOSE, "[VERBOSE]"},
                                          {LogLevel::INFO, "[INFO   ]"},
                                          {LogLevel::WARNING, "[WARNING]"},
                                          {LogLevel::ERROR, "[ERROR  ]"},
                                          {LogLevel::FATAL, "[FATAL  ]"}};

    map<LogLevel, string> levelMapShort = {{LogLevel::DEBUG, "[|]"},
                                           {LogLevel::VERBOSE, "[:]"},
                                           {LogLevel::INFO, "[.]"},
                                           {LogLevel::WARNING, "[!]"},
                                           {LogLevel::ERROR, "[-]"},
                                           {LogLevel::FATAL, "[x]"}};


    static shared_ptr<Logger> getInstance();

    void setTargetLogLevel(LogLevel targetLevel);

    void setOutputMode(bool longOutput);

    void log(LogLevel level, const string& message);

    static void shutdown();

   private:
    static shared_ptr<Logger> logger;
    LogLevel                  targetLevel;
    bool                      longOutput = false;

    Logger() = default;
};

#endif  // BGP_SIMULATION_LOGGER_LOGGER_H
