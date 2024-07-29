#pragma once

// #include "Urho3D/Container/Hash.h"
// #include "Urho3D/Math/MathDefs.h"

#include <Se/Format.hpp>
#include <Se/String.hpp>

#include <string_view>
#include <string>
#include <unordered_map>
#include <cstdint>
#include <cstdio>
#include <cassert>
#include <iostream>

namespace Se
{

namespace Detail
{

/// Calculate hash in compile time.
/// This function should be identical to eastl::hash<string_view> specialization from EASTL/string_view.h
static constexpr std::size_t CalculateEastlHash(const std::string_view& x)
{
    using namespace std;
    string_view::const_iterator p = x.cbegin();
    string_view::const_iterator end = x.cend();
    uint32_t result = 2166136261U; // We implement an FNV-like string hash.
    while (p != end)
        result = (result * 16777619) ^ (uint8_t)*p++;
    return (size_t)result;
}

// struct HashReverseMap {
//     static const std::unordered_map<StringHash, std::string>* data;
// };

// inline std::unordered_map<StringHash, std::string>* HashReverseMap::data = nullptr;

} // namespace Detail

//class StringHashRegister;

/// 32-bit hash value for a string.
class StringHash
{
    
public:



    /// Tag to disable population of hash reversal map.
    struct NoReverse
    {
    };

    /// Construct with zero value.
    constexpr StringHash() noexcept
        : value_(EmptyValue)
    {
    }

    /// Construct with an initial value.
    constexpr explicit StringHash(unsigned value) noexcept
        : value_(value)
    {
    }

    /// Construct from a C string (no side effects).
    constexpr StringHash(const char* str, NoReverse) noexcept // NOLINT(google-explicit-constructor)
        : StringHash(std::string_view{str}, NoReverse{})
    {
    }

    /// Construct from a string (no side effects).
    StringHash(const String& str, NoReverse) noexcept // NOLINT(google-explicit-constructor)
        : StringHash(std::string_view{str}, NoReverse{})
    {
    }

    /// Construct from a string (no side effects).
    constexpr StringHash(const std::string_view& str, NoReverse) noexcept // NOLINT(google-explicit-constructor)
        : value_(Calculate(str.data(), static_cast<unsigned>(str.length())))
    {
    }

    /// Construct from a C string.
    StringHash(const char* str) noexcept // NOLINT(google-explicit-constructor)
        : StringHash(std::string_view{str})
    {
    }

    /// Construct from a string.
    StringHash(const String& str) noexcept // NOLINT(google-explicit-constructor)
        : StringHash(std::string_view{str})
    {
    }

    /// Construct from a string.
    StringHash(const std::string_view& str) noexcept // NOLINT(google-explicit-constructor)
        : value_(Calculate(str.data(), str.length()))
    {
    #ifdef SE_HASH_DEBUG
        Se::GetGlobalStringHashRegister().RegisterString(*this, str);
    #endif
    }


    /// Test for equality with another hash.
    constexpr bool operator==(const StringHash& rhs) const { return value_ == rhs.value_; }

    /// Test for inequality with another hash.
    constexpr bool operator!=(const StringHash& rhs) const { return value_ != rhs.value_; }

    /// Test if less than another hash.
    constexpr bool operator<(const StringHash& rhs) const { return value_ < rhs.value_; }

    /// Test if greater than another hash.
    constexpr bool operator>(const StringHash& rhs) const { return value_ > rhs.value_; }

    /// Return true if nonempty hash value.
    constexpr bool IsEmpty() const { return value_ == EmptyValue; }

    /// Return true if nonempty hash value.
    constexpr explicit operator bool() const { return !IsEmpty(); }

    /// Return hash value.
    /// @property
    constexpr unsigned Value() const { return value_; }

    /// Return mutable hash value. For internal use only.
    constexpr unsigned& MutableValue() { return value_; }

    /// Return as string.
    String ToString()
    {
        int size = snprintf(nullptr, 0, "%08X", value_);

        String ret;
        ret.resize(size);
        sprintf(ret.data(), "%08X", value_);
        return ret;
    }

    /// Return debug string that contains hash value, and reversed hash string if possible.
    String ToDebugString()
    {
    #ifdef SE_HASH_DEBUG
        return format("#{} '{}'", ToString(), Reverse());
    #else
        return String(format("#{}", ToString().c_str()).c_str());
    #endif
    }

    /// Return string which has specific hash value. Return first string if many (in order of calculation).
    /// Use for debug purposes only. Return empty string if URHO3D_HASH_DEBUG is off.
    String Reverse() const
    {
    #ifdef SE_HASH_DEBUG
        return Se::GetGlobalStringHashRegister().GetStringCopy(*this);
    #else
        return ""; //String::EMPTY;
    #endif
    }

    /// Return hash value for HashSet & HashMap.
    constexpr unsigned ToHash() const { return value_; }

    /// Calculate hash value for string_view.
    static constexpr unsigned Calculate(const std::string_view& view)
    {
        return static_cast<unsigned>(Detail::CalculateEastlHash(view));
    }

    /// Calculate hash value from a C string.
    static constexpr unsigned Calculate(const char* str)
    { //
        return Calculate(std::string_view(str));
    }

    /// Calculate hash value from binary data.
    static constexpr unsigned Calculate(const char* data, unsigned length)
    {
        return Calculate(std::string_view(data, length));
    }

    // /// Get global StringHashRegister. Use for debug purposes only. Return nullptr if URHO3D_HASH_DEBUG is off.
    // static StringHashRegister* GetGlobalStringHashRegister();

    /// Hash of empty string. Is *not* zero!
    inline static constexpr auto EmptyValue = static_cast<unsigned>(Detail::CalculateEastlHash(std::string_view{""}));
    static const StringHash Empty;

#ifdef SE_HASH_DEBUG
    static StringHashRegister& GetGlobalStringHashRegister()
    {
        static StringHashRegister stringHashRegister(true /*thread safe*/);
        hashReverseMap = &stringHashRegister.GetInternalMap();
        return stringHashRegister;
    }
#endif

private:
    /// Hash value.
    unsigned value_;
};

inline const StringHash StringHash::Empty{""};

inline constexpr StringHash operator"" _sh(const char* str, std::size_t len)
{
    return StringHash{std::string_view{str, len}, StringHash::NoReverse{}};
}

static_assert(sizeof(StringHash) == sizeof(unsigned), "Unexpected StringHash size.");

} // namespace Se
