#pragma once

#include <cstring>
#include <cassert>
#include <string>
#include <vector>
#include <utility>
#include <algorithm>
#include <unordered_map>
//#include <cxxabi.h>

#include <Se/Format.hpp>

#ifdef _WIN32
#include <windows.h>
#define strcasecmp _stricmp
#define strncasecmp _strnicmp
#else // assuming POSIX or BSD compliant system
#include <strings.h>
#endif
 
typedef wchar_t WChar;
typedef std::wstring WString; 

#if __cplusplus >= 201103L
#  define STRING_CXX11 1
#endif

#if __cplusplus >= 201402L
#  define STRING_CXX14 1
#endif

#if __cplusplus >= 201703L
#  define STRING_CXX17 1
#endif

#if __cplusplus >= 202002L
#  define STRING_CXX20 1
#endif

#if __cplusplus >= 202207L
#  define STRING_CXX23 1
#endif

namespace Se {

class String : public std::string {
public:
    String() noexcept : std::string() {}

    /// Construct from another string.
    String(const String& str) : std::string() {
        resize(str.length());
        copy_chars(data(), str.c_str(), str.length());
    }

    //String(String& str) : std::string(str) {}

    String(const std::string& str) : std::string(str.c_str(), str.length()) {}

    //String(std::string& str) : std::string(str) {}

    /// Move-construct from another string.
    String(String && str) : std::string(std::move(str)) {}

    /// Construct from a C string.
    String(const char* str) : std::string(str ? str : "") {}

    /// Construct from a C string.
    String(char* str) : std::string(str) {}

    /// Construct from a char array and length.
    String(const char* str, std::size_t length) : std::string(str, length) {}

    virtual ~String() = default;

    /// Return index to the first occurrence of a character, or NPOS if not found.
    std::size_t find(char c, std::size_t startPos = 0, bool caseSensitive = true) const;
    ///
    std::size_t find(const String &str, std::size_t startPos = 0, bool caseSensitive = true) const;

    std::size_t find_last(char c, std::size_t startPos = npos, bool caseSensitive = true) const;

    std::size_t find_last(const String& str, std::size_t startPos = npos, bool caseSensitive  = true) const;

#if __cplusplus >= 202207L
    /// Return whether contains a specific occurrence of a string.
    bool contains(const char* str, bool caseSensitive) const {
        return find(str, 0, caseSensitive) != String::npos; }
    /// Return whether contains a specific character.
    bool contains(char c, bool caseSensitive) const {
        return find(c, 0, caseSensitive) != String::npos; }
#else
    /// Return whether contains a specific occurrence of a string.
    bool contains(const char* str, bool caseSensitive = true) const {
        return find(str, 0, caseSensitive) != String::npos; }
    /// Return whether contains a specific character.
    bool contains(char c, bool caseSensitive = true) const {
        return find(c, 0, caseSensitive) != String::npos; }
#endif

//#ifndef STRING_CXX20
    /// Return whether starts with a string.
    bool starts_with(const String& str, bool caseSensitive = true) const {
        return find(str, 0, caseSensitive) == 0;
    }
    /// Return whether ends with a string.
    bool ends_with(const String& str, bool caseSensitive = true) const {
        String tmpThis = *this;
        String tmpStr = str;
        if (!caseSensitive) {
            tmpThis.to_lower();
            tmpStr.to_lower();
        }

        std::size_t pos = tmpThis.rfind(tmpStr.c_str(), tmpThis.length()-1);
        return pos != String::npos && pos == tmpThis.length() - tmpStr.length();
    }

    bool starts_with(char c, bool caseSensitive = true) const {
        if (empty())
            return false;

        char cTmp = (*this).front();

        if (caseSensitive)
            return (std::tolower(cTmp) == std::tolower(c));

        return cTmp == c;
    }
    /// Return whether ends with a string.
    bool ends_with(char c, bool caseSensitive = true) const {
        if (empty())
            return false;

        char cTmp = (*this).back();

        if (caseSensitive)
            return (std::tolower(cTmp) == std::tolower(c));

        return cTmp == c;
    }
//#endif

    //erase_first

    template <typename T>
	inline iterator erase_first(const T& value)
	{
		//static_assert(std::has_equality_v<T>, "T must be comparable");

		iterator it = std::find(begin(), end(), value);
		return (it != end()) ? erase(it) : it;
	}

    void to_lower() {
        for (auto i = 0; i < this->length(); i++)
            (*this).data()[i] = std::tolower((*this).data()[i]);
    }

    void to_upper() {
        for(auto i = 0; i < this->length(); i++)
            (*this).data()[i] = std::toupper((*this).at(i));
    }

    bool comparei(const String& rsh) const
    {
        if (this->size() != rsh.length())
            return false;

        return strncasecmp(c_str(), rsh.c_str(), size());
    }

    bool compares(const String& rsh, bool caseSensitive = true)
    {
        if (this->size() != rsh.length())
            return false;

        return (caseSensitive ? strncmp(c_str(), rsh.c_str(), size()) :
            strncasecmp(c_str(), rsh.c_str(), size())) == 0;
    }

    void pop_front() 
    {
        if (empty()) {
            throw std::out_of_range("Cannot pop from an empty string");
        }
            
        // Remove the first character from the string
        erase(begin());
    }

    void replace(std::size_t pos, std::size_t length, const char *srcStart, std::size_t srcLength);

    void replace(char replaceThis, char replaceWith, bool caseSensitive = true);

    void replace(const String &replaceThis, const String &replaceWith, bool caseSensitive = true);

    String replaced(const String &replaceThis, const String &replaceWith) const {
        String ret = *this;
        ret.replace(replaceThis, replaceWith);
        return ret;
    }

    String replaced(char replaceThis, char replaceWith) const {
        String ret = *this;
        ret.replace(replaceThis, replaceWith);
        return ret;
    }

    /// Return string with whitespace trimmed from the beginning and the end.
    String trimmed() const;

    void remove();

    /// Return substrings split by a separator char. By default don't return empty strings.
    std::vector<String> split(char separator, bool keepEmptyStrings = false) const {
        return split(c_str(), separator, keepEmptyStrings); }
    /// Join substrings with a 'glue' string.
    void join(const std::vector<String>& subStrings, const String& glue) {
        *this = joined(subStrings, glue); }

    /// Return substrings split by a separator char. By default don't return empty strings.
    static std::vector<String> split(const char* str, char separator, bool keepEmptyStrings = false) {
        std::vector<String> ret;
        const char* strEnd = str + String::CStringLength(str);

        for (const char* splitEnd = str; splitEnd != strEnd; ++splitEnd)
        {
            if (*splitEnd == separator)
            {
                const std::ptrdiff_t splitLen = splitEnd - str;
                if (splitLen > 0 || keepEmptyStrings)
                    ret.emplace_back(str, splitLen);
                str = splitEnd + 1;
            }
        }

        const std::ptrdiff_t splitLen = strEnd - str;
        if (splitLen > 0 || keepEmptyStrings)
            ret.emplace_back(String(str, splitLen));

        return ret;
    }
    /// Return a string by joining substrings with a 'glue' string.
    static String joined(const std::vector<String>& subStrings, const String& glue) {
        if (subStrings.empty())
        return String();

        String joinedString(subStrings[0]);
        for (std::size_t i = 1; i < subStrings.size(); ++i) {
            joinedString.append(glue + subStrings[i]);
        }

        return joinedString;
    }

    String operator=(std::string& str)
    {
       *this = String(str.c_str());
       return *this;
    }

    template<typename T>
    String& operator=(std::basic_string<T>& str)
    {
       *this = String(str.c_str());
       return *this;
    }
    

    String& operator=(const std::string& str)
    {
        resize(str.length());
        copy_chars(data(), str.c_str(), str.length());
        return *this;
    }

    String& operator=(const String& str) 
    {
        auto len = str.length();
        resize(len);
        copy_chars(data(), str.c_str(), len);
        return *this;
    }

    String& operator=(const char* str)
    {
        auto len = strlen(str);
        resize(len);
        copy_chars(data(), str, len);
        return *this;
    }

    inline operator const char*() const { return data(); }
    // inline operator const void*() const { return data(); }

    // String operator=(std::string& str) {
    //     static_cast<std::string>(*this) = str;
    //     return *this;
    // }

    // const String operator=(const std::string& str) {
    //     static_cast<std::string>(*this) = str;
    //     return *this;
    // }

//    string operator+=(char* str) {
//        this->append(str);
//        return *this;
//    }


    String operator+(const String& str) {
        return String(*this).append(str);
    }

    String operator+(const std::string& str) {
        return String(*this).append(str);
    }

//    string operator+(std::string& str) {
//        this->append(str);
//        return *this;
//    }

    String operator+(const char* str) {
        return String(*this).append(str);
    }

    // operator std::string()
    // {
    //     return static_cast<std::string>(*this);
    // }

    static const String EMPTY;

    /// Return length of a C string.
    inline static std::size_t CStringLength(const char* str) {
        return str ? (std::size_t)strlen(str) : 0; }

protected:

    void move_range(std::size_t dest, std::size_t src, std::size_t count) {
        if (count)
            memmove(this->data() + dest, this->data() + src, count);
    }

    static void copy_chars(char *dest, const char *src, std::size_t count) {
#ifdef _MSC_VER
        if (count)
        memcpy(dest, src, count);
#else
        char *end = dest + count;
        while (dest != end) {
            *dest = *src;
            ++dest;
            ++src;
        }
#endif
    }

    void replace(std::size_t pos, std::size_t length, const String &replaceWith)
    {
        // If substring is illegal, do nothing
        if (pos + length > (*this).length())
            return;

        replace(pos, length, replaceWith.data(), replaceWith.length());
    }
#ifndef NDEBUG
    const char* ptr_{this->c_str()};
#endif
};

inline const String String::EMPTY{""};


namespace Helpers {

inline static void CopyChars(char *dest, const char *src, std::size_t count) {
#ifdef _MSC_VER
    if (count)
        memcpy(dest, src, count);
#else
    char *end = dest + count;
    while (dest != end) {
        *dest = *src;
        ++dest;
        ++src;
    }
#endif
}

inline unsigned DecodeUTF16(const wchar_t*& src)
{
    if (src == nullptr)
        return 0;

    unsigned short word1 = *src++;

    // Check if we are at a low surrogate
    if (word1 >= 0xdc00 && word1 < 0xe000)
    {
        while (*src >= 0xdc00 && *src < 0xe000)
            ++src;
        return '?';
    }

    if (word1 < 0xd800 || word1 >= 0xe000)
        return word1;
    else
    {
        unsigned short word2 = *src++;
        if (word2 < 0xdc00 || word2 >= 0xe000)
        {
            --src;
            return '?';
        }
        else
            return (((word1 & 0x3ff) << 10) | (word2 & 0x3ff)) + 0x10000;
    }
}

inline void EncodeUTF8(char*& dest, unsigned unicodeChar)
{
    if (unicodeChar < 0x80)
        *dest++ = unicodeChar;
    else if (unicodeChar < 0x800)
    {
        dest[0] = (char)(0xc0u | ((unicodeChar >> 6u) & 0x1fu));
        dest[1] = (char)(0x80u | (unicodeChar & 0x3fu));
        dest += 2;
    }
    else if (unicodeChar < 0x10000)
    {
        dest[0] = (char)(0xe0u | ((unicodeChar >> 12u) & 0xfu));
        dest[1] = (char)(0x80u | ((unicodeChar >> 6u) & 0x3fu));
        dest[2] = (char)(0x80u | (unicodeChar & 0x3fu));
        dest += 3;
    }
    else if (unicodeChar < 0x200000)
    {
        dest[0] = (char)(0xf0u | ((unicodeChar >> 18u) & 0x7u));
        dest[1] = (char)(0x80u | ((unicodeChar >> 12u) & 0x3fu));
        dest[2] = (char)(0x80u | ((unicodeChar >> 6u) & 0x3fu));
        dest[3] = (char)(0x80u | (unicodeChar & 0x3fu));
        dest += 4;
    }
    else if (unicodeChar < 0x4000000)
    {
        dest[0] = (char)(0xf8u | ((unicodeChar >> 24u) & 0x3u));
        dest[1] = (char)(0x80u | ((unicodeChar >> 18u) & 0x3fu));
        dest[2] = (char)(0x80u | ((unicodeChar >> 12u) & 0x3fu));
        dest[3] = (char)(0x80u | ((unicodeChar >> 6u) & 0x3fu));
        dest[4] = (char)(0x80u | (unicodeChar & 0x3fu));
        dest += 5;
    }
    else
    {
        dest[0] = (char)(0xfcu | ((unicodeChar >> 30u) & 0x1u));
        dest[1] = (char)(0x80u | ((unicodeChar >> 24u) & 0x3fu));
        dest[2] = (char)(0x80u | ((unicodeChar >> 18u) & 0x3fu));
        dest[3] = (char)(0x80u | ((unicodeChar >> 12u) & 0x3fu));
        dest[4] = (char)(0x80u | ((unicodeChar >> 6u) & 0x3fu));
        dest[5] = (char)(0x80u | (unicodeChar & 0x3fu));
        dest += 6;
    }
}

#define GET_NEXT_CONTINUATION_BYTE(ptr) *(ptr); if ((unsigned char)*(ptr) < 0x80 || (unsigned char)*(ptr) >= 0xc0) return '?'; else ++(ptr);
inline unsigned DecodeUTF8(const char*& src)
{
    if (src == nullptr)
        return 0;

    unsigned char char1 = *src++;

    // Check if we are in the middle of a UTF8 character
    if (char1 >= 0x80 && char1 < 0xc0)
    {
        while ((unsigned char)*src >= 0x80 && (unsigned char)*src < 0xc0)
            ++src;
        return '?';
    }

    if (char1 < 0x80)
        return char1;
    else if (char1 < 0xe0)
    {
        unsigned char char2 = GET_NEXT_CONTINUATION_BYTE(src);
        return (unsigned)((char2 & 0x3fu) | ((char1 & 0x1fu) << 6u));
    }
    else if (char1 < 0xf0)
    {
        unsigned char char2 = GET_NEXT_CONTINUATION_BYTE(src);
        unsigned char char3 = GET_NEXT_CONTINUATION_BYTE(src);
        return (unsigned)((char3 & 0x3fu) | ((char2 & 0x3fu) << 6u) | ((char1 & 0xfu) << 12u));
    }
    else if (char1 < 0xf8)
    {
        unsigned char char2 = GET_NEXT_CONTINUATION_BYTE(src);
        unsigned char char3 = GET_NEXT_CONTINUATION_BYTE(src);
        unsigned char char4 = GET_NEXT_CONTINUATION_BYTE(src);
        return (unsigned)((char4 & 0x3fu) | ((char3 & 0x3fu) << 6u) | ((char2 & 0x3fu) << 12u) | ((char1 & 0x7u) << 18u));
    }
    else if (char1 < 0xfc)
    {
        unsigned char char2 = GET_NEXT_CONTINUATION_BYTE(src);
        unsigned char char3 = GET_NEXT_CONTINUATION_BYTE(src);
        unsigned char char4 = GET_NEXT_CONTINUATION_BYTE(src);
        unsigned char char5 = GET_NEXT_CONTINUATION_BYTE(src);
        return (unsigned)((char5 & 0x3fu) | ((char4 & 0x3fu) << 6u) | ((char3 & 0x3fu) << 12u) | ((char2 & 0x3fu) << 18u) |
                          ((char1 & 0x3u) << 24u));
    }
    else
    {
        unsigned char char2 = GET_NEXT_CONTINUATION_BYTE(src);
        unsigned char char3 = GET_NEXT_CONTINUATION_BYTE(src);
        unsigned char char4 = GET_NEXT_CONTINUATION_BYTE(src);
        unsigned char char5 = GET_NEXT_CONTINUATION_BYTE(src);
        unsigned char char6 = GET_NEXT_CONTINUATION_BYTE(src);
        return (unsigned)((char6 & 0x3fu) | ((char5 & 0x3fu) << 6u) | ((char4 & 0x3fu) << 12u) | ((char3 & 0x3fu) << 18u) |
                          ((char2 & 0x3fu) << 24u) | ((char1 & 0x1u) << 30u));
    }
}


inline unsigned LengthUTF8(const String& string)
{
    unsigned ret = 0;
    const char* src = string.c_str();
    if (!src)
        return ret;

    while (src < &(*string.end()))
    {
        DecodeUTF8(src);
        ++ret;
    }

    return ret;
}

}


//void ToLower(std::string& extension);
//void ToUpper(std::string& extension);

// void Replace(std::string& in, std::size_t pos, std::size_t length, const char* srcStart, std::size_t srcLength);

// void Replace(std::string& in, char replaceThis, char replaceWith, bool caseSensitive = true);

// void Replace(std::string& in, const std::string& replaceThis, const std::string& replaceWith, bool caseSensitive = true);

// std::size_t Find(std::string &in, const std::string &str, std::size_t startPos, bool caseSensitive = true);

// std::string Replaced(std::string inStr, const std::string &replaceThis, const std::string &replaceWith);

//std::string Trimmed(std::string in);

inline String StringMemory(std::size_t size)
{
    String ret;
    if (size < 1024)
        ret += cformat("%db", size);
    else if (size < 1024*1024)
        ret += cformat("%.1fKb", size/1024.0);
    else if (size < 1024*1024*1024)
        ret += cformat("%.1fMb", size/(1024.0*1024.0));
    else ret += cformat("%.1fGb", size/(1024.0*1024.0*1024.0));
    return ret;
}

inline std::wstring ToWString(const String& str)
{
#ifdef _WIN32
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), nullptr, 0);
    std::wstring wstrTo( size_needed, 0 );
    MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
#else
    std::wstring wstrTo(str.begin(), str.end());
#endif
    return wstrTo;
}

inline std::string Ucs2ToUtf8(const wchar_t* string)
{
    std::string result{};
    char temp[7];

    if (!string)
        return result;

    while (*string)
    {
        unsigned unicodeChar = Helpers::DecodeUTF16(string);
        char* dest = temp;
        Helpers::EncodeUTF8(dest, unicodeChar);
        *dest = 0;
        result += (temp);
    }
    return result;
}


template<class T>
void remove_fast(std::vector<T>& arr, std::size_t index)
{
    auto newSize = arr.size()-1;
    arr[index] = arr[newSize]; // write last element in this element
    arr.resize(newSize);
}

// template<class T>
// void remove(std::vector<T>& arr) {
//     arr.resize(arr.size()-1);
// }






inline std::size_t String::find(char c, std::size_t startPos , bool caseSensitive) const {
    if (caseSensitive)
    {
        for (std::size_t i = startPos; i < length(); ++i)
        {
            if (data()[i] == c)
                return i;
        }
    }
    else
    {
        c = (char)tolower(c);
        for (std::size_t i = startPos; i < length(); ++i)
        {
            if (tolower(data()[i]) == c)
                return i;
        }
    }

    return String::npos;
}

inline std::size_t String::find(const String &str, std::size_t startPos, bool caseSensitive) const {
    auto& base = (*this);
    if (!str.length() || str.length() > base.length())
        return String::npos;

    char first = str.front();
    if (!caseSensitive)
        first = (char) tolower(first);

    for (std::size_t i = startPos; i <= base.length() - str.length(); ++i) {
        char c = base[i];
        if (!caseSensitive)
            c = (char) tolower(c);

        if (c == first) {
            std::size_t skip = String::npos;
            bool found = true;
            for (std::size_t j = 1; j < str.length(); ++j) {
                c = base[i + j];
                char d = str[j];
                if (!caseSensitive) {
                    c = (char) tolower(c);
                    d = (char) tolower(d);
                }

                if (skip == String::npos && c == first)
                    skip = i + j - 1;

                if (c != d) {
                    found = false;
                    if (skip != String::npos)
                        i = skip;
                    break;
                }
            }
            if (found)
                return i;
        }
    }

    return String::npos;
}

inline std::size_t String::find_last(char c, std::size_t startPos, bool caseSensitive) const
{
    if (startPos >= length())
        startPos = length() - 1;

    if (caseSensitive)
    {
        for (std::size_t i = startPos; i < length(); --i)
        {
            if (data()[i] == c)
                return i;
        }
    }
    else
    {
        c = (char)tolower(c);
        for (std::size_t i = startPos; i < length(); --i)
        {
            if (tolower(data()[i]) == c)
                return i;
        }
    }

    return npos;
}

inline std::size_t String::find_last(const String& str, std::size_t startPos, bool caseSensitive) const
{
    if (!str.length() || str.length() > length())
        return npos;
    if (startPos > length() - str.length())
        startPos = length() - str.length();

    char first = str.data()[0];
    if (!caseSensitive)
        first = (char)tolower(first);

    for (unsigned i = startPos; i < length(); --i)
    {
        char c = data()[i];
        if (!caseSensitive)
            c = (char)tolower(c);

        if (c == first)
        {
            bool found = true;
            for (unsigned j = 1; j < str.length(); ++j)
            {
                c = data()[i + j];
                char d = str.data()[j];
                if (!caseSensitive)
                {
                    c = (char)tolower(c);
                    d = (char)tolower(d);
                }

                if (c != d)
                {
                    found = false;
                    break;
                }
            }
            if (found)
                return i;
        }
    }

    return npos;
}

inline void String::replace(std::size_t pos, std::size_t length, const char *srcStart, std::size_t srcLength) {
    auto& base = (*this);
    int delta = (int) srcLength - (int) length;

    if (pos + length < base.length()) {
        if (delta < 0) {
            move_range(pos + srcLength, pos + length, base.length() - pos - length);
            base.resize(base.length() + delta);
        }
        if (delta > 0) {
            base.resize(base.length() + delta);
            move_range(pos + srcLength, pos + length, base.length() - pos - length - delta);
        }
    } else
        base.resize(base.length() + delta);

    copy_chars(base.data() + pos, srcStart, srcLength);
}

inline void String::replace(char replaceThis, char replaceWith, bool caseSensitive) {
    auto& base = (*this);
    if (caseSensitive) {
        for (std::size_t i = 0; i < base.length(); ++i) {
            if (base[i] == replaceThis)
                base[i] = replaceWith;
        }
    } else {
        replaceThis = (char) tolower(replaceThis);
        for (std::size_t i = 0; i < base.length(); ++i) {
            if (tolower(base[i]) == replaceThis)
                base[i] = replaceWith;
        }
    }
}

inline void String::replace(const String &replaceThis, const String &replaceWith, bool caseSensitive) {
        auto& base = (*this);

    std::size_t nextPos = 0;

    while (nextPos < base.length()) {
        std::size_t pos = find(replaceThis, nextPos, caseSensitive);
        if (pos == String::npos)
            break;
        replace(pos, replaceThis.length(), replaceWith);
        nextPos = pos + replaceWith.length();
    }
}

inline String String::trimmed() const
{
    String ret = *this;
    std::size_t trimStart = 0;
    std::size_t trimEnd = ret.length();

    while (trimStart < trimEnd)
    {
        char c = ret[trimStart];
        if (c != ' ' && c != 9)
            break;
        ++trimStart;
    }
    while (trimEnd > trimStart)
    {
        char c = ret[trimEnd - 1];
        if (c != ' ' && c != 9)
            break;
        --trimEnd;
    }

    ret = ret.substr(trimStart, trimEnd - trimStart);
    return ret;
}

inline void String::remove() {
    assert(length() > 0 && "String::remove(): bad length");
    resize(length()-1);
    //data()[length()-1] = '\0';
}

//-------------------------------------

class StringView : public std::string_view {

};

template <typename T, typename U>
inline typename std::vector<T>::iterator erase_first(std::vector<T>& arr, const U& value)
{
    //static_assert(std::has_equality_v<T>, "T must be comparable");

    auto it = std::find(arr.begin(), arr.end(), value);
    return (it != arr.end()) ? arr.erase(it) : it;
}

inline void BufferToHexString(String& dest, const void* data, unsigned size)
{
    dest.resize(size * 2);

    const auto* bytes = static_cast<const unsigned char*>(data);
    for (unsigned i = 0; i < size * 2; ++i)
    {
        const unsigned digit = i % 2
                               ? bytes[i / 2] & 0xf
                               : bytes[i / 2] >> 4;

        assert(digit < 16);
        if (digit < 10)
            dest[i] = '0' + digit;
        else
            dest[i] = 'a' + digit - 10;
    }
}

inline bool HexStringToBuffer(std::vector<unsigned char>& dest, const String& source)
{
    dest.resize(source.length() / 2);

    for (unsigned i = 0; i < source.length(); ++i)
    {
        const char ch = source[i];

        unsigned digit = 0;
        if (ch >= '0' && ch <= '9')
            digit = ch - '0';
        else if (ch >= 'A' && ch <= 'F')
            digit = 10 + ch - 'A';
        else if (ch >= 'a' && ch <= 'f')
            digit = 10 + ch - 'a';
        else
            return false;

        assert(digit < 16);
        if (i % 2 == 0)
            dest[i / 2] = digit << 4;
        else
            dest[i / 2] |= digit;
    }

    // Fail if extra symbols in string
    return source.length() % 2 == 0;
}

inline bool ToBool(const Se::String& source)
{
    return ToBool(source.c_str());
}

inline bool ToBool(const char* source)
{
    unsigned length = Se::String::CStringLength(source);

    for (unsigned i = 0; i < length; ++i)
    {
        auto c = (char)tolower(source[i]);
        if (c == 't' || c == 'y' || c == '1')
            return true;
        else if (c != ' ' && c != '\t')
            break;
    }

    return false;
}

// inline int ToInt(const Se::String& source, int base = 10)
// {
//     return ToInt(source.c_str(), base);
// }

inline int ToInt(const char* source, int base = 10)
{
    if (!source)
        return 0;

    // Shield against runtime library assert by converting illegal base values to 0 (autodetect)
    if (base < 2 || base > 36)
        base = 0;

    return (int)strtol(source, nullptr, base);
}

inline long long ToInt64(const char* source, int base = 10)
{
    if (!source)
        return 0;

    // Shield against runtime library assert by converting illegal base values to 0 (autodetect)
    if (base < 2 || base > 36)
        base = 0;

    return strtoll(source, nullptr, base);
}

inline long long ToInt64(const Se::String& source, int base = 10)
{
    return ToInt64(source.c_str(), base);
}


inline unsigned ToUInt(const Se::String& source, int base = 10)
{
    return ToUInt(source.c_str(), base);
}


inline unsigned long long ToUInt64(const char* source, int base = 10)
{
    if (!source)
        return 0;

    // Shield against runtime library assert by converting illegal base values to 0 (autodetect)
    if (base < 2 || base > 36)
        base = 0;

    return strtoull(source, nullptr, base);
}

inline float ToFloat(const Se::String& source)
{
    return ToFloat(source.c_str());
}

inline float ToFloat(const char* source)
{
    if (!source)
        return 0;

    return (float)strtod(source, nullptr);
}

inline double ToDouble(const Se::String& source)
{
    return ToDouble(source.c_str());
}

inline double ToDouble(const char* source)
{
    if (!source)
        return 0;

    return strtod(source, nullptr);
}

// String operator+(const char *s0,const String &s1);

// int operator==(const String &s0, const String &s1);
// int operator==(const String &s0, const char *s1);
// int operator==(const char *s0, const String &s1);

// Parse type from a C string.
template <class T> T FromString(const char* source) {};

template <> inline const char* FromString<const char*>(const char* source) { return source; }
template <> inline String FromString<String>(const char* source) { return source; }
template <> inline bool FromString<bool>(const char* source) { return ToBool(source); }
template <> inline float FromString<float>(const char* source) { return ToFloat(source); }
template <> inline double FromString<double>(const char* source) { return ToDouble(source); }
template <> inline int FromString<int>(const char* source) { return ToInt(source); }
template <> inline unsigned FromString<unsigned>(const char* source) { return ToUInt(source); }

/// Parse type from a string.
template <class T> T FromString(const String& source) { return FromString<T>(source.c_str()); }

// String operator=(const std::string& rhs) {
//     return String(rhs);
// }


#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wformat-security"
#endif

template<class... Args>
inline String cformat(const char * __restrict format, Args... args)
{
    int SIZE = snprintf(NULL, 0, format, args...);
    String tmp;
    tmp.resize(SIZE);
    sprintf(tmp.data(), format, args...);
    return {tmp.c_str()};
}

#ifdef __clang__
#pragma clang diagnostic pop
#endif

inline Se::String operator+(const Se::String &s0, const Se::String &s1) {
    Se::String ret = s0;
	ret.append(s1);
	return ret;
}
inline Se::String operator+(const Se::String &s0, const char *s1) {
    Se::String ret = s0;
	ret.append(s1);
	return ret;
}

inline Se::String operator+(const char *s0, const Se::String &s1) {
	Se::String ret = Se::String(s0);
	ret.append(s1);
	return ret;
}

inline bool IsAlpha(unsigned ch)
{
    return ch < 256 ? isalpha(ch) != 0 : false;
}

inline bool IsDigit(unsigned ch)
{
    return ch < 256 ? isdigit(ch) != 0 : false;
}

inline void EncodeUTF16(WChar*& dest, unsigned unicodeChar)
{
    if (unicodeChar < 0x10000)
        *dest++ = unicodeChar;
    else
    {
        unicodeChar -= 0x10000;
        *dest++ = 0xd800 | ((unicodeChar >> 10) & 0x3ff);
        *dest++ = 0xdc00 | (unicodeChar & 0x3ff);
    }
}

inline std::size_t NextUTF8Char(const Se::String& string, unsigned& byteOffset)
{
    if (string.empty())
        return String::npos;

    const char* src = string.c_str() + byteOffset;
    unsigned ret = Helpers::DecodeUTF8(src);
    byteOffset = (unsigned)(src - string.c_str());

    return ret;
}

inline WString Utf8ToUcs2(const char* string)
{
    WString result{};
    std::size_t neededSize = 0;
    WChar temp[3];

    unsigned byteOffset = 0;
    auto length = std::strlen(string);
    while (byteOffset < length)
    {
        WChar* dest = temp;
        EncodeUTF16(dest,  NextUTF8Char(string, byteOffset));
        neededSize += std::size_t(dest - temp);
    }

    result.resize(neededSize);

    byteOffset = 0;
    WChar* dest = &result[0];
    while (byteOffset < length)
        EncodeUTF16(dest, NextUTF8Char(string, byteOffset));

    return result;
}

inline WString MultiByteToWide(const char* string)
{
#if _WIN32
    return Utf8ToUcs2(string);
#else
    WString result{};
    result.resize(Helpers::LengthUTF8(string));

    unsigned byteOffset = 0;
    WChar* dest = &result[0];
    unsigned length = String::CStringLength(string);
    while (byteOffset < length)
        *dest++ = (wchar_t)NextUTF8Char(string, byteOffset);
    return result;
#endif
}

// String Ucs2ToUtf8(const WChar* string)
// {
//     String result{};
//     char temp[7];

//     if (!string)
//         return result;

//     while (*string)
//     {
//         unsigned unicodeChar = Helpers::DecodeUTF16(string);
//         char* dest = temp;
//         EncodeUTF8(dest, unicodeChar);
//         *dest = 0;
//         result.append(temp);
//     }
//     return result;
// }

inline String WideToMultiByte(const wchar_t* string)
{
#if _WIN32
    return Ucs2ToUtf8(string);
#else
    String result{};
    char temp[7];
    while (*string) {
        char* dest = temp;
        Helpers::EncodeUTF8(dest, (unsigned)*string++);
        *dest = 0;
        result +=String(temp);
    }
    return result;
#endif
}

// #endif


inline String ParcerTypeBlock(const char *c, const char *beginBlock, const char *endBlock)
{
    String output;
    if (strcmp(c, "IN") == 0)
        output += "<";

    if (strcmp(c, "EE") == 0)
        output += ">";

    return output;
}


template<typename T>
inline std::string ToStringTypeId() {
    std::string ret;

    static auto typeIdIsNumber = [](char c) -> bool {
        return c >= '0' && c <= '9';
    };

    static std::unordered_map<char, const char*> basicTypes_{
        {'i', "int"},
        {'l', "long"},
        {'f', "float"},
        {'d', "double"},
        {'b', "bool"},
        {'c', "char"},
        {'s', "short"},
        {'u', "unsigned"},
        {'v', "void"},
        {'w', "wchar_t"}
    };

    std::string typeOrig = typeid(T).name();
    const char *c = typeOrig.data();

    if (typeOrig.length() == 1)
        return basicTypes_[typeOrig[0]];

    std::string numStuck;

    bool isTemplate = false;

    while(*c) {

        if (typeIdIsNumber(*c)) {
            numStuck.push_back(*c);
        }
        else if (!numStuck.empty()) {
            size_t startPos = (c - typeOrig.data());
            size_t sizeS = atoi(numStuck.c_str());
            std::string tmp = typeOrig.substr(startPos, sizeS);
            if (!ret.empty())
                ret += "::";
            ret += tmp;
            numStuck.clear();
            c = typeOrig.data() + startPos + sizeS;
            continue;
        }
    
        c++;
    }

    return ret;
}

using StringVector = std::vector<String>;

} //namespace Se



// Register span with std::hash
namespace std {
template <>
struct hash<Se::String> {
    size_t operator()(const Se::String& s) const {
        return std::hash<std::string>()(static_cast<std::string>(s));
    }
};
} // namespace std