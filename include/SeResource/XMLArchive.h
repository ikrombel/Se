// Copyright (c) 2017-2020 the rbfx project.

#pragma once

#include <SeArc/ArchiveBase.hpp>
#include <SeResource/XMLElement.h>
#include <SeResource/XMLFile.h>


namespace Se
{

/// Base archive for XML serialization.
template <class BlockType, bool IsInputBool>
class XMLArchiveBase : public ArchiveBaseT<BlockType, IsInputBool, true>
{
public:
    /// @name Archive implementation
    /// @{
    String GetName() const { return xmlFile_ ? xmlFile_->GetName() : ""; }
    /// @}

protected:
    XMLArchiveBase(XMLElement element, const XMLFile* xmlFile, bool serializeRootName)
            : ArchiveBaseT<BlockType, IsInputBool, true>()
            , rootElement_(element)
            , xmlFile_(xmlFile)
            , serializeRootName_(serializeRootName)
    {
        SE_ASSERT(element, "");
    }

    XMLElement rootElement_{};
    const bool serializeRootName_{};

private:
    /// XML file.
    const XMLFile* xmlFile_{};
};

/// XML attribute reference for input and output archives.
class XMLAttributeReference
{
public:
    XMLAttributeReference(XMLElement element, const char* attribute) : element_(element), attribute_(attribute) {}

    XMLElement GetElement() const { return element_; }
    const char* GetAttributeName() const { return attribute_; }

private:
    XMLElement element_;
    const char* attribute_{};
};

/// XML output archive block. Internal.
class XMLOutputArchiveBlock : public ArchiveBlockBase
{
public:
    XMLOutputArchiveBlock(const char* name, ArchiveBlockType type, XMLElement blockElement, unsigned sizeHint);

    XMLElement CreateElement(ArchiveBase& archive, const char* elementName);
    XMLAttributeReference CreateElementOrAttribute(ArchiveBase& archive, const char* elementName);

    bool IsUnorderedAccessSupported() const { return type_ == ArchiveBlockType::Unordered; }
    bool HasElementOrBlock(const char* name) const { return false; }
    void Close(ArchiveBase& archive);

private:
    XMLElement blockElement_{};

    /// Expected block size (for arrays).
    unsigned expectedElementCount_{M_MAX_UNSIGNED};
    /// Number of elements in block.
    unsigned numElements_{};

    /// Set of used names for checking.
    std::vector<String> usedNames_{};
};

/// XML output archive.
class SE_API XMLOutputArchive : public XMLArchiveBase<XMLOutputArchiveBlock, false>
{
public:
    /// Construct from element.
    XMLOutputArchive(XMLElement element, XMLFile* xmlFile = nullptr, bool serializeRootName = false)
            : XMLArchiveBase(element, xmlFile, serializeRootName)
    {
    }
    /// Construct from file.
    explicit XMLOutputArchive(XMLFile* xmlFile)
            : XMLArchiveBase(xmlFile->GetOrCreateRoot(rootBlockName), xmlFile, true)
    {}

    /// @name Archive implementation
    /// @{
    void BeginBlock(const char* name, unsigned& sizeHint, bool safe, ArchiveBlockType type) final;

    void Serialize(const char* name, bool& value) final;
    void Serialize(const char* name, signed char& value) final;
    void Serialize(const char* name, unsigned char& value) final;
    void Serialize(const char* name, short& value) final;
    void Serialize(const char* name, unsigned short& value) final;
    void Serialize(const char* name, int& value) final;
    void Serialize(const char* name, unsigned int& value) final;
    void Serialize(const char* name, long long& value) final;
    void Serialize(const char* name, unsigned long long& value) final;
    void Serialize(const char* name, float& value) final;
    void Serialize(const char* name, double& value) final;
    void Serialize(const char* name, String& value) final;

    void SerializeBytes(const char* name, void* bytes, unsigned size) final;
    void SerializeVLE(const char* name, unsigned& value) final;
    /// @}

private:
    XMLAttributeReference CreateElementOrAttribute(const char* name);

    String tempString_;
};

/// XML input archive block. Internal.
class XMLInputArchiveBlock : public ArchiveBlockBase
{
public:
    XMLInputArchiveBlock(const char* name, ArchiveBlockType type, XMLElement blockElement);

    /// Return size hint.
    unsigned CalculateSizeHint() const;
    /// Read current child and move to the next one.
    XMLElement ReadElement(ArchiveBase& archive, const char* elementName);
    /// Read attribute (for Unordered blocks only) or the element and move to the next one.
    XMLAttributeReference ReadElementOrAttribute(ArchiveBase& archive, const char* elementName);

    bool IsUnorderedAccessSupported() const { return type_ == ArchiveBlockType::Unordered; }
    bool HasElementOrBlock(const char* name) const { return blockElement_.HasChild(name) || blockElement_.HasAttribute(name); }
    void Close(ArchiveBase& archive) {}

private:
    /// Block element.
    XMLElement blockElement_{};
    /// Next child to read.
    XMLElement nextChild_{};
};

/// XML input archive.
class SE_API XMLInputArchive : public XMLArchiveBase<XMLInputArchiveBlock, true>
{
public:
    /// Construct from file.
    explicit XMLInputArchive(XMLFile* xmlFile)
            : XMLArchiveBase(xmlFile->GetRoot(), xmlFile, false)
    {}

        /// Construct from element.
    XMLInputArchive(XMLElement element, const XMLFile* xmlFile = nullptr, bool serializeRootName = false)
        : XMLArchiveBase(element, xmlFile, serializeRootName)
    {
    }

    /// @name Archive implementation
    /// @{
    void BeginBlock(const char* name, unsigned& sizeHint, bool safe, ArchiveBlockType type) final;

    void Serialize(const char* name, bool& value) final;
    void Serialize(const char* name, signed char& value) final;
    void Serialize(const char* name, unsigned char& value) final;
    void Serialize(const char* name, short& value) final;
    void Serialize(const char* name, unsigned short& value) final;
    void Serialize(const char* name, int& value) final;
    void Serialize(const char* name, unsigned int& value) final;
    void Serialize(const char* name, long long& value) final;
    void Serialize(const char* name, unsigned long long& value) final;
    void Serialize(const char* name, float& value) final;
    void Serialize(const char* name, double& value) final;
    void Serialize(const char* name, String& value) final;

    void SerializeBytes(const char* name, void* bytes, unsigned size) final;
    void SerializeVLE(const char* name, unsigned& value) final;
    /// @}

private:
    XMLAttributeReference ReadElementOrAttribute(const char* name);
};

}
