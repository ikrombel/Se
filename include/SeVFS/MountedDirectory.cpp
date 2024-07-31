// Copyright (c) 2022-2022 the Urho3D project.

#include "MountedDirectory.h"

// #include "Se/Core/Context.h"
// #include "Se/Core/CoreEvents.h"
#include <Se/IO/File.h>
#include <Se/IO/FileSystem.h>
#include <Se/Console.hpp>

#include <Se/Application.hpp>
//#include "Se/Resource/ResourceEvents.h"

namespace Se
{

MountedDirectory::MountedDirectory(const String& directory, String scheme)
    : WatchableMountPoint()
    , scheme_(std::move(scheme))
    , directory_(SanitizeDirName(directory))
    , name_(scheme_.empty() ? directory_ : (scheme_ + "://" + directory_))
{
}

MountedDirectory::~MountedDirectory()
{
}

String MountedDirectory::SanitizeDirName(const String& name) const
{
    String fixedPath = AddTrailingSlash(name.c_str());
    if (!IsAbsolutePath(fixedPath))
        fixedPath = FileSystem::Get().GetCurrentDir() + fixedPath;

    // Sanitize away /./ construct
    fixedPath.replace("/./", "/");

    fixedPath = fixedPath.trimmed();
    return fixedPath;
}



void MountedDirectory::StartWatching()
{
    if (!fileWatcher_)
        fileWatcher_ = std::make_shared<FileWatcher>();

    fileWatcher_->StartWatching(directory_, true);

    // Subscribe BeginFrame for handling directory watcher
//    SubscribeToEvent(E_BEGINFRAME, &MountedDirectory::ProcessUpdates);
    idOnStopWatching = Application::onBeginFrame.connect([this](){
        this->ProcessUpdates();
    });
    //assert(0);
}

void MountedDirectory::StopWatching()
{
    if (fileWatcher_)
        fileWatcher_->StopWatching();

    Application::onBeginFrame.disconnect(idOnStopWatching);
        

//    UnsubscribeFromEvent(E_BEGINFRAME);
//    assert(0);
}

void MountedDirectory::ProcessUpdates()
{
    if (!fileWatcher_)
        return;

    FileChange change;
    while (fileWatcher_->GetNextChange(change))
    {
        FileChangeInfo tmp;

        tmp.fileName_ = fileWatcher_->GetPath() + change.fileName_;
        tmp.resourceName_ = FileIdentifier{scheme_, change.fileName_}.ToUri();
        tmp.kind_ = change.kind_;
        onFileChanged(tmp);
    }
}

bool MountedDirectory::AcceptsScheme(const String& scheme) const
{
    return scheme.comparei(scheme_) == 0;
}

bool MountedDirectory::Exists(const FileIdentifier& fileName) const
{
    // File system directory only reacts on specific scheme.
    if (!AcceptsScheme(fileName.scheme_))
        return false;

    auto fileSystem = FileSystem::Get();

    return fileSystem.FileExists(directory_ + fileName.fileName_);
}

AbstractFilePtr MountedDirectory::OpenFile(const FileIdentifier& fileName, FileMode mode)
{
    // File system directory only reacts on specific scheme.
    if (!AcceptsScheme(fileName.scheme_))
        return nullptr;

    auto fileSystem = FileSystem::Get();

    const bool needRead = mode == FILE_READ || mode == FILE_READWRITE;
    const bool needWrite = mode == FILE_WRITE || mode == FILE_READWRITE;
    const String fullPath = directory_ + fileName.fileName_;

    if (needRead && !fileSystem.FileExists(fullPath))
        return nullptr;

    if (needWrite)
    {
        String directory = GetPath(fullPath);
        if (!fileSystem.DirExists(directory))
        {
            if (!fileSystem.CreateDir(directory))
                return nullptr;
        }
    }

    auto file = std::make_shared<File>(fullPath, mode);
    if (!file->IsOpen())
        return nullptr;

    file->SetName(fileName.ToUri());
    return file;
}

std::optional<FileTime> MountedDirectory::GetLastModifiedTime(
    const FileIdentifier& fileName, bool creationIsModification) const
{
    if (!Exists(fileName))
        return std::nullopt;

    auto fileSystem = FileSystem::Get();
    const String fullPath = directory_ + fileName.fileName_;
    return fileSystem.GetLastModifiedTime(fullPath, creationIsModification);
}

String MountedDirectory::GetAbsoluteNameFromIdentifier(const FileIdentifier& fileName) const
{
    if (Exists(fileName))
        return directory_ + fileName.fileName_;

    return String::EMPTY;
}

FileIdentifier MountedDirectory::GetIdentifierFromAbsoluteName(const String& absoluteFileName) const
{
    if (absoluteFileName.starts_with(directory_))
        return FileIdentifier{scheme_, absoluteFileName.substr(directory_.length())};

    return FileIdentifier::Empty;
}

void MountedDirectory::Scan(
    std::vector<String>& result, const String& pathName, const String& filter, ScanFlags flags) const
{
    const auto fileSystem = FileSystem::Get();
    fileSystem.ScanDir(result, directory_ + pathName, filter, flags);
}

} // namespace Se
