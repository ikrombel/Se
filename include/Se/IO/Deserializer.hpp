// Copyright (c) 2008-2019 the GFrost project.

#pragma once

#include <Se/String.hpp>

namespace Se
{

/// Abstract stream for reading.
class Deserializer
{
public:
    /// Construct with zero size.
    Deserializer() : position_(0), size_(0) {}
    /// Construct with defined size.
    explicit Deserializer(unsigned size) : position_(0), size_(size) {}
    /// Destruct.
    virtual ~Deserializer() = default;

    /// Read bytes from the stream. Return number of bytes actually read.
    virtual unsigned Read(void* dest, unsigned size) = 0;
    /// Set position from the beginning of the stream. Return actual new position.
    virtual unsigned Seek(unsigned position) = 0;
    /// Return name of the stream.
    virtual String GetName() const {
        return String::EMPTY;
    }
    /// Return a checksum if applicable.
    virtual unsigned GetChecksum() { return 0; }
    /// Return whether the end of stream has been reached.
    virtual bool IsEof() const { return position_ >= size_; }

    /// Set position relative to current position. Return actual new position.
    unsigned SeekRelative(int delta) {
        return Seek(GetPosition() + delta); }
    /// Return current position.
    unsigned GetPosition() const { return position_; }
    /// Return current position.
    unsigned Tell() const { return position_; }

    /// Return size.
    unsigned GetSize() const { return size_; }

    template<class T>
    long long Read() { T ret; Read(&ret, sizeof ret);
        return ret; }

    /// Read a 64-bit integer.
    long long ReadInt64() { 
        return Read<long long>(); }
    /// Read a 32-bit integer.
    int ReadInt() { 
        return Read<int>(); }
    /// Read a 16-bit integer.
    short ReadShort() { 
        return Read<short>(); }
    /// Read an 8-bit integer.
    signed char ReadByte() { 
        return Read<char>(); }
    /// Read a 64-bit unsigned integer.
    unsigned long long ReadUInt64() { 
        return Read<unsigned long long>(); }
    /// Read a 32-bit unsigned integer.
    unsigned ReadUInt() { 
        return Read<unsigned>(); }
    /// Read a 16-bit unsigned integer.
    unsigned short ReadUShort()  { 
        return Read<unsigned short>(); }
    /// Read an 8-bit unsigned integer.
    unsigned char ReadUByte() { 
        return Read<unsigned char>(); }
    /// Read a bool.
    bool ReadBool() { 
        return ReadUByte(); }
    /// Read a float.
    float ReadFloat() { 
        return Read<float>(); }
    /// Read a double.
    double ReadDouble()  { 
        return Read<double>(); }
    // /// Read an IntRect.
    // IntRect ReadIntRect();
    // /// Read an IntVector2.
    // IntVector2 ReadIntVector2();
    // /// Read an IntVector3.
    // IntVector3 ReadIntVector3();
    // /// Read a Rect.
    // Rect ReadRect();
    // /// Read a Vector2.
    // Vector2 ReadVector2();
    // /// Read a Vector3.
    // Vector3 ReadVector3();
    // /// Read a Vector3 packed into 3 x 16 bits with the specified maximum absolute range.
    // Vector3 ReadPackedVector3(float maxAbsCoord);
    // /// Read a Vector4.
    // Vector4 ReadVector4();
    // /// Read a quaternion.
    // Quaternion ReadQuaternion();
    // /// Read a quaternion with each component packed in 16 bits.
    // Quaternion ReadPackedQuaternion();
    // /// Read a Matrix3.
    // Matrix3 ReadMatrix3();
    // /// Read a Matrix3x4.
    // Matrix3x4 ReadMatrix3x4();
    // /// Read a Matrix4.
    // Matrix4 ReadMatrix4();
    // /// Read a color.
    // Color ReadColor();
    // /// Read a bounding box.
    // BoundingBox ReadBoundingBox();
    /// Read the rest of content as a string.
    String ReadStringData() { 
        String ret;
        while (!IsEof())
            ret += (char)ReadByte();
        return ret;
    }
    /// Read a null-terminated string.
    String ReadString() {
        String ret;
        while (!IsEof()) {
            char c = ReadByte();
            if (!c)
                break;
            else
                ret += c;
        }
        return ret;
    }
    /// Read a four-letter file ID.
    String ReadFileID() {
        String ret;
        ret.resize(4);
        Read(&ret[0], 4);
        return ret;
    }
    // /// Read a 32-bit StringHash.
    // StringHash ReadStringHash();
    /// Read a buffer with size encoded as VLE.
    std::vector<unsigned char> ReadBuffer() {
        std::vector<unsigned char> ret(ReadVLE());
        if (ret.size())
            Read(&ret[0], ret.size());
        return ret;
    }
    // /// Read a resource reference.
    // ResourceRef ReadResourceRef();
    // /// Read a resource reference list.
    // ResourceRefList ReadResourceRefList();
    // /// Read a variant.
    // Variant ReadVariant();
    // /// Read a variant whose type is already known.
    // Variant ReadVariant(VariantType type, Context* context = nullptr);
    // /// Read a variant vector.
    // VariantVector ReadVariantVector();
    /// Read a string vector.
    std::vector<String> ReadStringVector() {
        std::vector<String> ret(ReadVLE());
        for (unsigned i = 0; i < ret.size(); ++i)
            ret[i] = ReadString();
        return ret;
    }
    // /// Read a variant map.
    // VariantMap ReadVariantMap();
    /// Read a variable-length encoded unsigned integer, which can use 29 bits maximum.
    unsigned ReadVLE()
    {
        unsigned byte = ReadUByte();
        unsigned ret = (unsigned)(byte & 0x7fu);
        if (byte < 0x80)
            return ret;

        byte = ReadUByte();
        ret |= ((unsigned)(byte & 0x7fu)) << 7u;
        if (byte < 0x80)
            return ret;

        byte = ReadUByte();
        ret |= ((unsigned)(byte & 0x7fu)) << 14u;
        if (byte < 0x80)
            return ret;

        byte = ReadUByte();
        ret |= ((unsigned)byte) << 21u;
        return ret;
    }
    /// Read a 24-bit network object ID.
    unsigned ReadNetID() {
        unsigned ret = 0;
        Read(&ret, 3);
        return ret;
    }
    /// Read a text line.
    String ReadLine() {
        String ret;

        while (!IsEof())
        {
            char c = ReadByte();
            if (c == 10)
                break;
            if (c == 13)
            {
                // Peek next char to see if it's 10, and skip it too
                if (!IsEof())
                {
                    char next = ReadByte();
                    if (next != 10)
                        Seek(position_ - 1);
                }
                break;
            }

            ret += c;
        }

        return ret;
    }

    template<class T>
    T* ReadArray(unsigned int size) {
        T buff[size];
        Read(buff, sizeof(T)*size);
        return buff;
    }

protected:
    /// Stream position.
    unsigned position_;
    /// Stream size.
    unsigned size_;
};

}
