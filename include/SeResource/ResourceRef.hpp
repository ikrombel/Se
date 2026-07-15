#pragma once

#include <Se/String.hpp>
#include <Se/Hash.hpp>

#if __has_include("SeArc/ArchiveSerialization.hpp")
#include <SeArc/ArchiveSerialization.hpp>
#define ARC_RESOURCREF
#endif

namespace Se
{

/// Typed resource reference.
struct ResourceRef
{
    /// Construct.
    ResourceRef() = default;

    /// Construct with type only and empty id.
    explicit ResourceRef(String type) :
        type_(type)
    {
    }

    // /// Construct with type and resource name.
    // ResourceRef(String type, const String& name) :
    //     type_(type),
    //     name_(name)
    // {
    // }

    /// Construct with type and resource name.
    ResourceRef(const String& type, const String& name) :
        type_(type),
        name_(name)
    {
    }

    /// Construct with type and resource name.
    ResourceRef(const char* type, const char* name) :
        type_(type),
        name_(name)
    {
    }

    /// Construct from another ResourceRef.
    ResourceRef(const ResourceRef& rhs) = default;

    /// Return hash value for HashSet & HashMap.
    Hash ToHash() const
    {
        Hash result = 0;
        hash_combine(result, make_hash(type_));
        hash_combine(result, make_hash(name_));
        return result;
    }

    /// Object type.
    String type_;
    /// Object name.
    String name_;

    /// Test for equality with another reference.
    bool operator ==(const ResourceRef& rhs) const { return type_ == rhs.type_ && name_ == rhs.name_; }

    /// Test for inequality with another reference.
    bool operator !=(const ResourceRef& rhs) const { return type_ != rhs.type_ || name_ != rhs.name_; }
};

/// %List of typed resource references.
struct ResourceRefList
{
    /// Construct.
    ResourceRefList() = default;

    /// Construct with type only.
    explicit ResourceRefList(String type) :
        type_(type)
    {
    }

    /// Construct with type and id list.
    ResourceRefList(String type, const StringVector& names) :
        type_(type),
        names_(names)
    {
    }

    /// Return hash value for HashSet & HashMap.
    unsigned ToHash() const
    {
        unsigned result = 0;
        hash_combine(result, make_hash(type_));
        for (const String& name : names_)
            hash_combine(result, make_hash(name));
        return result;
    }

    /// Object type.
    String type_;
    /// List of object names.
    StringVector names_;

    /// Test for equality with another reference list.
    bool operator ==(const ResourceRefList& rhs) const { 
        return type_ == rhs.type_ && names_ == rhs.names_; }

    /// Test for inequality with another reference list.
    bool operator !=(const ResourceRefList& rhs) const { 
        return type_ != rhs.type_ || names_ != rhs.names_; }
};

// namespace Datail {



// /// ResourceRefList to/from string.
// struct ResourceRefListStringCaster
// {
//     ea::string ToArchive(Archive& archive, const char* name, const ResourceRefList& value) const
//     {
//         return value.ToString(archive.GetContext());
//     }

//     ResourceRefList FromArchive(Archive& archive, const char* name, const ea::string& value) const
//     {
//         ea::vector<ea::string> chunks = value.split(';', true);
//         if (chunks.empty())
//             throw ArchiveException("Unexpected format of ResourceRefList '{}/{}'", archive.GetCurrentBlockPath(), name);

//         const ea::string typeName = ea::move(chunks[0]);
//         chunks.pop_front();

//         // Treat ";" as empty list
//         if (chunks.size() == 1 && chunks[0].empty())
//             chunks.clear();

//         return { StringHash{typeName}, chunks };
//     }
// };

// } // Datail


#ifdef ARC_RESOURCREF

namespace Detail {

/// ResourceRef to/from string.
struct ResourceRefStringCaster
{
    String ToArchive(Archive& archive, const char* name, const ResourceRef& value) const
    {
        return format("{};{}", value.type_, value.name_);
    }

    ResourceRef FromArchive(Archive& archive, const char* name, const String& value) const
    {
        const std::vector<String> chunks = value.split(';', true);
        if (chunks.size() != 2)
            throw ArchiveException("Unexpected format of ResourceRef '{}/{}'", archive.GetCurrentBlockPath(), name);

        return { chunks[0], chunks[1] };
    }
};

} // namespace Detail



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
#endif

#if 0
inline void SerializeValue(Archive& archive, const char* name, ResourceRefList& value)
{
    if (!archive.IsHumanReadable())
    {
        ArchiveBlock block = archive.OpenUnorderedBlock(name);
        SerializeValue(archive, "type", value.type_);
        SerializeVectorAsObjects(archive, "names", value.names_);
        return;
    }

    SerializeValueAsType<ea::string>(archive, name, value, Detail::ResourceRefListStringCaster{});
}
#endif

    
} // namespace Se
