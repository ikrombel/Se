#pragma once

#include <Se/String.hpp>

namespace Se
{

/// File identifier, similar to Uniform Resource Identifier (URI).
/// Known differences:
/// - If URI starts with `/` or `x:/` it is treated as `file` scheme automatically.
/// - Host names are not supported for `file:` scheme.
///   All of `file:/path/to/file`, `file://path/to/file`, and `file:///path/to/file` are supported
///   and denote absolute file path.
/// - If URI does not contain `:`, it is treated as special "empty" scheme,
///   and the entire URI is treated as relative path.
/// - Conversion to URI string uses `scheme://` format.
struct FileIdentifier
{
    /// File identifier that references nothing.
    static const FileIdentifier Empty;

    /// Construct default.
    FileIdentifier() = default;
    /// Construct from scheme and path (as is).
    FileIdentifier(const String& scheme, const String& fileName);
    /// Deprecated. Use FromUri() instead.
    FileIdentifier(const  String& uri) : FileIdentifier(FromUri(uri)) {}

    /// Construct from uri-like path.
    static FileIdentifier FromUri(const String& uri);
    /// Return URI-like path. Does not always return original path.
     String ToUri() const;

    /// Append path to the current path, adding slash in between if it's missing.
    /// Ignores current scheme restrictions.
    void AppendPath(const String& path);

    /// URI-like scheme. May be empty if not specified.
     String scheme_;
    /// URI-like path to the file.
    String fileName_;

    /// Return whether the identifier is empty.
    bool IsEmpty() const { return scheme_.empty() && fileName_.empty(); }

    /// Operators.
    /// @{
    explicit operator bool() const { return !IsEmpty(); }
    bool operator!() const { return IsEmpty(); }

    bool operator<(const FileIdentifier& rhs) const noexcept
    {
        return (scheme_ < rhs.scheme_) || (scheme_ == rhs.scheme_ && fileName_ < rhs.fileName_);
    }

    bool operator==(const FileIdentifier& rhs) const noexcept
    {
        return scheme_ == rhs.scheme_ && fileName_ == rhs.fileName_;
    }

    bool operator!=(const FileIdentifier& rhs) const noexcept { return !(rhs == *this); }

    FileIdentifier& operator+=(const String& rhs)
    {
        AppendPath(rhs);
        return *this;
    }

    FileIdentifier operator+(const String& rhs) const
    {
        FileIdentifier tmp = *this;
        tmp += rhs;
        return tmp;
    }
    /// @}

    static  String SanitizeFileName(const String& fileName);
};

} // namespace Se
