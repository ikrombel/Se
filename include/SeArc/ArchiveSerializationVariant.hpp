#pragma once

#include <SeMath/ArchiveMath.hpp>
//#include <GFrost/Core/Context.h>
//#include <GFrost/Core/StringUtils.h>
#if __has_include("SeEngine/Core/Variant.h")
#include <SeEngine/Core/Variant.h>
#include <SeEngine/Core/VariantCurve.h>

#include <SeArc/ArchiveSerialization.hpp>
#include <SeArc/ArchiveSerializationBasic.hpp>
#include <SeArc/ArchiveSerializationContainerSTD.hpp>

#include <Se/IO/VectorBuffer.h>

namespace Se
{

void SerializeValue(Archive& archive, const char* name, VariantType& value);
void SerializeValue(Archive& archive, const char* name, Variant& value);
void SerializeVariantInBlock(Archive& archive, Variant& value);

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
            const StringVector chunks = value.split(';', true);
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

    template <class T>
    inline void SerializeVariantAsType(Archive& archive, const char* name, Variant& variant)
    {
        if (archive.IsInput())
        {
            T value{};
            SerializeValue(archive, name, value);
            variant = value;
        }
        else
        {
            const T& value = variant.Get<T>();
            SerializeValue(archive, name, const_cast<T&>(value));
        }
    }
}

/// Serialize type of the Variant.
inline void SerializeValue(Archive& archive, const char* name, VariantType& value)
{
    SerializeEnum(archive, name, value, Variant::GetTypeNameList());
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

/// Serialize value of the Variant.
//SE_ENGINE_API void SerializeVariantAsType(Archive& archive, const char* name, Variant& value, VariantType variantType);
inline void SerializeVariantAsType(Archive& archive, const char* name, Variant& value, VariantType variantType)
{
    static_assert(MAX_VAR_TYPES == 30, "Update me");
    switch (variantType)
    {
    case VAR_NONE:
        return;

    case VAR_INT:
        Detail::SerializeVariantAsType<int>(archive, name, value);
        return;

    case VAR_BOOL:
        Detail::SerializeVariantAsType<bool>(archive, name, value);
        return;

    case VAR_FLOAT:
        Detail::SerializeVariantAsType<float>(archive, name, value);
        return;

    case VAR_VECTOR2:
        Detail::SerializeVariantAsType<Vector2>(archive, name, value);
        return;

    case VAR_VECTOR3:
        Detail::SerializeVariantAsType<Vector3>(archive, name, value);
        return;

    case VAR_VECTOR4:
        Detail::SerializeVariantAsType<Vector4>(archive, name, value);
        return;

    case VAR_QUATERNION:
        Detail::SerializeVariantAsType<Quaternion>(archive, name, value);
        return;

    case VAR_COLOR:
        Detail::SerializeVariantAsType<Color>(archive, name, value);
        return;

    case VAR_STRING:
        Detail::SerializeVariantAsType<String>(archive, name, value);
        return;

    case VAR_RESOURCEREF:
        Detail::SerializeVariantAsType<ResourceRef>(archive, name, value);
        return;

    case VAR_RESOURCEREFLIST:
        Detail::SerializeVariantAsType<ResourceRefList>(archive, name, value);
        return;

    case VAR_INTRECT:
        Detail::SerializeVariantAsType<IntRect>(archive, name, value);
        return;

    case VAR_INTVECTOR2:
        Detail::SerializeVariantAsType<IntVector2>(archive, name, value);
        return;

    case VAR_MATRIX3:
        Detail::SerializeVariantAsType<Matrix3>(archive, name, value);
        return;

    case VAR_MATRIX3X4:
        Detail::SerializeVariantAsType<Matrix3x4>(archive, name, value);
        return;

    case VAR_MATRIX4:
        Detail::SerializeVariantAsType<Matrix4>(archive, name, value);
        return;

    case VAR_DOUBLE:
        Detail::SerializeVariantAsType<double>(archive, name, value);
        return;

    case VAR_RECT:
        Detail::SerializeVariantAsType<Rect>(archive, name, value);
        return;

    case VAR_INTVECTOR3:
        Detail::SerializeVariantAsType<IntVector3>(archive, name, value);
        return;

    case VAR_INT64:
        Detail::SerializeVariantAsType<long long>(archive, name, value);
        return;

    case VAR_VARIANTCURVE:
        Detail::SerializeVariantAsType<VariantCurve>(archive, name, value);
        return;

    case VAR_BUFFER:
    {
        if (archive.IsInput() && !value.GetBufferPtr())
            value = ByteVector();

        auto ptr = value.GetBufferPtr();
        SE_ASSERT(ptr, "Cannot save Variant of mismatching type");
        LibSTD::SerializeVectorAsBytes(archive, name, *ptr);
        return;
    }

    case VAR_VARIANTVECTOR:
    {
        if (archive.IsInput() && !value.GetVariantVectorPtr())
            value = VariantVector{};

        auto ptr = value.GetVariantVectorPtr();
        SE_ASSERT(ptr, "Cannot save Variant of mismatching type");
        LibSTD::SerializeVectorAsObjects(archive, name, *ptr);
        return;
    }

    case VAR_VARIANTMAP:
    {
        if (archive.IsInput() && !value.GetVariantMapPtr())
            value = VariantMap{};

        auto ptr = value.GetVariantMapPtr();
        SE_ASSERT(ptr, "Cannot save Variant of mismatching type");
        SerializeVariantMap(archive, name, *ptr);
        return;
    }

    case VAR_STRINGVECTOR:
    {
        if (archive.IsInput() && !value.GetStringVectorPtr())
            value = StringVector{};

        auto ptr = value.GetStringVectorPtr();
        SE_ASSERT(ptr, "Cannot save Variant of mismatching type");
        LibSTD::SerializeVectorAsObjects(archive, name, *ptr);
        return;
    }

    case VAR_CUSTOM:
    {
        // Even if loading, value should be initialized to default value.
        // It's the only way to know type.
        CustomVariantValue* ptr = value.GetCustomVariantValuePtr();
        SE_ASSERT(ptr, "Cannot serialize CustomVariant of unknown type");

        ptr->Serialize(archive, name);
        return;
    }

    case VAR_STRINGVARIANTMAP:
    {
        if (archive.IsInput() && !value.GetStringVariantMapPtr())
            value = StringVariantMap();

        auto ptr = value.GetStringVariantMapPtr();
        SE_ASSERT(ptr, "Cannot save Variant of mismatching type");
        SerializeVariantMap(archive, name, *ptr);
        return;
    }

    case VAR_VOIDPTR:
    case VAR_PTR:
        throw ArchiveException("'{}/{}' has unsupported variant type");

    case MAX_VAR_TYPES:
    default:
        SE_ASSERT(0, "Unknown variant type");
    }
}



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


// template <class T, class TSerializer = Detail::DefaultSerializer>
// void SerializeVariantMap(Archive& archive, const char* name, T& map, const char* element = "element",
//     const TSerializer& serializeValue = TSerializer{})
// {
//     using KeyType = typename T::key_type;
//     using ValueType = typename T::mapped_type;

//     if constexpr (std::is_same_v<ValueType, Variant> && std::is_same_v<TSerializer, Detail::DefaultSerializer>)
//     {
//         auto block = archive.OpenArrayBlock(name, map.size());
//         if (archive.IsInput())
//         {
//             map.clear();
//             for (unsigned i = 0; i < block.GetSizeHint(); ++i)
//             {
//                 auto elementBlock = archive.OpenUnorderedBlock(element);
//                 KeyType key{};
//                 SerializeValue(archive, "key", key);
//                 SerializeVariantInBlock(archive, map[key]);
//             }
//         }
//         else
//         {
//             for (const auto& [key, value] : map)
//             {
//                 auto elementBlock = archive.OpenUnorderedBlock(element);
//                 SerializeValue(archive, "key", const_cast<KeyType&>(key));
//                 SerializeVariantInBlock(archive, const_cast<Variant&>(value));
//             }
//         }
//     }
// }

}

#endif