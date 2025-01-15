#pragma once

//#include <GFrost/Precompiled.h>
//#include <GFrost/Core/Format.h>
//#include <GFrost/Core/StringUtils.h>

//#include "ArchiveSerialization.h"
#include <SeArc/ArchiveSerializationBasic.hpp>
#include <SeArc/ArchiveSerializationContainerSTD.hpp>

#include <string>
#include <vector>
#include <cstring>
//#include <ranges>

namespace Se
{

namespace Detail
{
    // inline std::vector<String> split(const String& str, const String& delim) {
    //     std::vector<String> ret;
    //     auto words = str.split(delim.c_str());
    //     for (const auto word : words)
    //         ret.push_back(word);
    //     return ret;
    // }


    inline String NumberArrayToString(float* values, unsigned size)
    {
        String result;
        for (unsigned i = 0; i < size; ++i)
        {
            if (i > 0)
                result += " ";
            result += std::to_string(values[i]);
        }
        return result;
    }

    inline String NumberArrayToString(int* values, unsigned size)
    {
        String result;
        for (unsigned i = 0; i < size; ++i)
        {
            if (i > 0)
                result += " ";
            result += std::to_string(values[i]);
        }
        return result;
    }

    inline std::size_t StringToNumberArray(const String& string, float* values, std::size_t maxSize)
    {
        auto elements = string.split(' ');
        const unsigned size = elements.size() < maxSize ? elements.size() : maxSize;
        for (unsigned i = 0; i < size; ++i)
            values[i] = atof(elements[i].c_str());

        return elements.size();
    }

    inline unsigned StringToNumberArray(const String& string, int* values, unsigned maxSize)
    {
        const auto elements = string.split(' ');
        const unsigned size = elements.size() < maxSize ? elements.size() : maxSize;
        for (unsigned i = 0; i < size; ++i)
            values[i] = atoi(elements[i].c_str());

        return elements.size();
    }

}

inline void SerializeValue(Archive& archive, const char* name, std::vector<String>& value)
{
    LibSTD::SerializeVectorAsObjects(archive, name, value);
}


// void SerializeValue(Archive& archive, const char* name, ResourceRefList& value)
// {
//     if (!archive.IsHumanReadable())
//     {
//         ArchiveBlock block = archive.OpenUnorderedBlock(name);
//         SerializeValue(archive, "type", value.type_);
//         LibSTD::SerializeVectorAsObjects(archive, "names", value.names_);
//         return;
//     }

//     SerializeValueAsType<String>(archive, name, value, Detail::ResourceRefListStringCaster{});
// }

}
