#pragma once

#include <SeArc/Archive.hpp>
#include <SeArc/ArchiveSerializationVariant.h>
#include <SeResource/BinaryArchive.h>
#include <Se/IO/VectorBuffer.h>
#include <SeResource/Resource.h>

#include <functional>

namespace Se
{

/// Resource for generic binary file.
class SE_API BinaryFile : public Resource
{
//    SE_OBJECT(BinaryFile, Resource);

public:
    /// Construct empty.
    explicit BinaryFile();
    /// Destruct.
    ~BinaryFile() override;
    // /// Register object factory.
    // static void RegisterObject(Context* context);

    inline static String GetTypeStatic() { return "BinaryFile"; }

    /// Load resource from stream. May be called from a worker thread. Return true if successful.
    bool BeginLoad(Deserializer& source) override;
    /// Save resource to a stream.
    bool Save(Serializer& dest) const override;

    /// Save/load objects using Archive serialization.
    /// @{
    bool SaveObjectCallback(const std::function<void(Archive&)> serializeValue);
    bool LoadObjectCallback(const std::function<void(Archive&)> serializeValue) const;
    template <class T, class ... Args>
    bool SaveObject(const char* name, const T& object, Args &&... args);
    template <class T, class ... Args>
    bool LoadObject(const char* name, T& object, Args &&... args) const;

    // bool SaveObject(const Object& object) {
    //     return SaveObject(object.GetTypeName().CString(), object);
    // }
    // bool LoadObject(Object& object) const {
    //     return LoadObject(object.GetTypeName().CString(), object);
    // }
    /// @}

    /// Clear data.
    void Clear();
    /// Set data.
    void SetData(const ByteVector& data);
    /// Return immutable data.
    const ByteVector& GetData() const;
    /// Return immutable data as string view.
    String GetText() const;
    /// Return data as text lines.
    StringVector ReadLines() const;

    /// Cast to Serializer.
    Serializer& AsSerializer() { return buffer_; }
    /// Cast to Deserializer.
    Deserializer& AsDeserializer() { return buffer_; }

private:
    VectorBuffer buffer_;
};

template <class T, class ... Args>
bool BinaryFile::SaveObject(const char* name, const T& object, Args &&... args)
{
    return SaveObjectCallback([&](Archive& archive) {
              SerializeValue(archive, name, const_cast<T&>(object), std::forward<Args>(args)...);
          });
}

template <class T, class ... Args>
bool BinaryFile::LoadObject(const char* name, T& object, Args &&... args) const
{
    return LoadObjectCallback([&](Archive& archive) {
              SerializeValue(archive, name, object, std::forward<Args>(args)...);
          });
}

}
