#pragma once

#include <SeArc/ArchiveSerializationBasic.hpp>
#include <SeArc/ArchiveSerializationContainerSTD.hpp>
//#include <GFrost/Core/Context.h>
//#include <GFrost/Core/StringUtils.h>
#if __has_include("SeEngine/Core/Variant.h")
#include <SeEngine/Core/Variant.h>




namespace Se
{

namespace Detail
{

    /// ResourceRef to/from string.
    struct ResourceRefStringCaster
    {
        String ToArchive(Archive& archive, const char* name, const ResourceRef& value) const
        {
            //Context* context = archive.GetContext();
            const String typeName = value.type_;
            return format("{};{}", typeName, value.name_);
        }

        ResourceRef FromArchive(Archive& archive, const char* name, const String& value) const
        {
            const std::vector<String> chunks = value.split(';', true);
            if (chunks.size() != 2)
                throw ArchiveException("Unexpected format of ResourceRef '{}/{}'",
                                       archive.GetCurrentBlockPath(), name);

            return {chunks.at(0), chunks.at(1)};
        }
    };

    /// ResourceRefList to/from string.
    struct ResourceRefListStringCaster
    {
        String ToArchive(Archive& archive, const char* name, const ResourceRefList& value) const
        {
            const String typeName = value.type_;
            return format("{};{}", typeName, String::joined(value.names_, ";"));
        }

        ResourceRefList FromArchive(Archive& archive, const char* name, const String& value) const
        {
            std::vector<String> chunks = value.split(';', true);
            if (chunks.empty())
                throw ArchiveException("Unexpected format of ResourceRefList '{}/{}'", archive.GetCurrentBlockPath(), name);

            const String typeName = std::move(chunks[0]);
            chunks.erase(chunks.begin()); //chunks.pop_front();

            // Treat ";" as empty list
            if (chunks.size() == 1 && chunks[0].empty())
                chunks.clear();

            return { String{typeName}, chunks };
        }
    };
}

/// Serialize type of the Variant.
inline void SerializeValue(Archive& archive, const char* name, VariantType& value)
{
    SerializeEnum(archive, name, value, Variant::GetTypeNameList());
}

/// Serialize value of the Variant.
SE_ENGINE_API void SerializeVariantAsType(Archive& archive, const char* name, Variant& value, VariantType variantType);

/// Serialize Variant in existing block.
inline void SerializeVariantInBlock(Archive& archive, Variant& value)
{
    VariantType variantType = value.GetType();
    SerializeValue(archive, "type", variantType);
    SerializeVariantAsType(archive, "value", value, variantType);
}

/// Serialize Variant.
inline void SerializeValue(Archive& archive, const char* name, Variant& value)
{
    ArchiveBlock block = archive.OpenUnorderedBlock(name);
    SerializeVariantInBlock(archive, value);
}

/// Serialize variant types.
/// @{
SE_ENGINE_API void SerializeValue(Archive& archive, const char* name, StringVector& value);
SE_ENGINE_API void SerializeValue(Archive& archive, const char* name, VariantVector& value);
SE_ENGINE_API void SerializeValue(Archive& archive, const char* name, VariantMap& value);
SE_ENGINE_API void SerializeValue(Archive& archive, const char* name, StringVariantMap& value);
SE_ENGINE_API void SerializeValue(Archive& archive, const char* name, ResourceRef& value);
SE_ENGINE_API void SerializeValue(Archive& archive, const char* name, ResourceRefList& value);
/// @}



inline void SerializeValue(Archive& archive, const char* name, VariantVector& value)
{
    LibSTD::SerializeVectorAsObjects(archive, name, value);
}

inline void SerializeValue(Archive& archive, const char* name, VariantMap& value)
{
    LibSTD::SerializeMap(archive, name, value);
}

// inline void SerializeValue(Archive& archive, const char* name, StringVariantMap& value)
// {
//     LibSTD::SerializeMap(archive, name, value);
// }

inline void SerializeValue(Archive& archive, const char* name, ResourceRef& value)
{
    if (!archive.IsHumanReadable())
    {
        ArchiveBlock block = archive.OpenUnorderedBlock(name);
        SerializeValue(archive, "type", value.type_);
        SerializeValue(archive, "name", value.name_);
        return;
    }

    SerializeValueAsType<String>(archive, name, value, Detail::ResourceRefStringCaster{});
}

inline void SerializeValue(Archive& archive, const char* name, ResourceRefList& value)
{
    if (!archive.IsHumanReadable())
    {
        ArchiveBlock block = archive.OpenUnorderedBlock(name);
        SerializeValue(archive, "type", value.type_);
        LibSTD::SerializeVectorAsObjects(archive, "names", value.names_);
        return;
    }

    SerializeValueAsType<String>(archive, name, value, Detail::ResourceRefListStringCaster{});
}


template <class T, class TSerializer = Detail::DefaultSerializer>
void SerializeVariantMap(Archive& archive, const char* name, T& map, const char* element = "element",
    const TSerializer& serializeValue = TSerializer{})
{
    using KeyType = typename T::key_type;
    using ValueType = typename T::mapped_type;

    if constexpr (std::is_same_v<ValueType, Variant> && std::is_same_v<TSerializer, Detail::DefaultSerializer>)
    {
        auto block = archive.OpenArrayBlock(name, map.size());
        if (archive.IsInput())
        {
            map.clear();
            for (unsigned i = 0; i < block.GetSizeHint(); ++i)
            {
                auto elementBlock = archive.OpenUnorderedBlock(element);
                KeyType key{};
                SerializeValue(archive, "key", key);
                SerializeVariantInBlock(archive, map[key]);
            }
        }
        else
        {
            for (const auto& [key, value] : map)
            {
                auto elementBlock = archive.OpenUnorderedBlock(element);
                SerializeValue(archive, "key", const_cast<KeyType&>(key));
                SerializeVariantInBlock(archive, const_cast<Variant&>(value));
            }
        }
    }
}

}

#endif