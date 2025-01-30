#pragma once

#include <Se/String.hpp>
#include <Se/StringHash.hpp>
#include <Se/FlagSet.hpp>
#include <Se/NonCopyable.hpp>
#include <Se/IO/ScanFlags.hpp>

#include <unordered_set>
#include <list>

namespace Se
{

class AsyncExecRequest;

enum FSItemType
{
    FSIT_FILE,
    FSIT_DIR,
    FSIT_LINK,
    FSIT_FILECUSTOM, 
    FSIT_UNKNOWN
};

enum FSItemFlag
{
    FSIF_NONE = 0,
    FSIF_READONLY = 0x1,
    FSIF_DIRECTORY = 0x2,
    FSIF_EXECUTABLE = 0x4,
    FSIF_HIDDEN = 0x8,
    FSIF_ARCHIVE = 0x10,
    FSIF_COMPRESSED = 0x20,
    FSIF_ENCRYPTED = 0x40,
    FSIF_SYSTEM = 0x80,
    FSIF_TEMPORARY = 0x100,
    FSIF_CHANGED = 0x200,
    FSIF_OPENED = 0x400,
    FSIF_HAS_ERROR = 0x800,
    FSIF_HAS_WARNING = 0x1000,
};
SE_FLAGSET(FSItemFlag, FSItemFlags);

struct DirectoryNode
{
	Se::String FullPath;
	Se::String FileName;
	std::vector<DirectoryNode> Children;
    DirectoryNode* parent{nullptr};
    FSItemFlags Flags{FSItemFlag::FSIF_NONE};
};

/// Subsystem for file and directory operations and access control.
class FileSystem
{
public:
    /// Construct.
    explicit FileSystem();
    /// Destruct.
    virtual ~FileSystem();

    /// Set the current working directory.
    /// @property
    bool SetCurrentDir(const String& pathName);
    /// Create a directory.
    bool CreateDir(const String& pathName);
    /// Set whether to execute engine console commands as OS-specific system command.
    /// @property
    void SetExecuteConsoleCommands(bool enable);
    /// Run a program using the command interpreter, block until it exits and return the exit code. Will fail if any allowed paths are defined.
    int SystemCommand(const String& commandLine, bool redirectStdOutToLog = false);
    /// Run a specific program, block until it exits and return the exit code. Will fail if any allowed paths are defined. Returns STDOUT output of subprocess.
    int SystemRun(const String& fileName, const std::vector<String>& arguments, String& output);
    /// Run a specific program, block until it exits and return the exit code. Will fail if any allowed paths are defined.
    int SystemRun(const String& fileName, const std::vector<String>& arguments);
    /// Run a specific program, do not block until it exits. Will fail if any allowed paths are defined.
    int SystemSpawn(const String& fileName, const std::vector<String>& arguments);
    /// Run a program using the command interpreter asynchronously. Return a request ID or M_MAX_UNSIGNED if failed. The exit code will be posted together with the request ID in an AsyncExecFinished event. Will fail if any allowed paths are defined.
    unsigned SystemCommandAsync(const String& commandLine);
    /// Run a specific program asynchronously. Return a request ID or M_MAX_UNSIGNED if failed. The exit code will be posted together with the request ID in an AsyncExecFinished event. Will fail if any allowed paths are defined.
    unsigned SystemRunAsync(const String& fileName, const std::vector<String>& arguments);
    /// Open a file in an external program, with mode such as "edit" optionally specified. Will fail if any allowed paths are defined.
    bool SystemOpen(const String& fileName, const String& mode = String::EMPTY);
    /// Copy a file. Return true if successful.
    bool Copy(const String& srcFileName, const String& destFileName);
    /// Rename a file. Return true if successful.
    bool Rename(const String& srcFileName, const String& destFileName);
    /// Delete a file. Return true if successful.
    bool Delete(const String& fileName);
    /// Register a path as allowed to access. If no paths are registered, all are allowed. Registering allowed paths is considered securing the Urho3D execution environment: running programs and opening files externally through the system will fail afterward.
    void RegisterPath(const String& pathName);
    /// Set a file's last modified time as seconds since 1.1.1970. Return true on success.
    bool SetLastModifiedTime(const String& fileName, FileTime newTime);
    /// Reveal path or file in OS file browser.
    bool Reveal(const String& path);

    /// Return the absolute current working directory.
    /// @property
    String GetCurrentDir() const;

    /// Return whether is executing engine console commands as OS-specific system command.
    /// @property
    bool GetExecuteConsoleCommands() const { return executeConsoleCommands_; }

    /// Return whether paths have been registered.
    bool HasRegisteredPaths() const { return allowedPaths_.size() > 0; }

    /// Check if a path is allowed to be accessed. If no paths are registered, all are allowed.
    bool CheckAccess(const String& pathName) const;
    /// Returns the file's last modified time as seconds since 1.1.1970, or 0 if can not be accessed.
    FileTime GetLastModifiedTime(const String& fileName, bool creationIsModification = false) const;
    /// Check if a file exists.
    bool FileExists(const String& fileName) const;
    /// Check if a directory exists.
    bool DirExists(const String& pathName) const;
    /// Scan a directory for specified files.
    void ScanDir(std::vector<String>& result, const String& pathName, const String& filter, ScanFlags flags) const;
    /// Scan a directory for specified files.
    void ScanDirTree(DirectoryNode& result, const String& pathName, const String& filter, ScanFlags flags) const;
    /// Return the program's directory.
    /// @property
    String GetProgramDir() const;
    /// Return the program's executable file path, or empty string if not applicable.
    String GetProgramFileName() const;
    /// Return the user documents directory.
    /// @property
    String GetUserDocumentsDir() const;
    ///
    String GetENV(const String& envName) const;
    /// Return the application preferences directory.
    String GetAppPreferencesDir(const String& org, const String& app) const;
    /// Check if a file or directory exists at the specified path
    bool Exists(const String& pathName) const { return FileExists(pathName) || DirExists(pathName); }
    /// Copy files from one directory to another.
    bool CopyDir(const String& directoryIn, const String& directoryOut, std::vector<String>* copiedFiles = nullptr);
    /// Create subdirectories. New subdirectories will be made only in a subpath specified by `subdirectory`.
    bool CreateDirs(const String& root, const String& subdirectory);
    /// Create specified subdirectory and any parent directory if it does not exist.
    bool CreateDirsRecursive(const String& directoryIn);
    /// Remove files in a directory, or remove entire directory recursively.
    bool RemoveDir(const String& directoryIn, bool recursive);
    /// Return path of temporary directory. Path always ends with a forward slash.
    /// @property
    String GetTemporaryDir() const;
    /// Try to find resource prefix path starting from executable and going up.
    String FindResourcePrefixPath() const;

    static FileSystem& Get();

private:
    /// Scan directory, called internally.
    void ScanDirInternal(std::vector<String>& result, const String& path, const String& startPath,
        const String& filter, ScanFlags flags) const;

    void ScanDirInternalTree(DirectoryNode& result, const String& path, const String& startPath,
        const String& filter, ScanFlags flags) const;
    // /// Handle begin frame event to check for completed async executions.
    // void HandleBeginFrame(StringHash eventType, VariantMap& eventData);
    // /// Handle a console command event.
    // void HandleConsoleCommand(StringHash eventType, VariantMap& eventData);

    /// Allowed directories.
    std::unordered_set<String> allowedPaths_;
    /// Async execution queue.
    std::list<AsyncExecRequest*> asyncExecQueue_;
    /// Next async execution ID.
    unsigned nextAsyncExecID_{1};
    /// Flag for executing engine console commands as OS-specific system command. Default to true.
    bool executeConsoleCommands_{};
};

/// Helper class to create and destory temporary directory.
class TemporaryDir : public MovableNonCopyable
{
public:
    TemporaryDir(FileSystem* fs, const String& path);
    ~TemporaryDir();

    TemporaryDir(TemporaryDir&& rhs);
    TemporaryDir& operator = (TemporaryDir&& rhs);

    /// Return the path.
    String GetPath() const { return path_; }

private:
    FileSystem* fs_{};
    String path_;
};

/// Split a full path to path, filename and extension. The extension will be converted to lowercase by default.
void
    SplitPath(const String& fullPath, String& pathName, String& fileName, String& extension, bool lowercaseExtension = true);
/// Return the path from a full path.
String GetPath(const String& fullPath);
/// Return the filename from a full path.
String GetFileName(const String& fullPath);
/// Return the extension from a full path, converted to lowercase by default.
String GetExtension(const String& fullPath, bool lowercaseExtension = true);
/// Return the filename and extension from a full path. The case of the extension is preserved by default, so that the file can be opened in case-sensitive operating systems.
String GetFileNameAndExtension(const String& fileName, bool lowercaseExtension = false);
/// Replace the extension of a file name with another.
String ReplaceExtension(const String& fullPath, const String& newExtension);
/// Add a slash at the end of the path if missing and convert to internal format (use slashes).
String AddTrailingSlash(const String& pathName);
/// Remove the slash from the end of a path if exists and convert to internal format (use slashes).
String RemoveTrailingSlash(const String& pathName);
/// Return the parent path, or the path itself if not available.
String GetParentPath(const String& path);
/// Convert a path to internal format (use slashes).
String GetInternalPath(const String& pathName);
/// Convert a path to the format required by the operating system.
String GetNativePath(const String& pathName);
/// Convert a path to the format required by the operating system in wide characters.
std::wstring GetWideNativePath(const String& pathName);
/// Return whether a path is absolute.
bool IsAbsolutePath(const String& pathName);
///
bool IsAbsoluteParentPath(const String& absParentPath, const String& fullPath);
///
String GetSanitizedPath(const String& path);
// /// Convert file name to valid Windows filename.
// String GetSanitizedName(const String& name);
/// Given two absolute directory paths, get the relative path from one to the other
/// Returns false if either path isn't absolute, or if they are unrelated
bool GetRelativePath(const String& fromPath, const String& toPath, String& output);
/// Eliminate special dot and double dot path segments, replace directory separators with /.
/// One dot (.) always refers to the current folder.
/// Two dots(..) refers to the folder that is one level higher than the current folder.
/// The leading ../ portions of the path will be removed to prevent reaching for the files outside work directory.
String ResolvePath(const String& path);
/// Convert relative or absolute path to absolute path.
String GetAbsolutePath(const String& path, const String& currentPath, bool addTrailingSlash = false);
std::vector<String> GetAbsolutePaths(const std::vector<String>& paths, const String& currentPath, bool addTrailingSlash = false);
/// Convert extension from filter mask, or return empty string if no filter specified.
String GetExtensionFromFilter(const String& filter);
/// Check if a file name matches search mask.
bool MatchFileName(const String& fileName, const String& path, const String& extension,
    bool recursive = true, bool caseSensitive = true);
/// Trim prefix path following slash from the file name.
/// No check is performed if the prefix path is actually a prefix of the file name.
String TrimPathPrefix(const String& fileName, const String& prefixPath);

String FindProgramPath(const String& name);

/// Create a tree node from a path.
void TreeNodeAddPath(DirectoryNode* parent, const Se::String& path);
/// TODO not implement filter, flags
//void TreeNodeScanTree(DirectoryNode& result, const String& pathName, const String& filter, ScanFlags flags);
/// Sort tree nodes.
void SortTreeByName(Se::DirectoryNode& node);

}
