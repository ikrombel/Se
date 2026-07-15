// Copyright (c) 2017-2020 the rbfx project.

#pragma once

#include <SeArc/ArchiveSerializationBasic.hpp>
//#include <SeArc/ArchiveSerializationVariant.hpp>

#include <algorithm>
#include <vector>

namespace Se
{

namespace Detail
{

/// Serialize tie of vectors of the same size. Each tie of elements is serialized as separate block.
template <class T, class U, std::size_t... Is>
inline void SerializeVectorTieSTD(Archive& archive, const char* name, T& vectorTie, const char* element, const U& serializeValue, std::index_sequence<Is...>)
{
    const std::size_t sizes[] = { std::get<Is>(vectorTie).size()... };

    std::size_t numElements = sizes[0];
    auto block = archive.OpenArrayBlock(name, numElements);

    if (archive.IsInput())
    {
        numElements = block.GetSizeHint();
        (std::get<Is>(vectorTie).clear(), ...);
        (std::get<Is>(vectorTie).resize(numElements), ...);
    }
    else
    {
        if (std::find_if(std::begin(sizes), std::end(sizes), [numElements](std::size_t size) { return size != numElements; }) != std::end(sizes))
            throw ArchiveException("Vectors of '{}/{}' have mismatching sizes", archive.GetCurrentBlockPath().c_str(), name);
    }

    for (std::size_t i = 0; i < numElements; ++i)
    {
        const auto elementTuple = std::tie(std::get<Is>(vectorTie)[i]...);
        serializeValue(archive, element, elementTuple);
    }
}

SEARC_TYPE_TRAIT(IsVectorTypeSTD, (\
    std::declval<T&>().size(),\
    std::declval<T&>().data(),\
    std::declval<T&>().clear(),\
    std::declval<T&>().resize(0u),\
    std::declval<typename T::value_type&>() = std::declval<T&>()[0u]\
));

SEARC_TYPE_TRAIT(IsMapTypeSTD, (\
    std::declval<T&>().size(),\
    std::declval<T&>().clear(),\
    std::declval<typename T::mapped_type&>() = std::declval<T&>()[std::declval<typename T::key_type>()]\
));

SEARC_TYPE_TRAIT(IsSetTypeSTD, (\
    std::declval<T&>().size(),\
    std::declval<T&>().clear(),\
    std::declval<T&>().emplace(std::declval<typename T::key_type>()),\
    std::declval<typename T::key_type&>() = *std::declval<T&>().begin()\
));

}

namespace LibSTD {
/// Serialize vector with standard interface. Content is serialized as separate objects.
template <class T, class TSerializer = Detail::DefaultSerializer>
void SerializeVectorAsObjects(Archive& archive, const char* name, T& vector, const char* element = "element",
    const TSerializer& serializeValue = TSerializer{})
{
    using ValueType = typename T::value_type;

    unsigned numElements = vector.size();
    auto block = archive.OpenArrayBlock(name, numElements);

    if (archive.IsInput())
    {
        numElements = block.GetSizeHint();
        vector.clear();
        vector.resize(numElements);
    }

    for (unsigned i = 0; i < numElements; ++i)
        serializeValue(archive, element, vector[i]);
}

/// Serialize array with standard interface (compatible with std::span, std::array, etc). Content is serialized as separate objects.
template <class T, class TSerializer = Detail::DefaultSerializer>
void SerializeArrayAsObjects(Archive& archive, const char* name, T& array, const char* element = "element",
    const TSerializer& serializeValue = TSerializer{})
{
    const unsigned numElements = array.size();
    auto block = archive.OpenArrayBlock(name, numElements);

    if (archive.IsInput())
    {
        if (numElements != block.GetSizeHint())
            throw ArchiveException("'{}/{}' has unexpected array size", archive.GetCurrentBlockPath().c_str(), name);
    }

    for (unsigned i = 0; i < numElements; ++i)
        serializeValue(archive, element, array[i]);
}

template <class T, class TSerializer = Detail::DefaultSerializer>
void SerializeVectorTieAsObjects(Archive& archive, const char* name, T vectorTie, const char* element = "element",
    const TSerializer& serializeValue = TSerializer{})
{
    static constexpr auto tupleSize = std::tuple_size_v<T>;
    Detail::SerializeVectorTieSTD(archive, name, vectorTie, element, serializeValue, std::make_index_sequence<tupleSize>{});
}

/// Serialize vector with standard interface. Content is serialized as bytes.
template <class T>
void SerializeVectorAsBytes(Archive& archive, const char* name, T& vector)
{
    using ValueType = typename T::value_type;
    static_assert(std::is_standard_layout<ValueType>::value, "Type should have standard layout to safely use byte serialization");
    static_assert(std::is_trivially_copyable<ValueType>::value, "Type should be trivially copyable to safely use byte serialization");

    const bool loading = archive.IsInput();
    ArchiveBlock block = archive.OpenUnorderedBlock(name);

    unsigned sizeInBytes{};

    if (!loading)
        sizeInBytes = vector.size() * sizeof(ValueType);

    archive.SerializeVLE("size", sizeInBytes);

    if (loading)
    {
        if (sizeInBytes % sizeof(ValueType) != 0)
            throw ArchiveException("'{}/{}' has unexpected size in bytes", archive.GetCurrentBlockPath().c_str(), name);
        vector.resize(sizeInBytes / sizeof(ValueType));
    }

    archive.SerializeBytes("data", vector.data(), sizeInBytes);
}

/// Serialize vector in the best possible format.
template <class T>
void SerializeVector(Archive& archive, const char* name, T& vector, const char* element = "element")
{
    using ValueType = typename T::value_type;
    static constexpr bool standardLayout = std::is_standard_layout<ValueType>::value;
    static constexpr bool triviallyCopyable = std::is_trivially_copyable<ValueType>::value;

    if constexpr (standardLayout && triviallyCopyable)
    {
        if (!archive.IsHumanReadable())
        {
            LibSTD::SerializeVectorAsBytes(archive, name, vector);
            return;
        }
    }

    LibSTD::SerializeVectorAsObjects(archive, name, vector, element);
}

/// Serialize map or hash map with with standard interface.
template <class T, class TSerializer = Detail::DefaultSerializer>
void SerializeMap(Archive& archive, const char* name, T& map, const char* element = "element",
    const TSerializer& serializeValue = TSerializer{})
{
    using KeyType = typename T::key_type;
    using ValueType = typename T::mapped_type;

    // if constexpr (std::is_same_v<ValueType, Variant> && std::is_same_v<TSerializer, Detail::DefaultSerializer>)
    // {
    //     auto block = archive.OpenArrayBlock(name, map.size());
    //     if (archive.IsInput())
    //     {
    //         map.clear();
    //         for (unsigned i = 0; i < block.GetSizeHint(); ++i)
    //         {
    //             auto elementBlock = archive.OpenUnorderedBlock(element);
    //             KeyType key{};
    //             SerializeValue(archive, "key", key);
    //             SerializeVariantInBlock(archive, map[key]);
    //         }
    //     }
    //     else
    //     {
    //         for (const auto& [key, value] : map)
    //         {
    //             auto elementBlock = archive.OpenUnorderedBlock(element);
    //             SerializeValue(archive, "key", const_cast<KeyType&>(key));
    //             SerializeVariantInBlock(archive, const_cast<Variant&>(value));
    //         }
    //     }
    // }
    // else
    {
        auto block = archive.OpenArrayBlock(name, map.size());
        if (archive.IsInput())
        {
            map.clear();
            for (unsigned i = 0; i < block.GetSizeHint(); ++i)
            {
                auto elementBlock = archive.OpenUnorderedBlock(element);
                KeyType key{};
                serializeValue(archive, "key", key);
                serializeValue(archive, "value", map[key]);
            }
        }
        else
        {
            for (const auto& [key, value] : map)
            {
                auto elementBlock = archive.OpenUnorderedBlock(element);
                serializeValue(archive, "key", const_cast<KeyType&>(key));
                serializeValue(archive, "value", const_cast<ValueType&>(value));
            }
        }
    }
}

/// Serialize set or hash set with standard interface.
template <class T, class TSerializer = Detail::DefaultSerializer>
void SerializeSet(Archive& archive, const char* name, T& set, const char* element = "element",
    const TSerializer& serializeValue = TSerializer{})
{
    using ValueType = typename T::value_type;

    auto block = archive.OpenArrayBlock(name, set.size());
    if (archive.IsInput())
    {
        set.clear();
        for (unsigned i = 0; i < block.GetSizeHint(); ++i)
        {
            ValueType value{};
            serializeValue(archive, element, value);
            set.emplace(std::move(value));
        }
    }
    else
    {
        for (const auto& value : set)
            serializeValue(archive, element, const_cast<ValueType&>(value));
    }
}

} // STD

/// Aliases for SerializeValue.
/// @{
template <class T, std::enable_if_t<Detail::IsVectorTypeSTD<T>::value, int> = 0>
void SerializeValue(Archive& archive, const char* name, T& vector) { LibSTD::SerializeVector(archive, name, vector); }
template <class T, std::enable_if_t<Detail::IsMapTypeSTD<T>::value, int> = 0>
void SerializeValue(Archive& archive, const char* name, T& map) { LibSTD::SerializeMap(archive, name, map); }
template <class T, std::enable_if_t<Detail::IsSetTypeSTD<T>::value, int> = 0>
void SerializeValue(Archive& archive, const char* name, T& set) {   LibSTD::SerializeSet(archive, name, set); }
/// @}

}
