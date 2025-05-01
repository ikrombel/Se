// Copyright (c) 2017-2020 the rbfx project.

//#include <GFrost/Core/StringUtils.h>
#include <Se/Profiler.hpp>
#include <SeArc/ArchiveSerialization.hpp>
#include "XMLArchive.h"

namespace Se
{

/// Name of internal key attribue of Map block.
static const char* keyAttribute = "key";

XMLOutputArchiveBlock::XMLOutputArchiveBlock(
        const char* name, ArchiveBlockType type, XMLElement blockElement, unsigned sizeHint)
        : ArchiveBlockBase(name, type)
        , blockElement_(blockElement)
{
    if (type_ == ArchiveBlockType::Array)
        expectedElementCount_ = sizeHint;
}

XMLElement XMLOutputArchiveBlock::CreateElement(ArchiveBase& archive, const char* elementName)
{
    SE_ASSERT(numElements_ < expectedElementCount_, "");

    bool containInUsedNames = std::find(usedNames_.begin(), usedNames_.end(), elementName) != usedNames_.end();

    if (type_ == ArchiveBlockType::Unordered && containInUsedNames)
        throw archive.DuplicateElementException(elementName);

    XMLElement element = blockElement_.CreateChild(elementName);
    ++numElements_;

    if (type_ == ArchiveBlockType::Unordered)
        usedNames_.emplace_back(elementName);

    SE_ASSERT(element, "");
    return element;
}

XMLAttributeReference XMLOutputArchiveBlock::CreateElementOrAttribute(ArchiveBase& archive, const char* elementName)
{
    if (type_ != ArchiveBlockType::Unordered)
    {
        XMLElement child = CreateElement(archive, elementName);
        return { child, "value" };
    }

    // Special case for Unordered
    if (std::find(usedNames_.begin(), usedNames_.end(), elementName) != usedNames_.end())
        throw archive.DuplicateElementException(elementName);

    ++numElements_;
    return { blockElement_, elementName };
}

void XMLOutputArchiveBlock::Close(ArchiveBase& archive)
{
    SE_ASSERT(expectedElementCount_ == M_MAX_UNSIGNED || numElements_ == expectedElementCount_, "");
}

void XMLOutputArchive::BeginBlock(const char* name, unsigned& sizeHint, bool safe, ArchiveBlockType type)
{
    CheckBeforeBlock(name);
    CheckBlockOrElementName(name);

    if (stack_.empty())
    {
        if (serializeRootName_)
            rootElement_.SetName(name);
        stack_.push_back(Block{ name, type, rootElement_, sizeHint });
        return;
    }

    XMLElement blockElement = GetCurrentBlock().CreateElement(*this, name);
    Block block{ name, type, blockElement, sizeHint };
    stack_.push_back(block);
}

void XMLOutputArchive::SerializeBytes(const char* name, void* bytes, unsigned size)
{
    XMLAttributeReference ref = CreateElementOrAttribute(name);
    BufferToHexString(tempString_, bytes, size);
    ref.GetElement().SetString(ref.GetAttributeName(), tempString_);
}

void XMLOutputArchive::SerializeVLE(const char* name, unsigned& value)
{
    XMLAttributeReference ref = CreateElementOrAttribute(name);
    ref.GetElement().SetUInt(ref.GetAttributeName(), value);
}

XMLAttributeReference XMLOutputArchive::CreateElementOrAttribute(const char* name)
{
    CheckBeforeElement(name);
    CheckBlockOrElementName(name);

    XMLOutputArchiveBlock& block = GetCurrentBlock();
    return GetCurrentBlock().CreateElementOrAttribute(*this, name);
}

// Generate serialization implementation (XML output)
#define SE_XML_OUT_IMPL(type, function) \
void XMLOutputArchive::Serialize(const char* name, type& value) \
{ \
    XMLAttributeReference ref = CreateElementOrAttribute(name); \
    ref.GetElement().function(ref.GetAttributeName(), value); \
}

SE_XML_OUT_IMPL(bool, SetBool);
SE_XML_OUT_IMPL(signed char, SetInt);
SE_XML_OUT_IMPL(short, SetInt);
SE_XML_OUT_IMPL(int, SetInt);
SE_XML_OUT_IMPL(long long, SetInt64);
SE_XML_OUT_IMPL(unsigned char, SetUInt);
SE_XML_OUT_IMPL(unsigned short, SetUInt);
SE_XML_OUT_IMPL(unsigned int, SetUInt);
SE_XML_OUT_IMPL(unsigned long long, SetUInt64);
SE_XML_OUT_IMPL(float, SetFloat);
SE_XML_OUT_IMPL(double, SetDouble);
SE_XML_OUT_IMPL(String, SetString);

#undef SE_XML_OUT_IMPL

XMLInputArchiveBlock::XMLInputArchiveBlock(const char* name, ArchiveBlockType type, XMLElement blockElement)
        : ArchiveBlockBase(name, type)
        , blockElement_(blockElement)
{
    if (blockElement_)
        nextChild_ = blockElement_.GetChild();
}

unsigned XMLInputArchiveBlock::CalculateSizeHint() const
{
    XMLElement child = blockElement_.GetChild();
    unsigned count = 0;
    while (child)
    {
        ++count;
        child = child.GetNext();
    }
    return count;
}

XMLElement XMLInputArchiveBlock::ReadElement(ArchiveBase& archive, const char* elementName)
{
    if (type_ != ArchiveBlockType::Unordered && !nextChild_)
        throw archive.ElementNotFoundException(elementName);

    XMLElement element;
    if (type_ == ArchiveBlockType::Unordered)
        element = blockElement_.GetChild(elementName);
    else
    {
        element = nextChild_;
        nextChild_ = nextChild_.GetNext();
    }

    if (!element)
        throw archive.ElementNotFoundException(elementName);

    return element;
}

XMLAttributeReference XMLInputArchiveBlock::ReadElementOrAttribute(ArchiveBase& archive, const char* elementName)
{
    if (type_ != ArchiveBlockType::Unordered)
    {
        XMLElement child = ReadElement(archive, elementName);
        return { child, "value" };
    }

    // Special case for Unordered
    if (!blockElement_.HasAttribute(elementName))
        throw archive.ElementNotFoundException(elementName);

    return { blockElement_, elementName };
}

void XMLInputArchive::BeginBlock(const char* name, unsigned& sizeHint, bool safe, ArchiveBlockType type)
{
    CheckBeforeBlock(name);
    CheckBlockOrElementName(name);

    // Open root block
    if (stack_.empty())
    {
        if (serializeRootName_ && rootElement_.GetName() != name)
            throw ElementNotFoundException(name);

        Block block{ name, type, rootElement_ };
        sizeHint = block.CalculateSizeHint();
        stack_.push_back(block);
        return;
    }

    XMLElement blockElement = GetCurrentBlock().ReadElement(*this, name);
    Block block{ name, type, blockElement };
    sizeHint = block.CalculateSizeHint();
    stack_.push_back(block);
}

void XMLInputArchive::SerializeBytes(const char* name, void* bytes, unsigned size)
{
    XMLAttributeReference ref = ReadElementOrAttribute(name);
    const String& value = ref.GetElement().GetAttribute(ref.GetAttributeName());
    ReadBytesFromHexString(name, value, bytes, size);
}

void XMLInputArchive::SerializeVLE(const char* name, unsigned& value)
{
    XMLAttributeReference ref = ReadElementOrAttribute(name);
    value = ref.GetElement().GetUInt(ref.GetAttributeName());
}

XMLAttributeReference XMLInputArchive::ReadElementOrAttribute(const char* name)
{
    CheckBeforeElement(name);
    CheckBlockOrElementName(name);

    return GetCurrentBlock().ReadElementOrAttribute(*this, name);
}

// Generate serialization implementation (XML input)
#define SE_XML_IN_IMPL(type, function) \
void XMLInputArchive::Serialize(const char* name, type& value) \
{ \
    XMLAttributeReference ref = ReadElementOrAttribute(name); \
    value = ref.GetElement().function(ref.GetAttributeName()); \
}

SE_XML_IN_IMPL(bool, GetBool);
SE_XML_IN_IMPL(signed char, GetInt);
SE_XML_IN_IMPL(short, GetInt);
SE_XML_IN_IMPL(int, GetInt);
SE_XML_IN_IMPL(long long, GetInt64);
SE_XML_IN_IMPL(unsigned char, GetUInt);
SE_XML_IN_IMPL(unsigned short, GetUInt);
SE_XML_IN_IMPL(unsigned int, GetUInt);
SE_XML_IN_IMPL(unsigned long long, GetUInt64);
SE_XML_IN_IMPL(float, GetFloat);
SE_XML_IN_IMPL(double, GetDouble);
SE_XML_IN_IMPL(String, GetAttribute);

#undef SE_XML_IN_IMPL

}
