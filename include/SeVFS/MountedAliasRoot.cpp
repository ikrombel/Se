// Copyright (c) 2023-2023 the rbfx project.
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT> or the accompanying LICENSE file.

#include "MountedAliasRoot.h"

#include <algorithm>
#include <vector>
#include <unordered_map>


//#include "Urho3D/IO/Log.h"

namespace Se
{

namespace
{

static const  String aliasSeparator = ":/";

String StripFileName(const String& fileName, const String& alias)
{
    const unsigned offset = alias.size() + aliasSeparator.size();
    return offset < fileName.size() ? fileName.substr(offset) : String{};
}

FileIdentifier StripFileIdentifier(const FileIdentifier& fileName, const String& alias, const String& scheme)
{
    return {scheme, StripFileName(fileName.fileName_, alias)};
}

} // namespace

MountedAliasRoot::MountedAliasRoot()
    : MountPoint()
{
}

MountedAliasRoot::~MountedAliasRoot() = default;

void MountedAliasRoot::AddAlias(const String& path, const String& scheme, std::shared_ptr<MountPoint> mountPoint)
{
    aliases_[path] = {mountPoint, scheme};
}

void MountedAliasRoot::RemoveAliases(MountPointPtr mountPoint)
{

    using AliasesType = std::unordered_map<Se::String, 
             std::pair<std::weak_ptr<Se::MountPoint>, String>>;

    // std::vector<typename std::unordered_map<Se::String, 
    //         std::pair<std::weak_ptr<Se::MountPoint>, String>>::iterator> stack;

    // for (auto it = aliases_.begin(); it != aliases_.end(); ++it) {

    //     auto mount = it->second.first;
    //     if (mount == mountPoint)
    //         stack.push_back(it);
    // }

    // aliases_.erase()

    //std::erase_if
    std::erase_if(aliases_, [mountPoint](const auto& pair) { 
        return pair.second.first == mountPoint; });
    // std::remove_if(aliases_.begin(), aliases_.end(), [mountPoint](const AliasesType::mapped_type& pair) {
    //      return pair.second.first.get() == mountPoint.get(); });
    //assert(0 && "TODO");
}

std::tuple<MountPointPtr, String, String> MountedAliasRoot::FindMountPoint(String fileName) const
{
    const auto separatorPos = fileName.find(aliasSeparator);
    if (separatorPos == std::string::npos)
        return {};

    const String alias = fileName.substr(0, separatorPos);
    const auto iter = aliases_.find(alias);
    if (iter == aliases_.end() || !iter->second.first)
        return {};

    return {iter->second.first, iter->first, iter->second.second};
}

bool MountedAliasRoot::AcceptsScheme(const String& scheme) const
{
    return scheme.comparei("alias") == 0;
}

bool MountedAliasRoot::Exists(const FileIdentifier& fileName) const
{
    if (!AcceptsScheme(fileName.scheme_))
        return false;

    const auto [mountPoint, alias, scheme] = FindMountPoint(fileName.fileName_);
    if (!mountPoint)
        return false;

    const FileIdentifier resolvedFileName = StripFileIdentifier(fileName, alias, scheme);
    return mountPoint->Exists(resolvedFileName);
}

AbstractFilePtr MountedAliasRoot::OpenFile(const FileIdentifier& fileName, FileMode mode)
{
    if (!AcceptsScheme(fileName.scheme_))
        return nullptr;

    const auto [mountPoint, alias, scheme] = FindMountPoint(fileName.fileName_);
    if (!mountPoint)
        return nullptr;

    const FileIdentifier resolvedFileName = StripFileIdentifier(fileName, alias, scheme);
    return mountPoint->OpenFile(resolvedFileName, mode);
}

std::optional<FileTime> MountedAliasRoot::GetLastModifiedTime(
    const FileIdentifier& fileName, bool creationIsModification) const
{
    if (!AcceptsScheme(fileName.scheme_))
        return std::nullopt;

    const auto [mountPoint, alias, scheme] = FindMountPoint(fileName.fileName_);
    if (!mountPoint)
        return std::nullopt;

    const FileIdentifier resolvedFileName = StripFileIdentifier(fileName, alias, scheme);
    return mountPoint->GetLastModifiedTime(resolvedFileName, creationIsModification);
}

String MountedAliasRoot::GetName() const
{
    static const String name = "alias://";
    return name;
}

String MountedAliasRoot::GetAbsoluteNameFromIdentifier(const FileIdentifier& fileName) const
{
    if (!AcceptsScheme(fileName.scheme_))
        return String::EMPTY;

    const auto [mountPoint, alias, scheme] = FindMountPoint(fileName.fileName_);
    if (!mountPoint)
        return String::EMPTY;

    const FileIdentifier resolvedFileName = StripFileIdentifier(fileName, alias, scheme);
    return mountPoint->GetAbsoluteNameFromIdentifier(resolvedFileName);
}

FileIdentifier MountedAliasRoot::GetIdentifierFromAbsoluteName(const String& absoluteFileName) const
{
    // This operation is not supported, actual mount points should do this work.
    return FileIdentifier{};
}

void MountedAliasRoot::Scan(
    std::vector<String>& result, const String& pathName, const String& filter, ScanFlags flags) const
{
    // Scanning is not supported for aliases.
}

} // namespace Se
