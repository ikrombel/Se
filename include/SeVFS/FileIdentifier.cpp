// Copyright (c) 2022-2022 the Urho3D project.


#include "FileIdentifier.h"

#include <Se/IO/FileSystem.h>

namespace Se
{

const FileIdentifier FileIdentifier::Empty{};

FileIdentifier::FileIdentifier(const String& scheme, const String& fileName)
    : scheme_(scheme)
    , fileName_(fileName)
{
}

FileIdentifier FileIdentifier::FromUri(const String& uri)
{
    // Special case: absolute path
    if (uri.front() == '/' || (uri.length() >= 3 && uri[1] == ':' && (uri[2] == '/' || uri[2] == '\\')))
        return {"file", SanitizeFileName(uri)};

    const auto schemePos = uri.find(":");

    // Special case: empty scheme
    if (schemePos == std::string::npos)
        return {String::EMPTY, SanitizeFileName(uri)};

    const auto scheme = uri.substr(0, schemePos);
    const auto path = uri.substr(schemePos + 1);

    const auto isSlash = [](char c) { return c == '/'; };
    const unsigned numSlashes = std::find_if_not(path.begin(), path.end(), isSlash) - path.begin();

    // Special case: file scheme
    if (scheme == "file")
    {
        if (numSlashes == 0 || numSlashes > 3)
            return FileIdentifier::Empty;

        // Keep one leading slash
        const auto localPath = path.substr(numSlashes - 1);

        // Windows-like path, e.g. /c:/path/to/file
        if (localPath.size() >= 3 && localPath[2] == ':')
            return {scheme, localPath.substr(1)};

        // Unix-like path, e.h. /path/to/file
        return {scheme, localPath};
    }

    // Trim up to two leading slashes for other schemes
    return {scheme, path.substr(std::min(numSlashes, 2u))};
}

 String FileIdentifier::ToUri() const
{
    // Special case: empty scheme
    if (scheme_.empty())
        return fileName_;

    // Special case: file scheme
    if (scheme_ == "file")
    {
        if (fileName_.empty())
            return  String{};
        else if (fileName_.front() == '/')
            return "file://" + fileName_;
        else
            return "file:///" + fileName_;
    }

    // Use scheme://path/to/file format by default
    return scheme_ + "://" + fileName_;
}

void FileIdentifier::AppendPath(const String& path)
{
    String tmpStr = path;

    if (tmpStr.empty())
        return;

    if (fileName_.empty())
    {
        fileName_ = tmpStr;
        return;
    }

    if (fileName_.back() != '/' && tmpStr.front() != '/')
        fileName_.push_back('/');
    if (fileName_.back() == '/' && tmpStr.front() == '/')
        tmpStr = tmpStr.substr(1);

    fileName_.append(tmpStr.data(), tmpStr.length());
}

 String FileIdentifier::SanitizeFileName(const String& fileName)
{
    // Resolve path and prevent references outside of data folder, except absolute path which is treated differently.
    return ResolvePath(fileName);
}

} // namespace Se
