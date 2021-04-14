#ifndef BGP_SIMULATION_LOGGER_LOGGER_H
#define BGP_SIMULATION_LOGGER_LOGGER_H

#include <map>
#include <memory>
#include <mutex>
#include <string>

using namespace std;

#define L_DEBUG(owner, message) \
    Logger::getInstance()->log(LogLevel::DEBUG, owner, message)
#define L_VERBOSE(owner, message) \
    Logger::getInstance()->log(LogLevel::VERBOSE, owner, message)
#define L_INFO(owner, message) \
    Logger::getInstance()->log(LogLevel::INFO, owner, message)
#define L_SUCCESS(owner, message) \
    Logger::getInstance()->log(LogLevel::SUCCESS, owner, message)
#define L_WARNING(owner, message) \
    Logger::getInstance()->log(LogLevel::WARNING, owner, message);
#define L_ERROR(owner, message) \
    Logger::getInstance()->log(LogLevel::ERROR, owner, message)
#define L_FATAL(owner, message) \
    Logger::getInstance()->log(LogLevel::FATAL, owner, message)

enum class LogLevel : short {
    /**
     * Use this only for debug logs. Link entering/exiting methods or state
     * which low level operation are being performed.
     */
    DEBUG,
    /**
     * Use it for extra information. For example log a state change, or when the
     * program perform high level operation.
     */
    VERBOSE,
    /**
     * Use it to inform a potential user about what is going on. Use it only to
     * provide information targeted to a non-developer audience.
     */
    INFO,
    /**
     * Use it when something has succeeded and it is worth inform the user.
     */
    SUCCESS,
    /**
     * Use it to signal an unexpected situation that should not occur but does
     * not prevent the program to continue.
     */
    WARNING,
    /**
     * Use it to signal that an error has occurred and the program will try to
     * continue anyway, so the output might be not what the user expect.
     */
    ERROR,
    /**
     * Use it to signal an unrecoverable error that makes the program stop.
     */
    FATAL
};

class Logger {
   public:
    /**
     * Get an instance of the singleton that manages the Logger, creating it if
     * necessary.
     *
     * @return The singleton instance of the logger
     */
    static shared_ptr<Logger> getInstance();

    /**
     * Set the minimum log level that will be printed out.
     *
     * @param targetLevel The minimum displayed level.
     */
    void setTargetLogLevel(LogLevel targetLevel);

    /**
     * Set the length of the level suffix.
     *
     * @example
     * \code
     * Brief:
     * [|] Debug example
     * [:] Verbose example
     * [.] Info example
     * [+] Success example
     * [!] Warning example
     * [-] Error example
     * [x] Fatal example
     *
     * Long:
     * [DEBUG  ] Debug example
     * [VERBOSE] Verbose example
     * [INFO   ] Info example
     * [SUCCESS] Success example
     * [WARNING] Warning example
     * [ERROR  ] Error example
     * [FATAL  ] Fatal example
     * \endcode
     *
     * @param longPrefix \a true for the long version, \a false for the brief.
     */
    void setLongPrefix(bool longPrefix);

    /**
     * Set the output to be colorful.
     *
     * @param enableColor \a true enable the colors, \false disable them.
     */
    void setEnableColor(bool enableColor);

    /**
     * Show or hide the time when the log occurred.
     *
     * @param printTimestamp \a true enable the timestamp, \false disable it.
     */
    void setPrintTimestamp(bool printTimestamp);

    /**
     * Log the \a message if the current \a level is lower or equal to \a
     * logLevel. The function will treat \code '\n' \endcode in a special manner
     * by printing an alignment padding after the newline.
     * It is safe to call this method from multiple threads concurrently.
     *
     * @param logLevel The level with whom the message will be logged.
     * @param owner The name of component that originates the log message. (e.g.
     * "parser", "main", "Device 42", ...). It should have no more than 10
     * characters.
     * @param message The message to be logged.
     */
    void log(LogLevel logLevel, string owner, string message);

   private:
    static shared_ptr<Logger> logger;
    LogLevel                  targetLevel    = LogLevel::INFO;
    bool                      longPrefix     = false;
    bool                      enableColor    = true;
    bool                      printTimestamp = true;
    std::mutex                mutex;

    string ESCAPE_CHAR           = "\033";
    string COLOR_FG_DEFAULT      = ESCAPE_CHAR + "[39m";
    string COLOR_FG_LIGHT_GREY   = ESCAPE_CHAR + "[90m";
    string COLOR_FG_LIGHT_RED    = ESCAPE_CHAR + "[91m";
    string COLOR_FG_LIGHT_GREEN  = ESCAPE_CHAR + "[92m";
    string COLOR_FG_LIGHT_YELLOW = ESCAPE_CHAR + "[93m";

    map<LogLevel, string> levelMapLong = {{LogLevel::DEBUG, "DEBUG"},
                                          {LogLevel::VERBOSE, "VERBOSE"},
                                          {LogLevel::INFO, "INFO"},
                                          {LogLevel::SUCCESS, "SUCCESS"},
                                          {LogLevel::WARNING, "WARNING"},
                                          {LogLevel::ERROR, "ERROR"},
                                          {LogLevel::FATAL, "FATAL"}};

    const int padPrefixInternalLongLength =
        levelMapLong.at(LogLevel::SUCCESS)
            .size();  // SUCCESS if one of the longest

    const int padPrefixExternalLongLength =
        encloseInBrackets("", padPrefixInternalLongLength).size();

    map<LogLevel, string> levelMapShort = {{LogLevel::DEBUG, "|"},
                                           {LogLevel::VERBOSE, ":"},
                                           {LogLevel::INFO, "."},
                                           {LogLevel::SUCCESS, "+"},
                                           {LogLevel::WARNING, "!"},
                                           {LogLevel::ERROR, "-"},
                                           {LogLevel::FATAL, "x"}};

    const int padPrefixInternalShortLength =
        levelMapShort.at(LogLevel::SUCCESS).size();
    const int padPrefixExternalShortLength =
        encloseInBrackets("", padPrefixInternalShortLength).size();

    const int padTimestampInternalLength = string("HH:MM:ss.mmm").size();
    const int padTimestampExternalLength =
        encloseInBrackets("", padTimestampInternalLength).size();

    const int padOwnerInternalLength = 10;
    const int padOwnerExternalLength =
        encloseInBrackets("", padOwnerInternalLength).size();


    static string encloseInBrackets(const string& message, int padLenght);

    Logger() = default;
};

#endif  // BGP_SIMULATION_LOGGER_LOGGER_H
