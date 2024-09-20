#pragma once

#include <Se/IO/AbstractFile.hpp>

#include <SeResource/Resource.h>
#include <SeResource/JSONValue.h>

#include <functional>
#include <string>

namespace Se
{

class Archive;

/// JSON document resource.
class JSONFile : public Resource
{

public:
    /// Construct.
    explicit JSONFile() : Resource() {};
    /// Destruct.
    ~JSONFile() override {};

    /// Load resource from stream. May be called from a worker thread. Return true if successful.
    bool BeginLoad(Deserializer& source) override;
    /// Save resource with default indentation (one tab). Return true if successful.
    bool Save(Serializer& dest) const override;
    /// Save resource with user-defined indentation, only the first character (if any) of the string is used and the length of the string defines the character count. Return true if successful.
    bool Save(Serializer& dest, const String& indendation) const;

    /// Save/load objects using Archive serialization.
    /// @{
    bool SaveObjectCallback(const std::function<void(Archive&)> serializeValue);
    bool LoadObjectCallback(const std::function<void(Archive&)> serializeValue) const;
    template <class T, class ... Args> bool SaveObject(const char* name, const T& object, Args &&... args);
    template <class T, class ... Args> bool LoadObject(const char* name, T& object, Args &&... args) const;
    // bool SaveObject(const Object& object) { return SaveObject(object.GetTypeName().c_str(), object); }
    // bool LoadObject(Object& object) const { return LoadObject(object.GetTypeName().c_str(), object); }
    /// @}

    /// Deserialize from a string. Return true if successful.
    bool FromString(const String& source);
    /// Save to a string.
    String ToString(const String& indendation = "\t") const;

    /// Return root value.
    JSONValue& GetRoot() { return root_; }
    /// Return root value.
    const JSONValue& GetRoot() const { return root_; }

private:
    /// JSON root value.
    JSONValue root_;
};

String ToPrettyString(const JSONValue& json, const String& indendation = "\t");

template <class T, class ... Args>
bool JSONFile::SaveObject(const char* name, const T& object, Args &&... args)
{
    return SaveObjectCallback([&](Archive& archive) {
                                  SerializeValue(archive, name, const_cast<T&>(object), std::forward<Args>(args)...);
                              });
}

template <class T, class ... Args>
bool JSONFile::LoadObject(const char* name, T& object, Args &&... args) const
{
    return LoadObjectCallback([&](Archive& archive) {
                                  SerializeValue(archive, name, object, std::forward<Args>(args)...);
                              });
}

}
