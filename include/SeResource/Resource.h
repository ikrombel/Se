#pragma once

#include <Se/Export.hpp>
#include <Se/Algorithms.hpp>
#include <Se/StringHash.hpp>
#include <Se/Timer.h>
#include <Se/Hash.hpp>
// #include <SeArc/Archive.hpp>
// #include <SeArc/ArchiveSerialization.hpp>
#include <SeVFS/FileIdentifier.h>
// #include <SeArcJson/JSONValue.h>

#include <array>
#include <unordered_map>

#include <optional>

namespace Se
{

class Deserializer;
class Serializer;
class XMLElement;

/// Internal file format of Resource.
enum class InternalResourceFormat
{
    /// Resource uses custom serialization logic. Format is unknown.
    Unknown,
    /// Resource is serialized as JSON or JSON Archive.
    Json,
    /// Resource is serialized as XML or XML Archive.
    Xml,
    /// Resource is serialized as binary Archive.
    Binary
};

/// Size of the magic number for binary resources.
static constexpr unsigned BinaryMagicSize = 4;
using BinaryMagic = std::array<unsigned char, BinaryMagicSize>;
SE_API extern const BinaryMagic DefaultBinaryMagic;

/// Peek into resource file and determine its internal format.
/// It's optimized for the case when the file is either Binary, JSON or XML.
/// Deserializer is left in the same state as it was before the call.
SE_API InternalResourceFormat PeekResourceFormat(
    Deserializer& source, BinaryMagic binaryMagic = DefaultBinaryMagic);

/// Asynchronous loading state of a resource.
enum AsyncLoadState
{
    /// No async operation in progress.
    ASYNC_DONE = 0,
    /// Queued for asynchronous loading.
    ASYNC_QUEUED = 1,
    /// In progress of calling BeginLoad() in a worker thread.
    ASYNC_LOADING = 2,
    /// BeginLoad() succeeded. EndLoad() can be called in the main thread.
    ASYNC_SUCCESS = 3,
    /// BeginLoad() failed.
    ASYNC_FAIL = 4
};

/// Base class for resources.
class SE_API Resource
{

public:
    /// Resource reloading started. E_RELOADSTARTED
    Signal<> onReloadStarted;
    /// Resource reloading finished successfully. E_RELOADFINISHED
    Signal<> onReloadFinished;
    /// Resource reloading failed. E_RELOADFAILED
    Signal<> onReloadFailed;

    // /// Resource reloading failed. E_RELOADFAILED
    // static std::unordered_map<String, std::shared_ptr<Resource>(*)(void)> registered_;

    // static void Register(const String& resourceType, std::shared_ptr<Resource> (*func)(void))
    // {
    //     if (std::find(registered_.begin(), registered_.end(), resourceType) != registered_.end()) {
    //         return;
    //     }
    //     registered_[resourceType] = func;
    // }

    // static std::shared_ptr<Resource> Create(const String& resourceType)
    // {
    //     if (std::find(registered_.begin(), registered_.end(), resourceType) == registered_.end()) {
    //         SE_LOG_ERROR("Uknown Resource type: {}", resourceType);
    //         return nullptr;
    //     }
    //     return std::make_shared<Resource>(registered_[resourceType]);
    // }


    /// Construct.
    explicit Resource(const String& typeName);

    virtual ~Resource() = default;

    /// Load resource by reference.
    //static Resource* LoadFromCache(StringHash type, const String& name);

    /// Load resource synchronously. Call both BeginLoad() & EndLoad() and return true if both succeeded.
    bool Load(Deserializer& source);
    /// Load resource from stream. May be called from a worker thread. Return true if successful.
    virtual bool BeginLoad(Deserializer& source);
    /// Finish resource loading. Always called from the main thread. Return true if successful.
    virtual bool EndLoad();
    /// Save resource. Return true if successful.
    virtual bool Save(Serializer& dest) const;

    /// Load resource from file.
    bool LoadFile(const FileIdentifier& fileName);
    /// Save resource to file.
    virtual bool SaveFile(const FileIdentifier& fileName) const;

    /// Set name.
    void SetName(const String& name);
    /// Set memory use in bytes, possibly approximate.
    void SetMemoryUse(unsigned size);
    /// Reset last used timer.
    void ResetUseTimer();
    /// Set the asynchronous loading state. Called by ResourceCache. Resources in the middle of asynchronous loading are not normally returned to user.
    void SetAsyncLoadState(AsyncLoadState newState);
    /// Set absolute file name.
    void SetAbsoluteFileName(const String& fileName) { absoluteFileName_ = fileName; }

    String GetType() const { return type_; }

    StringHash GetTypeHash() { return StringHash(type_); }

    /// Return name.
    const String& GetName() const { return name_; }

    /// Return name hash.
    StringHash GetNameHash() const { return nameHash_; }

    /// Return memory use in bytes, possibly approximate.
    unsigned GetMemoryUse() const { return memoryUse_; }

    /// Return time since last use in milliseconds. If referred to elsewhere than in the resource cache, returns always zero.
    unsigned GetUseTimer();

    /// Return the asynchronous loading state.
    AsyncLoadState GetAsyncLoadState() const { return asyncLoadState_; }

    /// Return absolute file name.
    const String& GetAbsoluteFileName() const { return absoluteFileName_; }

private:
    /// Name.
    String name_;
    /// Name hash.
    StringHash nameHash_;
    /// Absolute file name.
    String absoluteFileName_;
    /// Last used timer.
    Timer useTimer_;
    /// Memory use in bytes.
    unsigned memoryUse_;
    /// Asynchronous loading state.
    AsyncLoadState asyncLoadState_;
    /// Resource type name.
    String type_;
};

#ifdef DISABLED

/// Base class for simple resource that uses Archive serialization.
class SE_API SimpleResource : public Resource
{

public:
    /// Construct.
    explicit SimpleResource();

    /// Force override of SerializeInBlock.
    void SerializeInBlock(Archive& archive) override = 0;

    /// Save resource in specified internal format.
    bool Save(Serializer& dest, InternalResourceFormat format) const;
    /// Save file with specified internal format.
    bool SaveFile(const FileIdentifier& fileName, InternalResourceFormat format) const;

    /// Implement Resource.
    /// @{
    bool BeginLoad(Deserializer& source) override;
    bool EndLoad() override;
    bool Save(Serializer& dest) const override;
    bool SaveFile(const FileIdentifier& fileName) const override;
    /// @}

protected:
    /// Binary archive magic word. Should be 4 bytes.
    virtual BinaryMagic GetBinaryMagic() const { return DefaultBinaryMagic; }
    /// Root block name. Used for XML serialization only.
    virtual const char* GetRootBlockName() const { return "resource"; }
    /// Default internal resource format on save.
    virtual InternalResourceFormat GetDefaultInternalFormat() const { return InternalResourceFormat::Json; }
    /// Try to load legacy XML format, whatever it is.
    virtual bool LoadLegacyXML(const XMLElement& source) { return false; }

private:
    std::optional<InternalResourceFormat> loadFormat_;
};

/// Base class for resources that support arbitrary metadata stored. Metadata serialization shall be implemented in derived classes.
class SE_API ResourceWithMetadata : public Resource
{

public:
    /// Construct.
    explicit ResourceWithMetadata() : Resource(context) {}

    /// Add new metadata variable or overwrite old value.
    void AddMetadata(const String& name, const Variant& value);
    /// Remove metadata variable.
    void RemoveMetadata(const String& name);
    /// Remove all metadata variables.
    void RemoveAllMetadata();
    /// Return all metadata keys.
    const StringVector& GetMetadataKeys() const { return metadataKeys_; }
    /// Return metadata variable.
    const Variant& GetMetadata(const String& name) const;
    /// Return whether the resource has metadata.
    bool HasMetadata() const;

protected:
    /// Load metadata from <metadata> children of XML element.
    void LoadMetadataFromXML(const XMLElement& source);
    /// Load metadata from JSON array.
    void LoadMetadataFromJSON(const JSONArray& array);
    /// Save as <metadata> children of XML element.
    void SaveMetadataToXML(XMLElement& destination) const;
    /// Copy metadata from another resource.
    void CopyMetadata(const ResourceWithMetadata& source);

private:
    /// Animation metadata variables.
    VariantMap metadata_;
    /// Animation metadata keys.
    StringVector metadataKeys_;
};

/// Serialize reference to a resource.
template <class T, std::enable_if_t<std::is_base_of_v<Resource, T>, int> = 0>
inline void SerializeResource(Archive& archive, const char* name, SharedPtr<T>& value, ResourceRef& resourceRef)
{
    SerializeValue(archive, name, resourceRef);
    if (archive.IsInput())
        value = SharedPtr<T>(dynamic_cast<T*>(Resource::LoadFromCache(archive.GetContext(), resourceRef.type_, resourceRef.name_)));
}

inline const String& GetResourceName(Resource* resource)
{
    return resource ? resource->GetName() : String::EMPTY;
}

inline StringHash GetResourceType(Resource* resource, StringHash defaultType)
{
    return resource ? resource->GetType() : defaultType;
}

inline ResourceRef GetResourceRef(Resource* resource, StringHash defaultType)
{
    return ResourceRef(GetResourceType(resource, defaultType), GetResourceName(resource));
}

template <class T> Vector<String> GetResourceNames(const Vector<SharedPtr<T> >& resources)
{
    Vector<String> ret;
    ret.Resize(resources.Size());
    for (unsigned i = 0; i < resources.Size(); ++i)
        ret[i] = GetResourceName(resources[i]);

    return ret;
}

template <class T> ResourceRefList GetResourceRefList(const Vector<SharedPtr<T> >& resources)
{
    return ResourceRefList(T::GetTypeStatic(), GetResourceNames(resources));
}

#endif // DISABLED

} // namespace Se

namespace std {
    
template <>
struct hash<Se::Resource> {
    size_t operator()(const Se::Resource& res) const {
        std::size_t hash{0};
        Se::hash_combine(hash, res.GetName());
        //Se::hash_combine(hash, res.GetSize());
        return hash;
    }
};

} // namespace std