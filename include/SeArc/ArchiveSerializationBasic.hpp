// Copyright (c) 2017-2020 the rbfx project.

#pragma once

#include <type_traits>
#include <cstring>
#include <string>
#include <vector>
#include <span>
#include <numeric>
#include <limits>

#include <Se/StringHash.hpp>
#include <Se/Console.hpp>
#include <SeArc/ArchiveBase.hpp>

#include <Se/Span.hpp>



namespace Se
{

namespace Detail {

// GFROST_API String NumberArrayToString(float* values, unsigned size);
// GFROST_API String NumberArrayToString(int* values, unsigned size);
// GFROST_API unsigned StringToNumberArray(const String& string, float* values, unsigned maxSize);
// GFROST_API unsigned StringToNumberArray(const String& string, int* values, unsigned maxSize);

namespace Detail {
    std::size_t StringToNumberArray(const String& string, float* values, std::size_t maxSize);


    unsigned StringToNumberArray(const String& string, int* values, unsigned maxSize);
}

#if !GetStringListIndex
inline unsigned GetStringListIndex(const char* value, const std::vector<String>& strings, unsigned defaultIndex, bool caseSensitive = true)
{
    unsigned i = 0;

    while (!strings[i].empty())
    {
        bool isOk = caseSensitive ? strcmp(value, strings[i].c_str()) : strcasecmp(value, strings[i].c_str());
        if (!isOk)
            return i;
        ++i;
    }

    return defaultIndex;
}

inline unsigned GetStringListIndex(const char* value, const char* const * strings, unsigned defaultIndex, bool caseSensitive = true)
{
    unsigned i = 0;

    while (strings[i])
    {
        bool isOk = caseSensitive ? strcmp(value, strings[i]) : strcasecmp(value, strings[i]);
        if (!isOk)
            return i;
        ++i;
    }

    return defaultIndex;
}

inline unsigned GetStringListIndex(const String& value, Se::span<String> strings, unsigned defaultIndex, bool caseSensitive = true)
{
    unsigned i = 0;
    while (i < strings.size())
    {
        bool isOk = caseSensitive ? strcmp(value.data(), strings[i].data()) : strcasecmp(value.data(), strings[i].data());
        if (!isOk)
            return i;
        ++i;
    }

    return defaultIndex;
}

inline unsigned GetStringListIndex(const String& value, const std::vector<String>& strings, unsigned defaultIndex, bool caseSensitive = true)
{
    return GetStringListIndex(value.c_str(), strings, defaultIndex, caseSensitive);
}

#endif

#ifndef SEARC_TYPE_TRAIT
/// Helper macro that creates type trait. Expression should be any well-formed C++ expression over template type U.
#define SEARC_TYPE_TRAIT(name, expr) \
    template <typename U> struct name \
    { \
        template<typename T> static decltype((expr), std::true_type{}) func(std::remove_reference<T>*); \
        template<typename T> static std::false_type func(...); \
        using type = decltype(func<U>(nullptr)); \
        static constexpr bool value{ type::value }; \
    }
#endif


/// Serialize primitive array type as bytes or as formatted string.
template <unsigned N, class T>
inline void SerializePrimitiveArray(Archive& archive, const char* name, T& value)
{
    // Serialize as bytes if we don't care about readability
    if (!archive.IsHumanReadable())
    {
        archive.SerializeBytes(name, &value, sizeof(value));
        return;
    }

    // Serialize as string otherwise
    using ElementType = std::decay_t<decltype(*value.Data())>;
    ElementType* elements = const_cast<ElementType*>(value.Data());

    const bool loading = archive.IsInput();
    String string;

    if (!loading)
        string = NumberArrayToString(elements, N);

    archive.Serialize(name, string);

    if (loading)
        StringToNumberArray(string, elements, N);
}

/// Default callback for value serialization: use SerializeValue.
struct DefaultSerializer
{
    template <class T>
    void operator()(Archive& archive, const char* name, T& value) const { SerializeValue(archive, name, value); }
};

/// Default converter: any type to/from any type.
template <class InternalType, class ExternalType>
struct DefaultTypeCaster
{
    InternalType ToArchive(Archive& archive, const char* name, const ExternalType& value) const
    {
        return static_cast<InternalType>(value);
    }

    ExternalType FromArchive(Archive& archive, const char* name, const InternalType& value) const
    {
        return static_cast<ExternalType>(value);
    }
};

/// String hash to/from string.
struct StringHashCaster
{
    String stringHint_;

    String ToArchive(Archive& archive, const char* name, const StringHash& value) const
    {
        return String(stringHint_);
    }

    StringHash FromArchive(Archive& archive, const char* name, const String& value) const
    {
        return static_cast<StringHash>(value);
    }
};

/// Enum to/from string.
template <class T>
struct EnumStringCaster
{
    using UnderlyingInteger = std::underlying_type_t<T>;
    const char* const* enumConstants_{};

    String ToArchive(Archive& archive, const char* name, const T& value) const
    {
        return enumConstants_[static_cast<UnderlyingInteger>(value)];
    }

    T FromArchive(Archive& archive, const char* name, const String& value) const
    {
        return static_cast<T>(GetStringListIndex(value.c_str(), enumConstants_, 0));
    }
};

template <> struct EnumStringCaster<unsigned>
{
    const char* const* enumConstants_{};

    String ToArchive(Archive& archive, const char* name, const unsigned& value) const
    {
        return enumConstants_[value];
    }

    unsigned FromArchive(Archive& archive, const char* name, const String& value) const
    {
        return GetStringListIndex(value.c_str(), enumConstants_, 0);
    }
};

template <class T> struct EnumStringSafeCaster
{
    using UnderlyingInteger = std::underlying_type_t<T>;
    Se::span<String> enumConstants_;

    String ToArchive(Archive& archive, const char* name, const T& value) const
    {
        UnderlyingInteger index = static_cast<UnderlyingInteger>(value);
        if (index < 0 || index >= enumConstants_.size())
            return std::to_string(index);
        return String{enumConstants_[index]};
    }

    T FromArchive(Archive& archive, const char* name, const String& value) const
    {
        constexpr unsigned invalidIndex = std::numeric_limits<unsigned>::max();
        unsigned index = GetStringListIndex(value.c_str(), enumConstants_, invalidIndex);
        if (index == invalidIndex)
        {
            char* end;
            const unsigned long res = std::strtoul(value.c_str(), &end, 10);
            index = (res == std::numeric_limits<unsigned long>::max()) ? 0 : static_cast<unsigned>(res);
        }
        return static_cast<T>(index);
    }
};

template <>
struct EnumStringSafeCaster<unsigned>
{
    Se::span<String> enumConstants_;

    String ToArchive(Archive& archive, const char* name, const unsigned& value) const
    {
        if (value >= enumConstants_.size())
            return std::to_string(value).c_str();
        return enumConstants_[value];
    }

    unsigned FromArchive(Archive& archive, const char* name, const String& value) const
    {
        constexpr unsigned invalidIndex = std::numeric_limits<unsigned>::max();
        unsigned index = GetStringListIndex(value, enumConstants_, invalidIndex);
        if (index == invalidIndex)
        {
            char* end;
            const unsigned long res = std::strtoul(value.c_str(), &end, 10);
            index = (res == std::numeric_limits<unsigned long>::max()) ? 0 : static_cast<unsigned>(res);
        }
        return index;
    }
};

}

/// Check whether the object can be serialized from/to Archive block.
SEARC_TYPE_TRAIT(IsObjectSerializableInBlock, std::declval<T&>().SerializeInBlock(std::declval<Archive&>()));

/// Check whether the object has "empty" method.
SEARC_TYPE_TRAIT(IsObjectEmptyCheckableSTD, std::declval<T&>().empty());

SEARC_TYPE_TRAIT(IsObjectEmptyCheckable, std::declval<T&>().Empty());

/// Placeholder that represents any empty object as default value in SerializeOptionalValue.
struct EmptyObject
{
    template <class T>
    bool operator==(const T& rhs) const
    {
        if constexpr (IsObjectEmptyCheckableSTD<T>::value)
            return rhs.empty();
        else if constexpr (IsObjectEmptyCheckable<T>::value)
            return rhs.Empty();
        else
            return rhs == T{};
    }

    template <class T>
    explicit operator T() const { return T{}; }

    //String ToString() { return ""; }
};

/// Placeholder that doesn't represent any object in SerializeOptionalValue.
struct AlwaysSerialize
{
    template <class T>
    bool operator==(const T& rhs) const { return false; }
};

/// Placeholder object that can be serialized as nothing.
struct EmptySerializableObject
{
    void SerializeInBlock(Archive& archive) {}
};



/// @name Serialize primitive types
/// @{
inline void SerializeValue(Archive& archive, const char* name, bool& value) { archive.Serialize(name, value); }
inline void SerializeValue(Archive& archive, const char* name, signed char& value) { archive.Serialize(name, value); }
inline void SerializeValue(Archive& archive, const char* name, unsigned char& value) { archive.Serialize(name, value); }
inline void SerializeValue(Archive& archive, const char* name, short& value) { archive.Serialize(name, value); }
inline void SerializeValue(Archive& archive, const char* name, unsigned short& value) { archive.Serialize(name, value); }
inline void SerializeValue(Archive& archive, const char* name, int& value) { archive.Serialize(name, value); }
inline void SerializeValue(Archive& archive, const char* name, unsigned int& value) { archive.Serialize(name, value); }
inline void SerializeValue(Archive& archive, const char* name, long long& value) { archive.Serialize(name, value); }
inline void SerializeValue(Archive& archive, const char* name, unsigned long long& value) { archive.Serialize(name, value); }
inline void SerializeValue(Archive& archive, const char* name, float& value) { archive.Serialize(name, value); }
inline void SerializeValue(Archive& archive, const char* name, double& value) { archive.Serialize(name, value); }
inline void SerializeValue(Archive& archive, const char* name, String& value) { archive.Serialize(name, value); }
inline void SerializeValue(Archive& archive, const char* name, StringHash& value) { archive.Serialize(name, value.MutableValue()); }
/// @}

namespace Detail {

template <class T>
inline void SerializeAsString(Archive& archive, const char* name, T& variant)
{
    if (archive.IsInput())
    {
        String value{};
        SerializeValue(archive, name, value);
        variant = FromString<T>(value);
    }
    else
    {
        String value = variant.ToString();
        SerializeValue(archive, name, value);
    }
}

}

/// Serialize object with standard interface as value.
template <class T, std::enable_if_t<IsObjectSerializableInBlock<T>::value, int> = 0>
inline void SerializeValue(Archive& archive, const char* name, T& value)
{
    ArchiveBlock block = archive.OpenUnorderedBlock(name);
    value.SerializeInBlock(archive);
}

/// Serialize value as another type.
template <class T, class U, class TCaster = Detail::DefaultTypeCaster<T, U>>
void SerializeValueAsType(Archive& archive, const char* name, U& value, const TCaster& caster = TCaster{})
{
    const bool loading = archive.IsInput();
    T convertedValue{};

    if (!loading)
        convertedValue = caster.ToArchive(archive, name, value);

    SerializeValue(archive, name, convertedValue);

    if (loading)
        value = caster.FromArchive(archive, name, convertedValue);
}

/// Serialize enum as integer.
template <class T, std::enable_if_t<std::is_enum_v<T>, int> = 0>
inline void SerializeValue(Archive& archive, const char* name, T& value)
{
    using UnderlyingInteger = std::underlying_type_t<T>;
    SerializeValueAsType<UnderlyingInteger>(archive, name, value);
}

/// Serialize string hash as integer or as string.
inline void SerializeStringHash(Archive& archive, const char* name, StringHash& value, const String stringHint)
{
    if (!archive.IsHumanReadable())
        SerializeValue(archive, name, value);
    else
        SerializeValueAsType<String>(archive, name, value, Detail::StringHashCaster{stringHint});
}

/// Serialize enum as integer as integer or as string.
template <class EnumType, class UnderlyingInteger = std::underlying_type_t<EnumType>>
void SerializeEnum(Archive& archive, const char* name, EnumType& value, const char* const* enumConstants)
{
    assert(enumConstants);

    if (!archive.IsHumanReadable())
        SerializeValueAsType<UnderlyingInteger>(archive, name, value);
    else
        SerializeValueAsType<String>(archive, name, value, Detail::EnumStringCaster<EnumType>{enumConstants});
}

/// Serialize enum as integer or as string.
template <class EnumType, class UnderlyingInteger = std::underlying_type_t<EnumType>>
void SerializeEnum(Archive& archive, const char* name, EnumType& value, const Se::span<String> enumConstants)
{
    if (!archive.IsHumanReadable())
        SerializeValueAsType<UnderlyingInteger>(archive, name, value);
    else
        SerializeValueAsType<String>(
            archive, name, value, Detail::EnumStringSafeCaster<EnumType>{enumConstants});
}

/// Serialize optional element or block.
template <class T, class U = EmptyObject, class TSerializer = Detail::DefaultSerializer>
void SerializeStrictlyOptionalValue(Archive& archive, const char* name, T& value, const U& defaultValue = U{},
                                    const TSerializer& serializeValue = TSerializer{})
{
    const bool loading = archive.IsInput();

    if (!archive.IsUnorderedAccessSupportedInCurrentBlock())
    {
        ArchiveBlock block = archive.OpenUnorderedBlock(name);

        bool initialized{};

        if (!loading)
            initialized = !(defaultValue == value);

        SerializeValue(archive, "initialized", initialized);

        if (initialized)
            serializeValue(archive, "value", value);
        else if (loading)
            value = static_cast<T>(defaultValue);
    }
    else
    {
        const bool initialized = loading ? archive.HasElementOrBlock(name) : !(defaultValue == value);
        if (initialized)
            serializeValue(archive, name, value);
        else if (loading)
            value = static_cast<T>(defaultValue);
    }
}

/// Serialize element or block that's optional if archive type supports it.
/// There's no overhead on optionality if Archive doesn't support optional blocks.
template <class T, class U = EmptyObject, class TSerializer = Detail::DefaultSerializer>
void SerializeOptionalValue(Archive& archive, const char* name, T& value, const U& defaultValue = U{},
                            const TSerializer& serializeValue = TSerializer{})
{
    if (!archive.IsUnorderedAccessSupportedInCurrentBlock())
    {
        serializeValue(archive, name, value);
        return;
    }

    const bool loading = archive.IsInput();
    const bool initialized = loading ? archive.HasElementOrBlock(name) : !(defaultValue == value);
    if (initialized)
        serializeValue(archive, name, value);
    else if (loading)
    {
        // Don't try to cast from AlwaysSerialize
        if constexpr(!std::is_base_of_v<AlwaysSerialize, U>)
            value = static_cast<T>(defaultValue);
    }
}

/// Serialize pair type.
template <class T, class U, class TSerializer = Detail::DefaultSerializer>
void SerializeValue(
    Archive& archive, const char* name, std::pair<T, U>& value, const TSerializer& serializeValue = TSerializer{})
{
    const ArchiveBlock block = archive.OpenUnorderedBlock(name);
    serializeValue(archive, "first", value.first);
    serializeValue(archive, "second", value.second);
}

/// Wrapper that consumes ArchiveException and converts it to boolean status.
template <class T>
bool ConsumeArchiveException(const T& lambda, bool errorOnException = true)
{
    try
    {
        lambda();
        return true;
    }
    catch (const ArchiveException& e)
    {
        if (errorOnException)
            SE_LOG_ERROR("Serialization error: {}", e.what());
        else
            SE_LOG_DEBUG("Archive cannot be serialization: {}", e.what());
        return false;
    }
}

}