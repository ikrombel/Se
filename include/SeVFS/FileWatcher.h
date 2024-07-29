#pragma once

#include <Se/Thread.h>
#include <Se/Timer.h>

#include <unordered_map>

namespace Se
{

class FileSystem;

enum FileChangeKind
{
    /// New file was created.
    FILECHANGE_ADDED,
    /// File was deleted.
    FILECHANGE_REMOVED,
    /// File was renamed.
    FILECHANGE_RENAMED,
    /// File was modified.
    FILECHANGE_MODIFIED,
};

/// File change information.
struct FileChange
{
    /// File change kind.
    FileChangeKind kind_;
    /// Name of modified file name. Always set.
    String fileName_;
    /// Previous file name in case of FILECHANGE_MODIFIED event. Empty otherwise.
    String oldFileName_;
};

/// Watches a directory and its subdirectories for files being modified.
class FileWatcher : public Thread
{

public:
    /// Construct.
    explicit FileWatcher();
    /// Destruct.
    ~FileWatcher() override;

    /// Directory watching loop.
    void ThreadFunction() override;

    /// Start watching a directory. Return true if successful.
    bool StartWatching(const String& pathName, bool watchSubDirs);
    /// Stop watching the directory.
    void StopWatching();
    /// Set the delay in seconds before file changes are notified. This (hopefully) avoids notifying when a file save is still in progress. Default 1 second.
    void SetDelay(float interval);
    /// Add a file change into the changes queue.
    void AddChange(const FileChange& change);
    /// Return a file change (true if was found, false if not.)
    bool GetNextChange(FileChange& dest);

    /// Return the path being watched, or empty if not watching.
    const String& GetPath() const { return path_; }

    /// Return the delay in seconds for notifying file changes.
    float GetDelay() const { return delay_; }

private:
    struct TimedFileChange
    {
        /// File change information.
        FileChange change_;
        /// Timer used to filter out repeated events when file is being written.
        Timer timer_;
    };

    /// Filesystem.
    FileSystem* fileSystem_;
    /// The path being watched.
    String path_;
    /// Pending changes. These will be returned and removed from the list when their timer has exceeded the delay.
    std::unordered_map<String, TimedFileChange> changes_;
    /// Mutex for the change buffer.
    Mutex changesMutex_;
    /// Delay in seconds for notifying changes.
    float delay_;
    /// Watch subdirectories flag.
    bool watchSubDirs_;

#ifdef _WIN32

    /// Directory handle for the path being watched.
    void* dirHandle_;

#elif __linux__

    /// HashMap for the directory and sub-directories (needed for inotify's int handles).
    std::unordered_map<int, String> dirHandle_;
    /// Linux inotify needs a handle.
    int watchHandle_;

#elif defined(__APPLE__) && !defined(IOS) && !defined(TVOS)

    /// Flag indicating whether the running OS supports individual file watching.
    bool supported_;
    /// Pointer to internal MacFileWatcher delegate.
    void* watcher_;

#endif
};

}
