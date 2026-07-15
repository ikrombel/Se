#pragma once

#include <Se/String.hpp>

#include <exception>

namespace Se
{

/// Generic runtime exception adapted for usage in Urho.
/// Note that this exception shouldn't leak into main loop of the engine and should only be used internally.
class RuntimeException : public std::exception
{
public:
    /// Construct exception with static message.
    explicit RuntimeException(const String& message) : message_(message) {}
    /// Construct exception with formatted message.
    template <class T, class ... Ts>
    RuntimeException(const String& fmt, const T& firstArg, const Ts& ... otherArgs)
    {
        try
        {
            message_ = format(fmt.c_str(), firstArg, otherArgs...);
        }
        catch(const std::exception& e)
        {
            message_ = "Failed to format RuntimeException: ";
            message_ += e.what();
        }
    }
    /// Return message.
    const String GetMessage() const { return message_; }

    const char* what() const noexcept override { return message_.c_str(); }

private:
    String message_;
};



}
