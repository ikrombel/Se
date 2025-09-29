// Copyright (c) 2017-2020 the rbfx project.


#include "Base64Archive.h"


namespace Se
{

static const String base64_chars =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789+/";


static inline bool IsBase64(char c) {
    return (isalnum(c) || (c == '+') || (c == '/'));
}

String EncodeBase64(const ByteVector& buffer)
{
    String ret;

    const unsigned char* bytesToEncode = buffer.data();
    unsigned length = buffer.size();

    int i = 0;
    int j = 0;
    unsigned char charArray3[3];
    unsigned char charArray4[4];

    while (length--)
    {
        charArray3[i++] = *(bytesToEncode++);
        if (i == 3)
        {
            charArray4[0] = (charArray3[0] & 0xfc) >> 2;
            charArray4[1] = ((charArray3[0] & 0x03) << 4) + ((charArray3[1] & 0xf0) >> 4);
            charArray4[2] = ((charArray3[1] & 0x0f) << 2) + ((charArray3[2] & 0xc0) >> 6);
            charArray4[3] = charArray3[2] & 0x3f;

            for (i = 0; (i < 4); i++)
                ret += base64_chars.c_str()[charArray4[i]];
            i = 0;
        }
    }

    if (i)
    {
        for (j = i; j < 3; j++)
            charArray3[j] = '\0';

        charArray4[0] = (charArray3[0] & 0xfc) >> 2;
        charArray4[1] = ((charArray3[0] & 0x03) << 4) + ((charArray3[1] & 0xf0) >> 4);
        charArray4[2] = ((charArray3[1] & 0x0f) << 2) + ((charArray3[2] & 0xc0) >> 6);

        for (j = 0; (j < i + 1); j++)
            ret += base64_chars.c_str()[charArray4[j]];

        while ((i++ < 3))
            ret += '=';
    }

    return ret;
}

ByteVector DecodeBase64(String encodedString)
{
    int inLen = encodedString.length();
    int i = 0;
    int j = 0;
    int in_ = 0;
    unsigned char charArray4[4], charArray3[3];
    ByteVector ret;

    while (inLen-- && (encodedString.c_str()[in_] != '=') && IsBase64(encodedString.c_str()[in_]))
    {
        charArray4[i++] = encodedString.c_str()[in_];
        in_++;

        if (i == 4)
        {
            for (i = 0; i < 4; i++)
                charArray4[i] = base64_chars.find(charArray4[i]);

            charArray3[0] = (charArray4[0] << 2u) + ((charArray4[1] & 0x30u) >> 4u);
            charArray3[1] = ((charArray4[1] & 0xfu) << 4u) + ((charArray4[2] & 0x3cu) >> 2u);
            charArray3[2] = ((charArray4[2] & 0x3u) << 6u) + charArray4[3];

            for (i = 0; (i < 3); i++)
                ret.push_back(charArray3[i]);

            i = 0;
        }
    }

    if (i)
    {
        for (j = i; j <4; j++)
            charArray4[j] = 0;

        for (j = 0; j <4; j++)
            charArray4[j] = base64_chars.find(charArray4[j]);

        charArray3[0] = (charArray4[0] << 2u) + ((charArray4[1] & 0x30u) >> 4u);
        charArray3[1] = ((charArray4[1] & 0xfu) << 4u) + ((charArray4[2] & 0x3cu) >> 2u);
        charArray3[2] = ((charArray4[2] & 0x3u) << 6u) + charArray4[3];

        for (j = 0; (j < i - 1); j++)
            ret.push_back(charArray3[j]);
    }

    return ret;
}


Base64OutputArchive::Base64OutputArchive()
    : BinaryOutputArchive(static_cast<VectorBuffer&>(*this))
{
}

String Base64OutputArchive::GetBase64() const
{
    return EncodeBase64(GetBuffer());
}

Base64InputArchive::Base64InputArchive(const String& base64)
    : VectorBuffer(DecodeBase64(base64))
    , BinaryInputArchive(static_cast<VectorBuffer&>(*this))
{
}

}
