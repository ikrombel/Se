#pragma once

#include <Se/String.hpp>
#include <Se/Hash.hpp>

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

    
} // namespace Se
