#ifndef SE_CONSOLE_H
#define SE_CONSOLE_H

#include <Se/Format.hpp>
//#include <Se/Signal.hpp>
#include <functional>
#include <string>

#ifdef _WIN32
//#include <processenv.h>
// #include <winbase.h>
// #include <wincon.h>
#endif

#if !defined(COLORED_TERMINAL) && !defined(_WIN32)
#  define COLORED_TERMINAL 1
#endif

namespace Console {

enum LogLevel {
    LogRaw = -1,
    LogTrace = 0,
    LogDebug = 1,
    LogInfo  = 2,
    LogWarning = 3,
    LogError   = 4,
    LogNone    = 5,
    LogTODO    = 6,
    MAX_LOGLEVELS
};

struct ConsoleInfo {
    LogLevel type_;
    int id_;
    const char* name_;
    const char* fileName_;
    const char* funcName_;
    std::size_t line_{0};
    std::size_t column_{0};
    std::function<void(const char*)> onEvent;
};

typedef std::function<void(const ConsoleInfo&, const char *msg)> LogCallback;

class ConsolePrivate {

public:
    inline static std::function<void(const ConsoleInfo&, const char *)> onPrint{nullptr};
    inline static std::function<void(const ConsoleInfo&, const char *)> onLog{nullptr};

    inline static LogCallback Empty = [](const ConsoleInfo&, const char *) {};


    inline static LogLevel level = 
#if NDEBUG
            LogLevel::LogError;
#else
            LogLevel::LogDebug;
#endif

};

template <typename F>
inline void setOutputLog(F&& console, F&& logger = ConsolePrivate::Empty, LogLevel level = ConsolePrivate::level)
{
    ConsolePrivate::level = level;

    if (console)
        ConsolePrivate::onPrint = console;
    if (logger)
        ConsolePrivate::onLog = logger;
}

template<class... Args>
inline void msg(const ConsoleInfo& info, const std::string& fmt, Args... args) {

    if (info.type_ < ConsolePrivate::level)
        return;

    std::string f = format(fmt.c_str(), args...);
    if (ConsolePrivate::onPrint)
        ConsolePrivate::onPrint(info, f.c_str());
    if (ConsolePrivate::onLog)
        ConsolePrivate::onLog(info, f.c_str());
    else {
#ifndef __ANDROID__
        printf("%s\n", f.c_str());
#endif
    }
}

template<class... Args> inline void error(const char* format, Args... args) { msg({LogLevel::LogError, 0}, format, args...); }
template<class... Args> inline void print(const char *format, Args... args) { msg({LogLevel::LogNone, 0}, format, args...); }
template<class... Args> inline void info(const char *format, Args... args) { msg({LogLevel::LogInfo, 0}, format, args...); }
template<class... Args> inline void warning(const char *format, Args... args) { msg({LogLevel::LogWarning, 0}, format, args...); }
template<class... Args> inline void debug(const char *format, Args... args) { msg({LogLevel::LogDebug, 0}, format, args...); }


inline LogCallback DefaultColored = [](const Console::ConsoleInfo& info, const char* msg) {

    using namespace Console;

    static const char* strLogLevel[] {
        "TRACE",
        "DEBUG",
        "INFO",
        "WARNING",
        "ERROR",
        "",
        "TODO"
    };
    

#if defined(COLORED_TERMINAL)
#  if defined(_WIN32)
    HANDLE var_name = GetStdHandle(STD_OUTPUT_HANDLE);
    switch(info.type_) {
        case LogLevel::LogNone: SetConsoleTextAttribute(var_name, 7); break;
        case LogLevel::LogInfo: SetConsoleTextAttribute(var_name, 10); break;
        case LogLevel::LogWarning: SetConsoleTextAttribute(var_name, 6); break;
        case LogLevel::LogError: SetConsoleTextAttribute(var_name, 12); break;
        case LogLevel::LogDebug: SetConsoleTextAttribute(var_name, 9); break;
        case LogLevel::LogTODO: SetConsoleTextAttribute(var_name, 11); break;
    }
    const char* msgTypeStyle = "{}[{}]: ";
#  else
    static const char* strLogStyle[] {
        "\033[35;1m{}\033[0m[\033[35;1m{}\033[0m]: ",   // LogTrace
        "\033[34;1m{}\033[0m[\033[34;1m{}\033[0m]: ",   // LogDebug
        "\033[32;1m{}\033[0m[\033[32;1m{}\033[0m]: ",   // LogInfo
        "\033[33;1m{}\033[0m[\033[33;1m{}\033[0m]: ",   // LogWarning
        "\033[31,1m{}\033[0m[\033[31,1m{}\033[0m]: ",   // LogError
        "", // LogNone
        "\033[36;1m{}\033[0m[\033[36;1m{}\033[0m]: ",   // LogTODO
    };
    const char* msgTypeStyle = strLogStyle[info.type_];
#  endif
#else
    const char* msgTypeStyle = "{}[{}]: ";
#endif

    std::string retFormat;
    if (info.type_ != LogLevel::LogNone && info.type_ != LogLevel::LogTrace){
        retFormat += format(msgTypeStyle, strLogLevel[info.type_], info.name_ ? info.name_ : "");
        retFormat.append(info.fileName_);
        if (info.line_ != 0)
            retFormat += format(":{}", info.line_);
        if (info.column_ != 0)
            retFormat += format(":{}", info.column_);
        if (info.funcName_)
            retFormat += format(" in function: {}", info.funcName_);

        retFormat += "\n";
    }
    retFormat.append(format("{}\n", msg));

    printf("%s", retFormat.c_str());

#if defined(_WIN32) && defined(COLORED_TERMINAL)
    SetConsoleTextAttribute(var_name, 7);
#endif
};


} // namespace Console


#define REGISTER_CONSOLE_GROUP(name, num) \
namespace Console { \
    namespace name { \
        struct reg { \
            inline static const char* name_ = #name; \
            inline static const int id_ = num; \
        }; \
        template<class... Args> static void error(const char* fStr, Args... args) { msg({LogLevel::LogError, num, #name}, fStr, args...); } \
        template<class... Args> static void print(const char *fStr, Args... args) { msg({LogLevel::LogNone, num, #name}, fStr, args...); } \
        template<class... Args> static void info(const char *fStr, Args... args)  { msg({LogLevel::LogInfo, num, #name}, fStr, args...); } \
        template<class... Args> static void warning(const char *fStr, Args... args) { msg({LogLevel::LogWarning, num, #name}, fStr, args...); } \
        template<class... Args> static void debug(const char *fStr, Args... args)   { msg({LogLevel::LogDebug, num, #name}, fStr, args...); } \
    } \
}

#define LOG_RAW(name, fStr, ...) Console::msg({Console::LogLevel::LogRaw,         Console::name::reg::id_, Console::name::reg::name_, __FILE__, __func__, __LINE__}, fStr, ##__VA_ARGS__)
#define LOG_TRACE(name, fStr, ...) Console::msg({Console::LogLevel::LogTrace,     Console::name::reg::id_, Console::name::reg::name_, __FILE__, __func__, __LINE__}, fStr, ##__VA_ARGS__)
#define LOG_ERROR(name, fStr, ...) Console::msg({Console::LogLevel::LogError,     Console::name::reg::id_, Console::name::reg::name_, __FILE__, __func__, __LINE__}, fStr, ##__VA_ARGS__)
#define LOG_PRINT(name, fStr, ...) Console::msg({Console::LogLevel::LogNone,      Console::name::reg::id_, Console::name::reg::name_, __FILE__, __func__, __LINE__}, fStr, ##__VA_ARGS__)
#define LOG_INFO(name, fStr, ...) Console::msg({Console::LogLevel::LogInfo,       Console::name::reg::id_, Console::name::reg::name_, __FILE__, __func__, __LINE__}, fStr, ##__VA_ARGS__)
#define LOG_WARNING(name, fStr, ...) Console::msg({Console::LogLevel::LogWarning, Console::name::reg::id_, Console::name::reg::name_, __FILE__, __func__, __LINE__}, fStr, ##__VA_ARGS__)
#define LOG_DEBUG(name, fStr, ...) Console::msg({Console::LogLevel::LogDebug,     Console::name::reg::id_, Console::name::reg::name_, __FILE__, __func__, __LINE__}, fStr, ##__VA_ARGS__)
#define LOG_TODO(name, fStr, ...) { \
    static bool logged_todo_##name##__LINE__ = false; \
    if (!logged_todo_##name##__LINE__) { \
        logged_todo_##name##__LINE__ = true; \
        Console::msg({Console::LogLevel::LogTODO, Console::name::reg::id_, Console::name::reg::name_, __FILE__, __func__, __LINE__}, fStr, ##__VA_ARGS__); \
    }}

REGISTER_CONSOLE_GROUP(Se, 0)
#define SE_LOG_INFO(formatStr, ...) LOG_INFO(Se, formatStr, ##__VA_ARGS__)
#define SE_LOG_WARNING(formatStr, ...) LOG_WARNING(Se, formatStr, ##__VA_ARGS__)
#define SE_LOG_ERROR(formatStr, ...) LOG_ERROR(Se, formatStr, ##__VA_ARGS__)
#define SE_LOG_DEBUG(formatStr, ...) LOG_DEBUG(Se, formatStr, ##__VA_ARGS__)
#define SE_LOG_PRINT(formatStr, ...) LOG_PRINT(Se, formatStr, ##__VA_ARGS__)
#define SE_LOG_TODO(formatStr, ...) LOG_TODO(Se, formatStr, ##__VA_ARGS__)

//REGISTER_CONSOLE_GROUP(Script, 1)
//REGISTER_CONSOLE_GROUP(Core, 2)
//REGISTER_CONSOLE_GROUP(Editor, 3)

#endif //SCXX_CONSOLE_H
