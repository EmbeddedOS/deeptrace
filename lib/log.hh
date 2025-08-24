#pragma once
#include <assert.h>
#include <string>
#include <iostream>
#include <sstream>

namespace DeepTrace
{
    class Logger
    {
    public:
        enum class Level
        {
            Debug = 0,
            Info,
            Warn,
            Error,
            Fatal
        };

    private:
        std::ostringstream _str;
        Logger::Level _level;

    public:
        Logger(Logger::Level level,
               const char *file,
               int line,
               const char *function) : _level{level}
        {
            this->_str << "[" << LevelToString(level) << "]["
                       << file << ":" << line << "][" << function << "()]: ";
        }

        ~Logger()
        {
            std::cerr << std::boolalpha << this->_str.str() << std::endl;
            if (this->_level == Logger::Level::Fatal)
            {
                abort();
            }
        }

        template <typename T>
        Logger &operator<<(T data)
        {
            this->_str << data;
            return *this;
        }

    private:
        std::string LevelToString(Logger::Level level)
        {
            switch (level)
            {
            case Logger::Level::Debug:
                return "DEBUG";
            case Logger::Level::Info:
                return "INFO";
            case Logger::Level::Warn:
                return "WARN";
            case Logger::Level::Error:
                return "ERROR";
            case Logger::Level::Fatal:
                return "FATAL";
            }

            return "";
        }
    };
}

#define Debug() DeepTrace::Logger(DeepTrace::Logger::Level::Debug, \
                               __FILE_NAME__,                \
                               __LINE__,                     \
                               __func__)

#define Info() DeepTrace::Logger(DeepTrace::Logger::Level::Info, \
                              __FILE_NAME__,               \
                              __LINE__,                    \
                              __func__)

#define Warn() DeepTrace::Logger(DeepTrace::Logger::Level::Warn, \
                              __FILE_NAME__,               \
                              __LINE__,                    \
                              __func__)

#define Error() DeepTrace::Logger(DeepTrace::Logger::Level::Error, \
                               __FILE_NAME__,                \
                               __LINE__,                     \
                               __func__)

#define Fatal() DeepTrace::Logger(DeepTrace::Logger::Level::Fatal, \
                               __FILE_NAME__,                \
                               __LINE__,                     \
                               __func__)
