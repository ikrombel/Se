// Copyright (c) 2008-2019 the GFrost project.


#include <SeResource/Resource.h>

#include <Se/Profiler.hpp>

#include <Se/Console.hpp>
#include <Se/Finally.hpp>
#include <Se/Thread.h>
#include <Se/IO/File.h>
#include <Se/IO/FileSystem.h>


#include <SeVFS/VirtualFileSystem.h>

// #include <GFrost/Resource/XMLElement.h>
// #include <GFrost/Resource/XMLArchive.h>
// #include <GFrost/Resource/XMLFile.h>
// #include <GFrost/Resource/JSONFile.h>
// #include <GFrost/Resource/JSONArchive.h>
// #include <GFrost/Resource/BinaryFile.h>
// #include <GFrost/Resource/ResourceCache.h>


namespace Se
{

const BinaryMagic DefaultBinaryMagic{{'\0', 'B', 'I', 'N'}};

InternalResourceFormat PeekResourceFormat(Deserializer& source, BinaryMagic binaryMagic)
{
    const auto basePos = source.Tell();
    const auto guard = MakeFinally([&]() { source.Seek(basePos); });
    const auto isBlank = [](char c) { return std::isspace(static_cast<unsigned char>(c)); };

    BinaryMagic magic;
    const unsigned count = source.Read(magic.data(), BinaryMagicSize);

    // It's binary file only if it starts with magic word
    if (count == BinaryMagicSize)
    {
        if (magic == binaryMagic)
            return InternalResourceFormat::Binary;
    }

    // It's XML file only if it starts with "<"
    if (count == BinaryMagicSize)
    {
        const auto iter = std::find(std::begin(magic), std::end(magic), '<');
        if (iter != std::end(magic) && std::all_of(std::begin(magic), iter, isBlank))
            return InternalResourceFormat::Xml;
    }

    // It's JSON file only if it starts with "{"
    if (count >= 2)
    {
        const auto iter = std::find(std::begin(magic), std::end(magic), '{');
        if (iter != std::end(magic) && std::all_of(std::begin(magic), iter, isBlank))
            return InternalResourceFormat::Json;
    }

    // If starts with unexpected symbols, it's unknown
    if (!std::all_of(std::begin(magic), std::end(magic), isBlank))
        return InternalResourceFormat::Unknown;

    // It still may be XML or JSON file, warn user about performance penalty and peek more data
    SE_LOG_WARNING(
        "File starts with whitespace, peeking more data to determine format. "
        "It may cause performance penalty.");

    while (!source.IsEof())
    {
        const char c = source.ReadUByte();
        if (c == '<')
            return InternalResourceFormat::Xml;
        if (c == '{')
            return InternalResourceFormat::Json;
        if (!isBlank(c))
            return InternalResourceFormat::Unknown;
    }

    return InternalResourceFormat::Unknown;
}

Resource::Resource(const String& typeName) :
    memoryUse_(0),
    asyncLoadState_(ASYNC_DONE),
    type_(typeName)
{

}


bool Resource::Load(Deserializer& source)
{
    // Because BeginLoad() / EndLoad() can be called from worker threads, where profiling would be a no-op,
    // create a type name -based profile block here
    SE_PROFILE_C("Load", PROFILER_COLOR_RESOURCES);
    String eventName = format("{}::Load(\"{}\")", GetType(), GetName());
    SE_PROFILE_ZONENAME(eventName);

    // If we are loading synchronously in a non-main thread, behave as if async loading (for example use
    // GetTempResource() instead of GetResource() to load resource dependencies)
    SetAsyncLoadState(Thread::IsMainThread() ? ASYNC_DONE : ASYNC_LOADING);
    bool success = BeginLoad(source);
    if (success)
        success &= EndLoad();
    SetAsyncLoadState(ASYNC_DONE);

    return success;
}

bool Resource::BeginLoad(Deserializer& source)
{
    // This always needs to be overridden by subclasses
    return false;
}

bool Resource::EndLoad()
{
    // If no GPU upload step is necessary, no override is necessary
    return true;
}

bool Resource::Save(Serializer& dest) const
{
    SE_LOG_ERROR("Save not supported for {}", this->GetType());
    return false;
}

bool Resource::LoadFile(const FileIdentifier& fileName)
{
    auto vfs = VirtualFileSystem::Get();
    auto file = vfs->OpenFile(fileName, FILE_READ);
    return file && Load(*file);
}

bool Resource::SaveFile(const FileIdentifier& fileName) const
{
    auto vfs = VirtualFileSystem::Get();
    auto file = vfs->OpenFile(fileName, FILE_WRITE);
    return file && Save(*file);
}

void Resource::SetName(const String& name)
{
    name_ = name;
    nameHash_ = name;
}

void Resource::SetMemoryUse(unsigned size)
{
    memoryUse_ = size;
}

void Resource::ResetUseTimer()
{
    useTimer_.Reset();
}

void Resource::SetAsyncLoadState(AsyncLoadState newState)
{
    asyncLoadState_ = newState;
}

unsigned Resource::GetUseTimer()
{
    // // If more references than the resource cache, return always 0 & reset the timer
    // if (Refs() > 1)
    // {
    //     useTimer_.Reset();
    //     return 0;
    // }
    // else
        return useTimer_.GetMSec(false);
}

// SimpleResource begin
#ifdef DISABLED

SimpleResource::SimpleResource()
    : Resource()
{
}

bool SimpleResource::Save(Serializer& dest, InternalResourceFormat format) const
{
    const auto binaryMagic = GetBinaryMagic();

    try
    {
        switch (format)
        {
        case InternalResourceFormat::Json:
        {
            JSONFile jsonFile(context_);
            JSONOutputArchive archive{jsonFile.GetRoot(), &jsonFile};
            SerializeValue(archive, GetRootBlockName(), const_cast<SimpleResource&>(*this));
            return jsonFile.Save(dest);
        }
        case InternalResourceFormat::Xml:
        {
            XMLFile xmlFile(context_);
            XMLOutputArchive archive{context_, xmlFile.GetOrCreateRoot(GetRootBlockName()), &xmlFile};
            SerializeValue(archive, GetRootBlockName(), const_cast<SimpleResource&>(*this));
            return xmlFile.Save(dest);
        }
        case InternalResourceFormat::Binary:
        {
            dest.Write(binaryMagic.data(), BinaryMagicSize);

            BinaryOutputArchive archive{context_, dest};
            SerializeValue(archive, GetRootBlockName(), const_cast<SimpleResource&>(*this));
            return true;
        }
        default:
        {
            GFROST_ASSERT(false);
            return false;
        }
        }
    }
    catch (const ArchiveException& e)
    {
        SE_LOG_ERROR("Cannot save SimpleResource: ", e.what());
        return false;
    }
}

bool SimpleResource::SaveFile(const FileIdentifier& fileName, InternalResourceFormat format) const
{
    auto vfs = VirtualFileSystem::Get();
    auto file = vfs->OpenFile(fileName, FILE_WRITE);
    return file && Save(*file, format);
}

bool SimpleResource::BeginLoad(Deserializer& source)
{
    const auto binaryMagic = GetBinaryMagic();

    try
    {
        const auto format = PeekResourceFormat(source, binaryMagic);
        switch (format)
        {
        case InternalResourceFormat::Json:
        {
            JSONFile jsonFile(context_);
            if (!jsonFile.Load(source))
                return false;

            JSONInputArchive archive{context_, jsonFile.GetRoot(), &jsonFile};
            SerializeValue(archive, GetRootBlockName(), *this);

            loadFormat_ = format;
            return true;
        }
        case InternalResourceFormat::Xml:
        {
            XMLFile xmlFile(context_);
            if (!xmlFile.Load(source))
                return false;

            XMLElement xmlRoot = xmlFile.GetRoot();
            if (xmlRoot.GetName() == GetRootBlockName())
            {
                XMLInputArchive archive{context_, xmlFile.GetRoot(), &xmlFile};
                SerializeValue(archive, GetRootBlockName(), *this);
            }
            else
            {
                if (!LoadLegacyXML(xmlRoot))
                    return false;
            }

            loadFormat_ = format;
            return true;
        }
        case InternalResourceFormat::Binary:
        {
            source.SeekRelative(BinaryMagicSize);

            BinaryInputArchive archive{context_, source};
            SerializeValue(archive, GetRootBlockName(), *this);

            loadFormat_ = format;
            return true;
        }
        default:
        {
           SE_LOG_ERROR("Unknown resource format");
            return false;
        }
        }
    }
    catch (const ArchiveException& e)
    {
        SE_LOG_ERROR("Cannot load SimpleResource: ", e.what());
        return false;
    }
}

bool SimpleResource::EndLoad()
{
    return true;
}

bool SimpleResource::Save(Serializer& dest) const
{
    return Save(dest, loadFormat_.value_or(GetDefaultInternalFormat()));
}

bool SimpleResource::SaveFile(const FileIdentifier& fileName) const
{
    return SaveFile(fileName, loadFormat_.value_or(GetDefaultInternalFormat()));
}

// SimpleResource end

void ResourceWithMetadata::AddMetadata(const String& name, const Variant& value)
{
    bool exists = metadata_.Contains(name);
    metadata_[StringHash(name)] = value;
    if (!exists)
        metadataKeys_.push_back(name);
}

void ResourceWithMetadata::RemoveMetadata(const String& name)
{
    metadata_.Erase(name);
    auto it = std::find(metadataKeys_.begin(), metadataKeys_.end(), name);
    if (it != metadataKeys_.end())
        metadataKeys_.erase(it);

}

void ResourceWithMetadata::RemoveAllMetadata()
{
    metadata_.Clear();
    metadataKeys_.clear();
}

const GFrost::Variant& ResourceWithMetadata::GetMetadata(const String& name) const
{
    auto it = metadata_.Find(name);
    if (it != metadata_.End())
        return it->second_;

    return Variant::EMPTY;
}

bool ResourceWithMetadata::HasMetadata() const
{
    return !metadata_.Empty();
}

void ResourceWithMetadata::LoadMetadataFromXML(const XMLElement& source)
{
    RemoveAllMetadata();
    for (XMLElement elem = source.GetChild("metadata"); elem; elem = elem.GetNext("metadata"))
        AddMetadata(elem.GetAttribute("name"), elem.GetVariant());
}

void ResourceWithMetadata::LoadMetadataFromJSON(const JSONArray& array)
{
    RemoveAllMetadata();
    for (unsigned i = 0; i < array.size(); i++)
    {
        const JSONValue& value = array.at(i);
        AddMetadata(value.Get("name").GetString(), value.GetVariant());
    }
}

void ResourceWithMetadata::SaveMetadataToXML(XMLElement& destination) const
{
    for (unsigned i = 0; i < metadataKeys_.size(); ++i)
    {
        XMLElement elem = destination.CreateChild("metadata");
        elem.SetString("name", metadataKeys_[i]);
        elem.SetVariant(GetMetadata(metadataKeys_[i]));
    }
}

void ResourceWithMetadata::CopyMetadata(const ResourceWithMetadata& source)
{
    metadata_ = source.metadata_;
    metadataKeys_ = source.metadataKeys_;
}

#endif // DISABLED

}