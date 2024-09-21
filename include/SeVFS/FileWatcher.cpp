#include <Se/IO/File.h>
#include <Se/IO/FileSystem.h>
#include <Se/Console.hpp>

#include "FileWatcher.h"

#ifdef _WIN32
#  include <windows.h>
#  ifdef max
#    undef max
#  endif
#elif __linux__
#  include <sys/inotify.h>
#  include <sys/ioctl.h>
    extern "C" {
        // Need read/close for inotify
        #include <unistd.h>
    }
#elif defined(__APPLE__) && !defined(IOS) && !defined(TVOS)
    extern "C" {
    #include <Se/IO/MacFileWatcher.h>
    }
#endif

namespace Se
{
#ifndef __APPLE__
static const unsigned BUFFERSIZE = 4096;
#endif

Signal<const FileChangeInfo& /*FileInfo*/> FileWatcher::onFileChanged;

FileWatcher::FileWatcher() :
    Thread(),
    fileSystem_(&FileSystem::Get()),
    delay_(1.0f),
    watchSubDirs_(false)
{
#ifdef SE_FILEWATCHER
#ifdef __linux__
    watchHandle_ = inotify_init();
#elif defined(__APPLE__) && !defined(IOS) && !defined(TVOS)
    supported_ = IsFileWatcherSupported();
#endif
#endif
}

FileWatcher::~FileWatcher()
{
    StopWatching();
#ifdef SE_FILEWATCHER
#ifdef __linux__
    close(watchHandle_);
#endif
#endif
}

bool FileWatcher::StartWatching(const String& pathName, bool watchSubDirs, bool restart)
{
    if (restart) {
        String pathBackup = path_;
        // Stop any previous watching
        StopWatching(restart);
        path_ = pathBackup;
    }
    else
        StopWatching();

//    SetName("Watcher for " + pathName);

#if defined(SE_FILEWATCHER) && defined(SE_THREADING)
#ifdef _WIN32
    String nativePath = GetNativePath(RemoveTrailingSlash(pathName));

    dirHandle_ = (void*)CreateFileW(
        Utf8ToUcs2(nativePath.c_str()).c_str(),
        FILE_LIST_DIRECTORY,
        FILE_SHARE_WRITE | FILE_SHARE_READ | FILE_SHARE_DELETE,
        nullptr,
        OPEN_EXISTING,
        FILE_FLAG_BACKUP_SEMANTICS,
        nullptr);

    if (dirHandle_ != INVALID_HANDLE_VALUE)
    {
        path_ = AddTrailingSlash(pathName);
        watchSubDirs_ = watchSubDirs;
        Run();

        SE_LOG_DEBUG("Started watching path " + pathName);
        return true;
    }
    else
    {
        SE_LOG_ERROR("Failed to start watching path " + pathName);
        return false;
    }
#elif defined(__linux__)
    uint32_t flags = IN_CREATE | IN_DELETE | IN_MODIFY | IN_MOVED_FROM | IN_MOVED_TO;
    int handle = inotify_add_watch(watchHandle_, pathName.c_str(), flags);

    if (handle < 0)
    {
        SE_LOG_ERROR("Failed to start watching path " + pathName);
        return false;
    }
    else
    {
        // Store the root path here when reconstructed with inotify later
        dirHandle_[handle] = String::EMPTY;
        path_ = AddTrailingSlash(pathName);
        watchSubDirs_ = watchSubDirs;

        if (watchSubDirs_)
        {
            std::vector<String> subDirs;
            FileSystem::Get().ScanDir(subDirs, pathName, "*", SCAN_DIRS | SCAN_RECURSIVE);

            for (unsigned i = 0; i < subDirs.size(); ++i)
            {
                String subDirFullPath = AddTrailingSlash(path_ + subDirs[i]);

                // Don't watch ./ or ../ sub-directories
                if (!subDirFullPath.ends_with("./"))
                {
                    handle = inotify_add_watch(watchHandle_, subDirFullPath.c_str(), flags);
                    if (handle < 0)
                        SE_LOG_ERROR("Failed to start watching subdirectory path " + subDirFullPath);
                    else
                    {
                        // Store sub-directory to reconstruct later from inotify
                        dirHandle_[handle] = AddTrailingSlash(subDirs[i]);
                    }
                }
            }
        }
        if (!restart) {
            Run();
            SE_LOG_DEBUG("Started watching path " + pathName);
        }
        return true;
    }
#elif defined(__APPLE__) && !defined(IOS) && !defined(TVOS)
    if (!supported_)
    {
        SE_LOG_ERROR("Individual file watching not supported by this OS version, can not start watching path " + pathName);
        return false;
    }

    watcher_ = CreateFileWatcher(pathName.CString(), watchSubDirs);
    if (watcher_)
    {
        path_ = AddTrailingSlash(pathName);
        watchSubDirs_ = watchSubDirs;
        if (!restart) {
            Run();
            SE_LOG_DEBUG("Started watching path " + pathName);
        }
        return true;
    }
    else
    {
        SE_LOG_ERROR("Failed to start watching path " + pathName);
        return false;
    }
#else
    SE_LOG_ERROR("FileWatcher not implemented, can not start watching path " + pathName);
    return false;
#endif
#else
    SE_LOG_DEBUG("FileWatcher feature not enabled");
    return false;
#endif
}

void FileWatcher::StopWatching(bool restart)
{
    if (handle_)
    {
        if (!restart)
            shouldRun_ = false;

        // Create and delete a dummy file to make sure the watcher loop terminates
        // This is only required on Windows platform
        // TODO: Remove this temp write approach as it depends on user write privilege
#ifdef _WIN32
        String dummyFileName = path_ + "dummy.tmp";
        File file(dummyFileName, FILE_WRITE);
        file.Close();
        FileSystem::Get().Delete(dummyFileName);
#endif

#if defined(__APPLE__) && !defined(IOS) && !defined(TVOS)
        // Our implementation of file watcher requires the thread to be stopped first before closing the watcher
        if (!restart)
            Stop();
#endif

#ifdef _WIN32
        CloseHandle((HANDLE)dirHandle_);
#elif defined(__linux__)
        for (auto i = dirHandle_.begin(); i != dirHandle_.end(); ++i)
            inotify_rm_watch(watchHandle_, i->first);
        dirHandle_.clear();
#elif defined(__APPLE__) && !defined(IOS) && !defined(TVOS)
        CloseFileWatcher(watcher_);
#endif

        if (!restart) {
#ifndef __APPLE__
            Stop();
#endif
            SE_LOG_DEBUG("Stopped watching path " + path_);
        }
        path_.clear();
    }
}

void FileWatcher::SetDelay(float interval)
{
    delay_ = std::max(interval, 0.0f);
}

void FileWatcher::ThreadFunction()
{
#ifdef SE_FILEWATCHER
#  ifdef _WIN32
    unsigned char buffer[BUFFERSIZE];
    DWORD bytesFilled = 0;

    while (shouldRun_)
    {
        if (ReadDirectoryChangesW((HANDLE)dirHandle_,
            buffer,
            BUFFERSIZE,
            watchSubDirs_,
            FILE_NOTIFY_CHANGE_FILE_NAME |
            FILE_NOTIFY_CHANGE_LAST_WRITE,
            &bytesFilled,
            nullptr,
            nullptr))
        {
            unsigned offset = 0;
            FileChange rename{FILECHANGE_RENAMED, String::EMPTY, String::EMPTY};

            while (offset < bytesFilled)
            {
                FILE_NOTIFY_INFORMATION* record = (FILE_NOTIFY_INFORMATION*)&buffer[offset];

                String fileName;
                const wchar_t* src = record->FileName;
                const wchar_t* end = src + record->FileNameLength / 2;
                while (src < end)
                     fileName += Ucs2ToUtf8(src);

                fileName = GetInternalPath(fileName);

                if (record->Action == FILE_ACTION_MODIFIED)
                    AddChange({ FILECHANGE_MODIFIED, fileName, String::EMPTY });
                else if (record->Action == FILE_ACTION_ADDED)
                    AddChange({ FILECHANGE_ADDED, fileName, String::EMPTY });
                else if (record->Action == FILE_ACTION_REMOVED)
                    AddChange({ FILECHANGE_MOVED, fileName, String::EMPTY });
                else if (record->Action == FILE_ACTION_RENAMED_OLD_NAME)
                    rename.oldFileName_ = fileName;
                else if (record->Action == FILE_ACTION_RENAMED_NEW_NAME)
                    rename.fileName_ = fileName;

                if (!rename.oldFileName_.empty() && !rename.fileName_.empty())
                {
                    AddChange(rename);
                    rename = {};
                }

                if (!record->NextEntryOffset)
                    break;
                else
                    offset += record->NextEntryOffset;
            }
        }
    }
#  elif defined(__linux__)
    unsigned char buffer[BUFFERSIZE];

    while (shouldRun_)
    {
        unsigned available = 0;
        ioctl(watchHandle_, FIONREAD, &available);

        if (available == 0)
        {
            Time::Sleep(100);
            continue;
        }

        int i = 0;
        auto length = (int)read(watchHandle_, buffer, sizeof(buffer));

        if (length < 0)
            return;

        std::unordered_map<unsigned, FileChange> renames;
        while (i < length)
        {
            auto* event = (inotify_event*)&buffer[i];

            if (event->len > 0)
            {
                if (   event->mask & IN_MODIFY 
                    || event->mask & IN_MOVE
                    || event->mask & IN_DELETE
                    || event->mask & IN_CREATE)
                {
                    String fileName;
                    fileName = dirHandle_[event->wd] + event->name;
                    if ((event->mask & IN_CREATE) == IN_CREATE) {
                        AddChange({FILECHANGE_ADDED, fileName, String::EMPTY});
                        if ((event->mask & IN_ISDIR) == IN_ISDIR) {
                            StartWatching(path_, watchSubDirs_, true);
                        }
                    }
                    else if ((event->mask & IN_MOVED_TO) == IN_MOVED_TO ||
                            (event->mask & IN_MOVED_FROM) == IN_MOVED_FROM)
                        AddChange({FILECHANGE_MOVED, fileName, String::EMPTY});
                    else if ((event->mask & IN_MODIFY) == IN_MODIFY || (event->mask & IN_ATTRIB) == IN_ATTRIB)
                        AddChange({FILECHANGE_MODIFIED, fileName, String::EMPTY});
                    else if (event->mask & IN_MOVE)
                    {
                        auto& entry = renames[event->cookie];
                        if ((event->mask & IN_MOVED_FROM) == IN_MOVED_FROM)
                            entry.oldFileName_ = fileName;
                        else if ((event->mask & IN_MOVED_TO) == IN_MOVED_TO)
                            entry.fileName_ = fileName;

                        if (!entry.oldFileName_.empty() && !entry.fileName_.empty())
                        {
                            entry.kind_ = FILECHANGE_RENAMED;
                            AddChange(entry);
                        }
                    }
                }
            }

            i += sizeof(inotify_event) + event->len;
        }
    }
#elif defined(__APPLE__) && !defined(IOS) && !defined(TVOS)
    while (shouldRun_)
    {
        Time::Sleep(100);

        String changes = ReadFileWatcher(watcher_);
        if (!changes.empty())
        {
            std::vector<String> fileChanges = changes.Split('\n');
            FileChange change{};
            for (const String& fileResult : fileChanges)
            {
                change.kind_ = (FileChangeKind)fileResult[0];   // First byte is change kind.
                String fileName = &fileResult.At(1);
                if (change.kind_ == FILECHANGE_RENAMED)
                {
                    if (FileSystem::Get().FileExists(fileName))
                        change.fileName_ = std::move(fileName);
                    else
                        change.oldFileName_ = std::move(fileName);

                    if (!change.fileName_.empty() && !change.oldFileName_.empty())
                    {
                        AddChange(change);
                        change = {};
                    }
                }
                else
                {
                    change.fileName_ = std::move(fileName);
                    AddChange(change);
                    change = {};
                }
            }
        }
    }
#  endif
#endif
}

void FileWatcher::AddChange(const FileChange& change)
{
    //MutexLock lock(changesMutex_);
    MutexLock guard(changesMutex_);

    auto it = changes_.find(change.fileName_);
    if (it == changes_.end())
        changes_[change.fileName_].change_ = change;
    else
        // Reset the timer associated with the filename. Will be notified once timer exceeds the delay
        it->second.timer_.Reset();
}

bool FileWatcher::GetNextChange(FileChange& dest)
{
    //MutexLock lock(changesMutex_);
    MutexLock guard(changesMutex_);

    auto delayMsec = (unsigned)(delay_ * 1000.0f);

    if (changes_.empty())
        return false;

    for (auto i = changes_.begin(); i != changes_.end(); ++i)
    {
        if (i->second.timer_.GetMSec(false) >= delayMsec)
        {
            dest = i->second.change_;
            changes_.erase(i);
            return true;
        }
    }

    return false;
}

}
