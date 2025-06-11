#pragma once

#include <algorithm>
#include <cstring>

#include <Se/Console.hpp>
#include <Se/String.hpp>

namespace Se
{

/// Abstract stream for writing.
class Serializer
{
public:
    /// Destruct.
    virtual ~Serializer() = default;

    /// Write bytes to the stream. Return number of bytes actually written.
    virtual std::size_t Write(const void* data, std::size_t size) = 0;

    /// Write a 64-bit integer.
    bool WriteInt64(long long value) {
        return Write(&value, sizeof value) == sizeof value; }
    /// Write a 32-bit integer.
    bool WriteInt(int value) {
        return Write(&value, sizeof value) == sizeof value; }
    /// Write a 16-bit integer.
    bool WriteShort(short value) {
        return Write(&value, sizeof value) == sizeof value; }
    /// Write an 8-bit integer.
    bool WriteByte(signed char value) {
        return Write(&value, sizeof value) == sizeof value; }
    /// Write a 64-bit unsigned integer.
    bool WriteUInt64(unsigned long long value) {
        return Write(&value, sizeof value) == sizeof value; }
    /// Write a 32-bit unsigned integer.
    bool WriteUInt(unsigned value) {
        return Write(&value, sizeof value) == sizeof value; }
    /// Write a 16-bit unsigned integer.
    bool WriteUShort(unsigned short value) {
        return Write(&value, sizeof value) == sizeof value; }
    /// Write an 8-bit unsigned integer.
    bool WriteUByte(unsigned char value) {
        return Write(&value, sizeof value) == sizeof value; }
    /// Write a bool.
    bool WriteBool(bool value) {
        return WriteUByte((unsigned char)(value ? 1 : 0)) == 1; }
    /// Write a float.
    bool WriteFloat(float value) {
        return Write(&value, sizeof value) == sizeof value; }
    /// Write a double.
    bool WriteDouble(double value) {
        return Write(&value, sizeof value) == sizeof value; }
    /// Write a string data without zero termination.
    bool WriteStringData(const String& value) { const char* chars = value.c_str();
        return Write(chars,  static_cast<unsigned>(value.length())) == value.length(); }
    /// Write a null-terminated string.
    bool WriteString(const String& value) {
        const char* chars = value.c_str();
        // Count length to the first zero, because ReadString() does the same
        unsigned length = (unsigned)strlen(chars);
        return Write(chars, length + 1) == length + 1;
    }

    bool WriteText(const String& text)
    {
        if (text.empty())
            return false;

        return Write(text.data(), text.size()) == text.length();
    }

    bool WriteStringVector(const std::vector<String>& value)
    {
        bool success = true;
        success &= WriteVLE((unsigned)value.size());
        for (auto i = value.begin(); i != value.end(); ++i)
            success &= WriteString(*i);
        return success;
    }
    /// Write a four-letter file ID. If the string is not long enough, spaces will be appended.
    bool WriteFileID(const String& value) {
        bool success = true;
        size_t length = std::min<std::size_t>(value.length(), (size_t)4U);

        success &= Write(value.c_str(), length) == length;
        for (auto i = value.length(); i < 4; ++i)
            success &= WriteByte(' ');
        return success;
    }

    /// Write a null-terminated wstring.
    bool WriteWString(const WString& text)
    {
        if (text.empty())
            return false;

        std::size_t length = sizeof(WChar)*text.size();
        return Write(text.data(), length) == length;
    }

    // /// Write a 32-bit StringHash.
    // bool WriteStringHash(const StringHash& value);
    /// Write a buffer, with size encoded as VLE.
    bool WriteBuffer(const std::vector<unsigned char>& value) {
        bool success = true;
        std::size_t size = value.size();

        success &= WriteVLE(size);
        if (size)
            success &= Write(&value[0], size) == size;
        return success;
    }

    /// Write a variable-length encoded unsigned integer, which can use 29 bits maximum.
    bool WriteVLE(unsigned value)
    {
        unsigned char data[4];

        if (value < 0x80)
            return WriteUByte((unsigned char)value);
        else if (value < 0x4000)
        {
            data[0] = (unsigned char)(value | 0x80u);
            data[1] = (unsigned char)(value >> 7u);
            return Write(data, 2) == 2;
        }
        else if (value < 0x200000)
        {
            data[0] = (unsigned char)(value | 0x80u);
            data[1] = (unsigned char)(value >> 7u | 0x80u);
            data[2] = (unsigned char)(value >> 14u);
            return Write(data, 3) == 3;
        }
        else
        {
            data[0] = (unsigned char)(value | 0x80u);
            data[1] = (unsigned char)(value >> 7u | 0x80u);
            data[2] = (unsigned char)(value >> 14u | 0x80u);
            data[3] = (unsigned char)(value >> 21u);
            return Write(data, 4) == 4;
        }
    }

    /// Write a 24-bit network object ID.
    inline bool WriteNetID(unsigned value) {
        return Write(&value, 3) == 3;
    }

    /// Write a text line. Char codes 13 & 10 will be automatically appended.
    inline bool WriteLine(const String& value) {
        bool success = true;
        success &= Write(value.c_str(), value.length()) == value.length();
        success &= WriteUByte(13);
        success &= WriteUByte(10);
        return success;
    }

    template<class T>
    std::size_t WriteArray(T* buff, std::size_t size) {
        return Write(buff, sizeof(T)*size);
    }

    template<class T> bool Write(T val) {
        return Write((const char*)&val, sizeof(T));
    }

    template<class T> T WritePacked(float maxAbsCoord = 1.0f) {
        SE_LOG_WARNING("Serializer::WritePacked<T>(float) is not register for type {}", typeid(T).name()); 
        return T(); }

private:
    inline static const float q = 32767.0f;
};

}
