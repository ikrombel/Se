#pragma once

//#include <GFrost/Core/Exception.h>
//#include <GFrost/Core/NonCopyable.h>
//#include <GFrost/Core/StringUtils.h>

#include <algorithm>
#include <string>
#include <utility>
#include <string_view>

#include <Se/NonCopyable.hpp>
#include <Se/String.hpp>

namespace Se
{

class Archive;
//class Context;

/// Type of archive block.
/// - Default block type is Sequential.
/// - Other block types are used to improve quality of human-readable formats.
/// - Directly nested blocks and elements are called "items".
enum class ArchiveBlockType
{
    /// Sequential data block.
    /// - Items are saved and loaded in the order of serialization.
    /// - Names of items are optional and have no functional purpose.
    Sequential,
    /// Unordered data block.
    /// - Items are saved and loaded in the order of serialization.
    /// - Name must be unique for each item since it is used for input file lookup.
    /// - Input file may contain items of Unordered block in arbitrary order, if it is supported by actual archive format.
    /// - Syntax sugar for structures in human-readable and human-editable formats.
    /// - Best choice when number of items is known and fixed (e.g. structure or object).
    Unordered,
    /// Array data block.
    /// - Items are saved and loaded in the order of serialization.
    /// - Names of items are optional and have no functional purpose.
    /// - When reading, number of items is known when the block is opened.
    /// - When writing, number of items must be provided when the block is opened.
    /// - Syntax sugar for arrays in human-readable and human-editable formats.
    /// - Best choice when items are ordered and number of items is dynamic (e.g. dynamic array).
    Array,
    /// Map data block.
    /// - Order of serialization is not guaranteed.
    /// - Names of items are optional and have no functional purpose.
    /// - Before every item, key should be serialized via SerializeKey exactly once.
    /// - When reading, number of items is known when the block is opened.
    /// - When writing, number of items must be provided when the block is opened.
    /// - Key must be unique for each item.
    /// - Syntax sugar for maps in human-readable and human-editable formats.
    /// - Best choice when there are key-value pairs (e.g. map or hash map).
    Map,
};

/// Archive block scope guard.
class ArchiveBlock : private MovableNonCopyable {
public:
    /// Construct invalid.
    ArchiveBlock() = default;

    /// Construct valid.
    explicit ArchiveBlock(Archive &archive, unsigned sizeHint = 0) : archive_(&archive), sizeHint_(sizeHint) {}

    /// Destruct.
    ~ArchiveBlock();

    /// Move-construct.
    ArchiveBlock(ArchiveBlock &&other) { Swap(other); }

    /// Move-assign.
    ArchiveBlock &operator=(ArchiveBlock &&other) {
        Swap(other);
        return *this;
    }

    /// Swap with another.
    void Swap(ArchiveBlock &other) {
        std::swap(archive_, other.archive_);
        std::swap(sizeHint_, other.sizeHint_);
    }

    /// Convert to bool.
    explicit operator bool() const { return !!archive_; }

    /// Get size hint.
    unsigned GetSizeHint() const { return sizeHint_; }

private:
    /// Archive.
    Archive *archive_{};
    /// Block size.
    unsigned sizeHint_{};
};

/// Archive interface.
/// - Archive is a hierarchical structure of blocks and elements.
/// - Archive must have exactly one root block.
/// - Any block may contain other blocks or elements of any type.
/// - Any block or element may have name. Use C++ naming conventions for identifiers, arbitrary strings are not allowed. Name "key" is reserved.
/// - Unsafe block must not be closed until all the items are serialized.
class Archive {
public:
    virtual ~Archive() {}
    /// Return name of the archive if applicable.
    virtual String GetName() const = 0;
    /// Return a checksum if applicable.
    virtual unsigned GetChecksum() = 0;

    /// Whether the archive is in input mode.
    /// It is guaranteed that input archive doesn't read from any variable.
    /// It is guaranteed that output archive doesn't write to any variable.
    /// It is save to cast away const-ness when serializing into output archive.
    virtual bool IsInput() const = 0;
    /// Whether the human-readability is preferred over performance and output size.
    /// - Binary serialization is disfavored.
    /// - String hashes are serialized as strings, if possible.
    /// - Enumerators serialized as strings, if possible.
    /// - Simple compound types like Vector3 are serialized as formatted strings instead of blocks.
    virtual bool IsHumanReadable() const = 0;

    /// Return whether the unordered element access is supported in currently open block.
    /// Always false if current block is not Unordered. Always false for some archive types.
    virtual bool IsUnorderedAccessSupportedInCurrentBlock() const = 0;
    /// Return whether the element or block with given name is present.
    /// Should be called only if both IsInput and IsUnorderedAccessSupportedInCurrentBlock are true.
    virtual bool HasElementOrBlock(const char* name) const = 0;
    /// Whether the archive can no longer be serialized.
    virtual bool IsEOF() const = 0;
    /// Return current string stack.
    virtual String GetCurrentBlockPath() const = 0;

    /// Begin archive block.
    /// Size is required for Array blocks.
    /// It is guaranteed that errors occurred during serialization of the safe block don't affect data outside of the block.
    virtual void BeginBlock(const char* name, unsigned& sizeHint, bool safe, ArchiveBlockType type) = 0;
    /// End archive block.
    /// Failure usually means code error, for example one of the following:
    /// - Array or Map size doesn't match the number of serialized elements.
    /// - There is no corresponding BeginBlock call.
    virtual void EndBlock() = 0;
    /// Flush all pending events. Should be called at least once before destructor.
    virtual void Flush() = 0;

    /// \name Serialize keys
    /// \{

    virtual void Serialize(const char* name, bool& value) = 0;
    virtual void Serialize(const char* name, signed char& value) = 0;
    virtual void Serialize(const char* name, unsigned char& value) = 0;
    virtual void Serialize(const char* name, short& value) = 0;
    virtual void Serialize(const char* name, unsigned short& value) = 0;
    virtual void Serialize(const char* name, int& value) = 0;
    virtual void Serialize(const char* name, unsigned int& value) = 0;
    virtual void Serialize(const char* name, long long& value) = 0;
    virtual void Serialize(const char* name, unsigned long long& value) = 0;
    virtual void Serialize(const char* name, float& value) = 0;
    virtual void Serialize(const char* name, double& value) = 0;
    virtual void Serialize(const char* name, String& value) = 0;

    /// Serialize bytes. Size is not encoded and should be provided externally!
    virtual void SerializeBytes(const char* name, void* bytes, unsigned size) = 0;
    /// Serialize Variable Length Encoded unsigned integer, up to 29 significant bits.
    virtual void SerializeVLE(const char* name, unsigned& value) = 0;
    /// Serialize version number. 0 is invalid version.
    virtual unsigned SerializeVersion(unsigned version) = 0;

    /// Validate element or block name.
    static bool ValidateName(std::string_view name);

    /// Do BeginBlock and return the guard that will call EndBlock automatically on destruction.
    /// Return null block in case of error.
    ArchiveBlock OpenBlock(const char* name, unsigned sizeHint, bool safe, ArchiveBlockType type)
    {
        BeginBlock(name, sizeHint, safe, type);
        return ArchiveBlock{*this, sizeHint};
    }

    /// Open block helpers
    /// @{

    /// Open Sequential block. Will be automatically closed when returned object is destroyed.
    ArchiveBlock OpenSequentialBlock(const char* name) { return OpenBlock(name, 0, false, ArchiveBlockType::Sequential); }
    /// Open Unordered block. Will be automatically closed when returned object is destroyed.
    ArchiveBlock OpenUnorderedBlock(const char* name) { return OpenBlock(name, 0, false, ArchiveBlockType::Unordered); }
    /// Open Array block. Will be automatically closed when returned object is destroyed.
    ArchiveBlock OpenArrayBlock(const char* name, unsigned sizeHint = 0) { return OpenBlock(name, sizeHint, false, ArchiveBlockType::Array); }

    /// Open safe Sequential block. Will be automatically closed when returned object is destroyed.
    ArchiveBlock OpenSafeSequentialBlock(const char* name) { return OpenBlock(name, 0, true, ArchiveBlockType::Sequential); }
    /// Open safe Unordered block. Will be automatically closed when returned object is destroyed.
    ArchiveBlock OpenSafeUnorderedBlock(const char* name) { return OpenBlock(name, 0, true, ArchiveBlockType::Unordered); }

    /// @}
};

inline ArchiveBlock::~ArchiveBlock()
{
    if (archive_)
        archive_->EndBlock();
}

inline bool Archive::ValidateName(std::string_view name)
{
    // Empty names are not allowed
    if (name.empty())
        return false;

    // Name must start with letter or underscore.
    if (!isalpha(name[0]) && name[0] != '_')
        return false;

    // Name must contain only letters, digits, underscores, dots or colons.
    for (int i = 0; i < name.length(); i++)
    {
        auto ch = name[i];
        if (!isalnum(ch) && ch != '_' && ch != '.' && ch != ':')
            return false;
    }

    return true;
}


/// Archive implementation helper. Provides default Archive implementation for most cases.
//class GFROST_API ArchiveBase : public Archive, private NonCopyable
//{
//public:
//    /// Get context.
//    Context* GetContext() override { return nullptr; }
//    /// Return name of the archive.
//    String GetName() const override { return {}; }
//    /// Return a checksum if applicable.
//    unsigned GetChecksum() override { return 0; }
//
//    /// Return whether the unordered element access is supported in currently open block.
//    /// Always false if current block is not Unordered. Always false for some archive types.
//    virtual bool IsUnorderedAccessSupportedInCurrentBlock() const = 0;
//    /// Return whether the element or block with given name is present.
//    /// Should be called only if both IsInput and IsUnorderedAccessSupportedInCurrentBlock are true.
//    virtual bool HasElementOrBlock(const char* name) const = 0;
//    /// Whether the any following archive operation will result in failure.
//    bool IsEOF() const final { return eof_; }
//    /// Whether the serialization error occurred.
//    bool HasError() const final { return hasError_; }
//    /// Return first error string.
//    String GetErrorString() const final { return errorString_; }
//    /// Return first error stack.
//    String GetErrorStack() const final { return errorStack_; }
//
//    /// Serialize version number. 0 is invalid version.
//    unsigned SerializeVersion(unsigned version) final
//    {
//        if (!SerializeVLE(versionElementName_, version))
//            return 0;
//        return version;
//    }
//
//    /// Set archive error.
//    void SetError(String error) override
//    {
//        SetErrorFormatted(error);
//    }
//
//    /// Set archive error (formatted string).
//    template <class ... Args>
//    void SetErrorFormatted(String errorString, const Args& ... args)
//    {
//        if (!hasError_)
//        {
//            hasError_ = true;
//            errorStack_ = GetCurrentStackString();
//            errorString_ = ToString(errorString.CString(), args...);
//        }
//    }
//
//public:
//    /// Version element name.
//    static const char* versionElementName_;
//    /// Artificial element name used for error reporting related to Map keys.
//    static const char* keyElementName_;
//    /// Artificial element name used for error reporting related to block itself.
//    static const char* blockElementName_;
//
//    /// Fatal error message: root block was not opened. Placeholders: {elementName}.
//    static const String fatalRootBlockNotOpened_elementName;
//    /// Fatal error message: unexpected EndBlock call.
//    static const String fatalUnexpectedEndBlock;
//    /// Fatal error message: missing element name in Unordered block.
//    static const String fatalMissingElementName;
//    /// Fatal error message: missing key serialization.
//    static const String fatalMissingKeySerialization;
//    /// Fatal error message: duplicated key serialization.
//    static const String fatalDuplicateKeySerialization;
//    /// Fatal error message: unexpected key serialization.
//    static const String fatalUnexpectedKeySerialization;
//    /// Fatal error message: invalid element or block name.
//    static const String fatalInvalidName;
//    /// Error message: input archive has no more data. Placeholders: {elementName}.
//    static const String errorEOF_elementName;
//    /// Error message: unspecified I/O failure. Placeholders: {elementName}.
//    static const String errorUnspecifiedFailure_elementName;
//
//    /// Input error message: element or block is not found. Placeholders: {elementName}.
//    static const String errorElementNotFound_elementName;
//    /// Input error message: unexpected block type. Placeholders: {blockName}.
//    static const String errorUnexpectedBlockType_blockName;
//    /// Input error message: missing map key.
//    static const String errorMissingMapKey;
//
//    /// Output error message: duplicate element. Placeholders: {elementName}.
//    static const String errorDuplicateElement_elementName;
//    /// Output fatal error message: Archive or Map block overflow while writing archive.
//    static const String fatalBlockOverflow;
//    /// Output fatal error message: Archive or Map block underflow while writing archive.
//    static const String fatalBlockUnderflow;
//
//protected:
//    /// Set EOF flag.
//    void CloseArchive() { eof_ = true; }
//
//private:
//    /// End-of-file flag.
//    bool eof_{};
//    /// Error flag.
//    bool hasError_{};
//    /// Error string.
//    String errorString_{};
//    /// Error stack.
//    String errorStack_{};
//};

///// Archive implementation helper (template). Provides all archive traits
//template <bool IsInputBool, bool IsHumanReadableBool>
//class ArchiveBaseT : public ArchiveBase
//{
//public:
//    /// Whether the archive is in input mode.
//    bool IsInput() const final { return IsInputBool; }
//    /// Whether the human-readability is preferred over performance and output size.
//    bool IsHumanReadable() const final { return IsHumanReadableBool; }
//};

}