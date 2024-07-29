#pragma once

//#include <GFrost/Core/Assert.h>
#include <Se/NonCopyable.hpp>
#include <Se/String.hpp>
#include <Se/Exception.hpp>
#include <SeArc/Archive.hpp>

#include <cassert>

namespace Se
{

/// Exception thrown on I/O error on Archive serialization/deserialization.
/// Try to catch this exception outside of serialization code and don't leak it to user code.
/// Archive is generally not safe to use if ArchiveException has been thrown.
class ArchiveException : public RuntimeException
{
public:
    using RuntimeException::RuntimeException;
};



/// Archive implementation helper. Provides default Archive implementation for most cases.
class ArchiveBase : public Archive, public MovableNonCopyable
{
public:

    String GetName() const override { return String::EMPTY; }
    
    unsigned GetChecksum() override { return 0; }
    
    bool IsEOF() const final { return eof_; }
    
    void Flush() final { FlushDelayedException(); }

    unsigned SerializeVersion(unsigned version) final {
        SerializeVLE(versionElementName, version);
        return version;
    }
/// @}

/// @name Common exception factories
/// @{
ArchiveException IOFailureException(String elementName) const
{
    return ArchiveException("Unspecified I/O failure before '{}/{}'",
                            GetCurrentBlockPath().c_str(), elementName.c_str());
}

ArchiveException DuplicateElementException(String elementName) const
{
    return ArchiveException("'{}/{}' is serialized several times",
                            GetCurrentBlockPath().c_str(), elementName.c_str());
}

ArchiveException ElementNotFoundException(String elementName) const
{
    return ArchiveException("'{}/{}' is not found",
                            GetCurrentBlockPath().c_str(), elementName.c_str());
}

ArchiveException ElementNotFoundException(String elementName, unsigned elementIndex) const
{
    return ArchiveException("'{}/{}#{}' is not found",
                            GetCurrentBlockPath().c_str(), elementName.c_str(), elementIndex);
}

ArchiveException UnexpectedElementValueException(String elementName) const
{
    return ArchiveException("'{}/{}' has unexpected type",
                            GetCurrentBlockPath().c_str(), elementName.c_str());
}

ArchiveException UnexpectedEOFException(String elementName) const
{
    return ArchiveException("Unexpected end of file before '{}/{}'",
                            GetCurrentBlockPath().c_str(), elementName.c_str());
}
/// @}

protected:
    ArchiveBase() {}

    ~ArchiveBase() override {
        assert(!delayedException_ && "Archive::Flush was not called while having delayed exception");
    }

    static constexpr const char* rootBlockName = "Root";
    static constexpr const char* versionElementName = "Version";

    void SetDelayedException(std::exception_ptr ptr) {
        if (!delayedException_)
            delayedException_ = ptr;
    }
    
    void FlushDelayedException() {
        if (delayedException_)
        {
            std::exception_ptr ptr = delayedException_;
            delayedException_ = nullptr;
            std::rethrow_exception(ptr);
        }
    }
    
    void CheckIfNotEOF(String elementName) const {
        if (eof_)
            throw UnexpectedEOFException(elementName);
    }
    
    void CheckBlockOrElementName(String elementName) const {
        assert(ValidateName(elementName));
    }
    
    void CloseArchive() { eof_ = true; }
    
    void ReadBytesFromHexString(String elementName, const String& string, void* bytes, unsigned size)
    {
        static thread_local std::vector<unsigned char> tempBuffer;

        if (!HexStringToBuffer(tempBuffer, string))
            throw UnexpectedElementValueException(elementName);

        if (size != tempBuffer.size())
            throw UnexpectedElementValueException(elementName);

        std::copy(tempBuffer.begin(), tempBuffer.end(), static_cast<unsigned char*>(bytes));
    }

private:
    std::exception_ptr delayedException_;
    bool eof_{};
};

/// Base implementation of ArchiveBlock. May contain inline blocks.
class ArchiveBlockBase {
public:
    ArchiveBlockBase(const char* name, ArchiveBlockType type)
            : name_(name)
            , type_(type)
    {
    }

    String GetName() const { return name_; }
    ArchiveBlockType GetType() const { return type_; }

    /// @name Manage inline blocks
    /// @{
    void OpenInlineBlock() { ++inlineBlockDepth_; }
    void CloseInlineBlock()
    {
        assert(inlineBlockDepth_ > 0);
        --inlineBlockDepth_;
    }
    bool HasOpenInlineBlock() const { return inlineBlockDepth_; }
    /// @}

    /// @name To be implemented
    /// @{
    bool IsUnorderedAccessSupported() const = delete;
    bool HasElementOrBlock(const char* name) const = delete;
    void Close() = delete;
    /// @}

protected:
    const String name_;
    const ArchiveBlockType type_{};

    unsigned inlineBlockDepth_{};
};

/// Archive implementation helper (template). Provides default block-dependent Archive implementation for most cases.
template <class BlockType, bool IsInputBool, bool IsHumanReadableBool>
class ArchiveBaseT : public ArchiveBase {
    friend BlockType;

public:
    /// @name Archive implementation
    /// @{
    bool IsInput() const final { return IsInputBool; }

    bool IsHumanReadable() const final { return IsHumanReadableBool; }

    bool IsUnorderedAccessSupportedInCurrentBlock() const final
    {
        return !stack_.empty() && GetCurrentBlock().IsUnorderedAccessSupported();
    }

    bool HasElementOrBlock(const char* name) const final
    {
        if constexpr (IsInputBool)
        {
            CheckIfRootBlockOpen();
            return GetCurrentBlock().HasElementOrBlock(name);
        }
        else
        {
            assert(0 && "Archive::HasElementOrBlock is not supported for Output Archive");
            return false;
        }
    }

    String GetCurrentBlockPath() const final
    {
        String result;
        for (const Block& block : stack_)
        {
            if (!result.empty())
                result += "/";
            const auto& blockName = block.GetName();
            result += blockName;
            if (block.HasOpenInlineBlock())
                result += "/?";
        }
        return result;
    }

    void EndBlock() noexcept override
    {
        assert(!stack_.empty());

        // Close inline block if possible
        Block& currentBlock = GetCurrentBlock();
        if (currentBlock.HasOpenInlineBlock())
        {
            currentBlock.CloseInlineBlock();
            return;
        }

        // Close block normally
        try
        {
            currentBlock.Close(*this);
        }
        catch (const ArchiveException&)
        {
            SetDelayedException(std::current_exception());
        }

        stack_.pop_back();
        if (stack_.empty())
            CloseArchive();
    }
    /// @}

protected:
    using ArchiveBase::ArchiveBase;

    using Block = BlockType;

    Block& GetCurrentBlock() { return stack_.back(); }

    const Block& GetCurrentBlock() const { return stack_.back(); }

    void CheckIfRootBlockOpen() const { 
        assert(!stack_.empty() && "Root block must be opened before serialization"); }

    void CheckBeforeBlock(const char* elementName)
    {
        FlushDelayedException();
        CheckIfNotEOF(elementName);
    }

    void CheckBeforeElement(const char* elementName)
    {
        FlushDelayedException();
        CheckIfNotEOF(elementName);
        CheckIfRootBlockOpen();
    }

    std::vector<Block> stack_;
};

}