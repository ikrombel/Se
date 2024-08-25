#ifndef SE_CONSOLE_H
#define SE_CONSOLE_H

#include <Se/Format.hpp>
#include <Se/Signal.hpp>
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

enum MsgType {
    MsgNone,
    MsgInfo,
    MsgWarning,
    MsgError,
    MsgDebug
};

struct ConsoleInfo {
    MsgType type_;
    int id_;
    const char* name_;
    const char* fileName_;
    const char* funcName_;
    int line_; 
};

typedef std::function<void(const ConsoleInfo&, const char *msg)> LogCallback;

class ConsolePrivate {

public:
    inline static Se::Signal<const ConsoleInfo&, const char *> signalConsole;

    inline static LogCallback Empty = [](const ConsoleInfo&, const char *) {};
};

template <typename F>
inline void setOutputLog(F&& console, F&& logger = ConsolePrivate::Empty) {

    if (!ConsolePrivate::signalConsole.empty()) {
        //assert("already initializited.");
        return;
    }

    if (console)
        ConsolePrivate::signalConsole.connect(console);
    if (logger)
        ConsolePrivate::signalConsole.connect(logger);
}

template<class... Args>
inline void msg(const ConsoleInfo& info, const std::string& fmt, Args... args) {
    std::string f = format(fmt.c_str(), args...);
    if (!ConsolePrivate::signalConsole.empty())
        ConsolePrivate::signalConsole(info, f.c_str());
    else {
        printf("%s", f.c_str());
    }
}

template<class... Args> inline void error(const char* format, Args... args) { msg({MsgType::MsgError, 0}, format, args...); }
template<class... Args> inline void print(const char *format, Args... args) { msg({ MsgType::MsgNone, 0}, format, args...); }
template<class... Args> inline void info(const char *format, Args... args) { msg({MsgType::MsgInfo, 0}, format, args...); }
template<class... Args> inline void warning(const char *format, Args... args) { msg({MsgType::MsgWarning, 0}, format, args...); }
template<class... Args> inline void debug(const char *format, Args... args) { msg({MsgType::MsgDebug, 0}, format, args...); }


inline LogCallback DefaultColored = [](const Console::ConsoleInfo& info, const char* msg) {

    using namespace Console;

    static const char* strMsgType[] {
        "",
        "INFO",
        "WARNING",
        "ERROR",
        "DEBUG"
    };
    

#if defined(COLORED_TERMINAL)
#  if defined(_WIN32)
    HANDLE var_name = GetStdHandle(STD_OUTPUT_HANDLE);
    switch(info.type_) {
        case MsgType::MsgNone: SetConsoleTextAttribute(var_name, 7); break;
        case MsgType::MsgInfo: SetConsoleTextAttribute(var_name, 10); break;
        case MsgType::MsgWarning: SetConsoleTextAttribute(var_name, 6); break;
        case MsgType::MsgError: SetConsoleTextAttribute(var_name, 12); break;
        case MsgType::MsgDebug: SetConsoleTextAttribute(var_name, 9); break;
    }
    const char* msgTypeStyle = "{}[{}]: ";
#  else
    static const char* strMsgStyle[] {
        "",
        "\033[32;1m{}\033[0m[\033[32;1m{}\033[0m]: ",
        "\033[33;1m{}\033[0m[\033[33;1m{}\033[0m]: ",
        "\033[31;1m{}\033[0m[\033[31;1m{}\033[0m]: ",
        "\033[34;1m{}\033[0m[\033[34;1m{}\033[0m]: "
    };
    const char* msgTypeStyle = strMsgStyle[info.type_];
#  endif
#else
    const char* msgTypeStyle = "{}[{}]: ";
#endif

    std::string retFormat;
    if (info.type_ != MsgType::MsgNone) {
        retFormat += format(msgTypeStyle, strMsgType[info.type_], info.name_ ? info.name_ : "");
    }
    if (info.type_ != MsgType::MsgNone)
    retFormat.append(format("{}:{} in function: {}\n", info.fileName_, info.line_, info.funcName_));
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
        template<class... Args> static void error(const char* fStr, Args... args) { msg({MsgType::MsgError, num, #name}, fStr, args...); } \
        template<class... Args> static void print(const char *fStr, Args... args) { msg({MsgType::MsgNone, num, #name}, fStr, args...); } \
        template<class... Args> static void info(const char *fStr, Args... args) {    msg({MsgType::MsgInfo, num, #name}, fStr, args...); } \
        template<class... Args> static void warning(const char *fStr, Args... args) { msg({MsgType::MsgWarning, num, #name}, fStr, args...); } \
        template<class... Args> static void debug(const char *fStr, Args... args) {   msg({MsgType::MsgDebug, num, #name}, fStr, args...); } \
    } \
}

#define LOG_ERROR(name, fStr, ...) Console::msg({Console::MsgType::MsgError, Console::name::reg::id_, Console::name::reg::name_, __FILE__, __func__, __LINE__}, fStr, ##__VA_ARGS__)
#define LOG_PRINT(name, fStr, ...) Console::msg({Console::MsgType::MsgNone, Console::name::reg::id_, Console::name::reg::name_, __FILE__, __func__, __LINE__}, fStr, ##__VA_ARGS__)
#define LOG_INFO(name, fStr, ...) Console::msg({Console::MsgType::MsgInfo, Console::name::reg::id_, Console::name::reg::name_, __FILE__, __func__, __LINE__}, fStr, ##__VA_ARGS__)
#define LOG_WARNING(name, fStr, ...) Console::msg({Console::MsgType::MsgWarning, Console::name::reg::id_, Console::name::reg::name_, __FILE__, __func__, __LINE__}, fStr, ##__VA_ARGS__)
#define LOG_DEBUG(name, fStr, ...) Console::msg({Console::MsgType::MsgDebug, Console::name::reg::id_, Console::name::reg::name_, __FILE__, __func__, __LINE__}, fStr, ##__VA_ARGS__)

REGISTER_CONSOLE_GROUP(Se, 0)
#define SE_LOG_INFO(formatStr, ...) LOG_INFO(Se, formatStr, ##__VA_ARGS__)
#define SE_LOG_WARNING(formatStr, ...) LOG_WARNING(Se, formatStr, ##__VA_ARGS__)
#define SE_LOG_ERROR(formatStr, ...) LOG_ERROR(Se, formatStr, ##__VA_ARGS__)
#define SE_LOG_DEBUG(formatStr, ...) LOG_DEBUG(Se, formatStr, ##__VA_ARGS__)
#define SE_LOG_PRINT(formatStr, ...) LOG_PRINT(Se, formatStr, ##__VA_ARGS__)

//REGISTER_CONSOLE_GROUP(Script, 1)
//REGISTER_CONSOLE_GROUP(Core, 2)
//REGISTER_CONSOLE_GROUP(Editor, 3)

#endif //SCXX_CONSOLE_H
