#include "Se/Thread.h"
//#include "Se/Engine/EngineEvents.h"
#include "File.h"
#include "FileSystem.h"
//#include "Se/IO/IOEvents.h"
#include <Se/Console.hpp>
#include <Se/Finally.hpp>
#include <Se/StopToken.hpp>
#include <Se/String.hpp>

#include <SeMath/MathDefs.hpp>

#include <future>
#include <locale>
#include <codecvt>

#ifdef __ANDROID__
#include <SDL_rwops.h>
#endif

#ifdef HAVE_SDL
#include <SDL_filesystem.h>
#include <SDL_error.h>
#endif

#include <sys/stat.h>
#include <cstdio>
#include <algorithm>
//

#ifdef _WIN32
#include <windows.h>
#include <shellapi.h>
#include <direct.h>
#include <shlobj.h>
#include <sys/types.h>
#include <sys/utime.h>
#include <regex>
#else
#include <dirent.h>
#include <cerrno>
#include <unistd.h>
#include <utime.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <spawn.h>
#define MAX_PATH 256
#endif

#if defined(__APPLE__) && !IOS
#include <mach-o/dyld.h>
#include <crt_externs.h>
#define environ (*_NSGetEnviron())
#endif

extern "C"
{
#ifdef __ANDROID__
const char* SDL_Android_GetFilesDir();
char** SDL_Android_GetFileList(const char* path, int* count);
void SDL_Android_FreeFileList(char*** array, int* count);
#elif defined(IOS) || defined(TVOS)
const char* SDL_IOS_GetResourceDir();
const char* SDL_IOS_GetDocumentsDir();
#endif
}

#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif

namespace Se
{



namespace
{

bool ResolvePathSegment(String& sanitizedName, String::size_type& segmentStartIndex)
{
    if (sanitizedName.size() - segmentStartIndex <= 2)
    {
        const auto segment = sanitizedName.substr(segmentStartIndex);
        if (segment.empty())
        {
            // Keep leading /, otherwise skip empty segment.
            return segmentStartIndex == 0;
        }
        if (segment == ".")
        {
            sanitizedName.resize(segmentStartIndex);
            return false;
        }
        if (segment == "..")
        {
            // If there is a possibility of parent path
            if (segmentStartIndex > 1)
            {
                // Find where parent path starts
                segmentStartIndex = sanitizedName.find_last_of('/', segmentStartIndex - 2);
                // Find where parent path starts and set segment start right after / symbol.
                segmentStartIndex =
                    (segmentStartIndex == String::npos) ? 0 : segmentStartIndex + 1;
            }
            else
            {
                // If there is no way the parent path has parent of it's own then reset full path to empty.
                segmentStartIndex = 0;
            }
            // Reset sanitized name to position right after last known / or at the start.
            sanitizedName.resize(segmentStartIndex);
            return false;
        }
    }
    return true;
}

bool OpenURL(const String& url)
{
#ifdef HAVE_SDL
    const int error = SDL_OpenURL(url.c_str());
    if (error != 0)
    {
        SE_LOG_ERROR(SDL_GetError());
    }
    return !error;
#else
    return false;
#endif
}

bool StartsWith(const String& str, const String& prefix, bool caseSensitive = true)
{
    if (!caseSensitive)
        return str.starts_with(prefix);

    bool isOk = (caseSensitive ? strncmp(str.data(), prefix.data(), prefix.size()) : strncasecmp(str.data(), prefix.data(), prefix.size())) == 0;
//    bool isOk = std::CompareI(str.data(), prefix.data(), prefix.size()) == 0;

    return str.size() >= prefix.size() && isOk;
}

bool EndsWith(const String& str, const String& suffix, bool caseSensitive = true)
{
    if (!caseSensitive)
        return str.ends_with(suffix);

    auto strTmp = str.data() + str.size() - suffix.size();
    bool isOk = (caseSensitive ? strncmp(strTmp, suffix.data(), suffix.size()) : strncasecmp(strTmp, suffix.data(), suffix.size())) == 0;
//    bool isOk = std::CompareI(str.data() + str.size() - suffix.size(), suffix.data(), suffix.size()) == 0;

    return str.size() >= suffix.size() && isOk;
}

#ifdef _WIN32
using FileDescriptor = HANDLE;
#else
using FileDescriptor = int;
#endif
std::future<String> ReadFileAsync(FileDescriptor fileHandle, StopToken& stopToken)
{
    auto futureResult = std::async(std::launch::async,
        [fileHandle, stopToken]
    {
        String result;
        char buf[1024];
        while (true)
        {
            // Stop on error
#ifdef _WIN32
            DWORD bytesRead = 0;
            if (!ReadFile(fileHandle, buf, sizeof(buf), &bytesRead, nullptr))
            {
                if (GetLastError() != ERROR_NO_DATA)
                    break;
            }

            // Stop on EOF if stop was requested
            if (bytesRead == 0 && stopToken.IsStopped())
                break;
#else
            int bytesRead = read(fileHandle, buf, sizeof(buf));
            if (bytesRead < 0)
            {
                if (errno != EAGAIN)
                    break;
            }

            // Stop on EOF if stop was requested
            if (bytesRead <= 0 && stopToken.IsStopped())
                break;
#endif

            if (bytesRead > 0)
                result.append(buf, bytesRead);
        }
        return result;
    });
    return futureResult;
}

}

int DoSystemCommand(const String& commandLine, bool redirectToLog)
{
#if defined(TVOS) || defined(IOS) || defined(UWP) //|| !defined(HAVE_SDL)
    return -1;
#else
#  if !defined(__EMSCRIPTEN__)
    if (!redirectToLog)
#  endif
        return system(commandLine.c_str());


#  if !defined(__EMSCRIPTEN__)

    auto fs = FileSystem::Get();
    // Get a platform-agnostic temporary file name for stderr redirection
    String stderrFilename;
    String adjustedCommandLine(commandLine);
   auto prefPath = fs.GetAppPreferencesDir("Se", "temp");
    if (!prefPath.empty())
    {
        stderrFilename = String(prefPath) + "command-stderr";
        adjustedCommandLine += " 2>" + stderrFilename;
        //SDL_free(prefPath);
    }

#    ifdef _MSC_VER
#      define popen _popen
#      define pclose _pclose
#    endif


    // Use popen/pclose to capture the stdout and stderr of the command
    FILE* file = popen(adjustedCommandLine.c_str(), "r");
    if (!file)
        return -1;

    // Capture the standard output stream
    char buffer[0x2000];
    while (!feof(file))
    {
        if (fgets(buffer, sizeof(buffer), file))
        {
            String text(buffer);
            const char array[] = { ' ', '\t', '\r', '\n', 0 };
            text.erase(text.find_last_not_of(array) + 1);
            SE_LOG_INFO(text);
        }
    }
    int exitCode = pclose(file);

    // Capture the standard error stream
    if (!stderrFilename.empty())
    {
        auto errFile = std::make_shared<File>(stderrFilename, FILE_READ);
        while (!errFile->IsEof())
        {
            unsigned numRead = errFile->Read(buffer, sizeof(buffer));
            if (numRead)
                SE_LOG_ERROR(String(buffer, numRead));
        }
    }

    return exitCode;
#  endif
#endif
}

enum SystemRunFlag : unsigned
{
    SR_DEFAULT,
    SR_WAIT_FOR_EXIT,
    SR_READ_OUTPUT = 1u << 1u | SR_WAIT_FOR_EXIT,
};
SE_FLAGSET(SystemRunFlag, SystemRunFlags);

int DoSystemRun(const String& fileName, const std::vector<String>& arguments, SystemRunFlags flags, String& output)
{
    SE_LOG_DEBUG("Running system call:\n{} {}", fileName, String::joined(arguments, " "));

#if defined(TVOS) || defined(IOS) || (defined(__ANDROID__) && __ANDROID_API__ < 28) || defined(UWP)
    return -1;
#else
    String fixedFileName = GetNativePath(fileName);
    std::future<String> futureOutput;
    StopToken outputStopToken;
    auto stopGuard = MakeFinally([&] { 
        outputStopToken.Stop(); });

#ifdef _WIN32
    // Add .exe extension if no extension defined
    if (GetExtension(fixedFileName).empty())
        fixedFileName += ".exe";

    String commandLine = "\"" + fixedFileName + "\"";
    for (unsigned i = 0; i < arguments.size(); ++i)
        commandLine += " \"" + arguments[i] + "\"";

    STARTUPINFOW startupInfo{};
    PROCESS_INFORMATION processInfo{};
    startupInfo.cb = sizeof(startupInfo);

    std::wstring commandLineW = MultiByteToWide(commandLine);
    DWORD processFlags = 0;
    if (flags & SR_WAIT_FOR_EXIT)
        // If we are waiting for process result we are likely reading stdout, in that case we probably do not want to see a console window.
        processFlags = CREATE_NO_WINDOW;

    HANDLE pipeRead = 0;
    HANDLE pipeWrite = 0;
    const auto pipeGuard = MakeFinally([&]
    {
        if (pipeRead)
            CloseHandle(pipeRead);
        if (pipeWrite)
            CloseHandle(pipeWrite);
    });

    if (flags & SR_READ_OUTPUT)
    {
        SECURITY_ATTRIBUTES attr;
        attr.nLength = sizeof(SECURITY_ATTRIBUTES);
        attr.bInheritHandle = FALSE;
        attr.lpSecurityDescriptor = NULL;
        if (!CreatePipe(&pipeRead, &pipeWrite, &attr, 0))
            return -1;

        if (!SetHandleInformation(pipeRead, HANDLE_FLAG_INHERIT, 0))
            return -1;

        if (!SetHandleInformation(pipeWrite, HANDLE_FLAG_INHERIT, 1))
            return -1;

        DWORD mode = PIPE_NOWAIT;
        if (!SetNamedPipeHandleState(pipeRead, &mode, nullptr, nullptr))
            return -1;

        startupInfo.hStdOutput = pipeWrite;
        startupInfo.hStdError = pipeWrite;
        startupInfo.dwFlags |= STARTF_USESTDHANDLES;

        futureOutput = ReadFileAsync(pipeRead, outputStopToken);
    }

    if (!CreateProcessW(nullptr, (wchar_t*)commandLineW.c_str(), nullptr, nullptr, TRUE, processFlags, nullptr, nullptr, &startupInfo, &processInfo))
        return -1;

    DWORD exitCode = 0;
    if (flags & SR_WAIT_FOR_EXIT)
    {
        WaitForSingleObject(processInfo.hProcess, INFINITE);
        GetExitCodeProcess(processInfo.hProcess, &exitCode);
    }

    if (flags & SR_READ_OUTPUT)
    {
        stopGuard.Execute();
        output = futureOutput.get();
    }

    CloseHandle(processInfo.hProcess);
    CloseHandle(processInfo.hThread);

    return exitCode;
#else
    int desc[2] = {0, 0};
    if (flags & SR_READ_OUTPUT)
    {
        if (pipe(desc) == -1)
            return -1;
        fcntl(desc[0], F_SETFL, O_NONBLOCK);
        fcntl(desc[1], F_SETFL, O_NONBLOCK);
    }

    const auto pipeGuard = MakeFinally(
        [&]
    {
        if (desc[0])
            close(desc[0]);
        if (desc[1])
            close(desc[1]);
    });

    std::vector<const char*> argPtrs;
    argPtrs.push_back(fixedFileName.c_str());
    for (unsigned i = 0; i < arguments.size(); ++i)
        argPtrs.push_back(arguments[i].c_str());
    argPtrs.push_back(nullptr);

    pid_t pid = 0;
    posix_spawn_file_actions_t actions{};
    posix_spawn_file_actions_init(&actions);
    if (flags & SR_READ_OUTPUT)
    {
        posix_spawn_file_actions_addclose(&actions, STDOUT_FILENO);
        posix_spawn_file_actions_adddup2(&actions, desc[1], STDOUT_FILENO);
        posix_spawn_file_actions_addclose(&actions, STDERR_FILENO);
        posix_spawn_file_actions_adddup2(&actions, desc[1], STDERR_FILENO);

        futureOutput = ReadFileAsync(desc[0], outputStopToken);
    }
    posix_spawnp(&pid, fixedFileName.c_str(), &actions, nullptr, (char**)&argPtrs[0], environ);
    posix_spawn_file_actions_destroy(&actions);

    if (pid > 0)
    {
        int exitCode = 0;
        if (flags & SR_WAIT_FOR_EXIT)
            waitpid(pid, &exitCode, 0);

        if (flags & SR_READ_OUTPUT)
        {
            stopGuard.Execute();
            output = futureOutput.get();
        }
        return exitCode;
    }
    else
        return -1;
#endif
#endif
}

/// Base class for async execution requests.
class AsyncExecRequest : public Thread
{
public:
    /// Construct.
    explicit AsyncExecRequest(unsigned& requestID) :
        requestID_(requestID)
    {
        // Increment ID for next request
        ++requestID;
        if (requestID == M_MAX_UNSIGNED)
            requestID = 1;
    }

    /// Return request ID.
    unsigned GetRequestID() const { return requestID_; }

    /// Return exit code. Valid when IsCompleted() is true.
    int GetExitCode() const { return exitCode_; }

    /// Return completion status.
    bool IsCompleted() const { return completed_; }

protected:
    /// Request ID.
    unsigned requestID_{};
    /// Exit code.
    int exitCode_{};
    /// Completed flag.
    volatile bool completed_{};
};

/// Async system command operation.
class AsyncSystemCommand : public AsyncExecRequest
{
public:
    /// Construct and run.
    AsyncSystemCommand(unsigned requestID, const String& commandLine) :
        AsyncExecRequest(requestID),
        commandLine_(commandLine)
    {
        Run();
    }

    /// The function to run in the thread.
    void ThreadFunction() override
    {
        //SE_PROFILE_THREAD("AsyncSystemCommand Thread");

        exitCode_ = DoSystemCommand(commandLine_, false);
        completed_ = true;
    }

private:
    /// Command line.
    String commandLine_;
};

/// Async system run operation.
class AsyncSystemRun : public AsyncExecRequest
{
public:
    /// Construct and run.
    AsyncSystemRun(unsigned requestID, const String& fileName, const std::vector<String>& arguments) :
        AsyncExecRequest(requestID),
        fileName_(fileName),
        arguments_(arguments)
    {
        Run();
    }

    /// The function to run in the thread.
    void ThreadFunction() override
    {
//        SE_PROFILE_THREAD("AsyncSystemRun Thread");
        String output;
        exitCode_ = DoSystemRun(fileName_, arguments_, SR_WAIT_FOR_EXIT, output);
        completed_ = true;
    }

private:
    /// File to run.
    String fileName_;
    /// Command line split in arguments.
    const std::vector<String>& arguments_;
};

FileSystem::FileSystem()
{
//    SubscribeToEvent(E_BEGINFRAME, SE_HANDLER(FileSystem, HandleBeginFrame));
}

FileSystem::~FileSystem()
{
    // If any async exec items pending, delete them
    if (asyncExecQueue_.size())
    {
        for (auto i = asyncExecQueue_.begin(); i != asyncExecQueue_.end(); ++i)
            delete(*i);

        asyncExecQueue_.clear();
    }
}

bool FileSystem::SetCurrentDir(const String& pathName)
{
    if (!CheckAccess(pathName))
    {
        SE_LOG_ERROR("Access denied to " + pathName);
        return false;
    }
#ifdef _WIN32
    if (SetCurrentDirectoryW(GetWideNativePath(pathName).c_str()) == FALSE)
    {
        SE_LOG_ERROR("Failed to change directory to " + pathName);
        return false;
    }
#else
    if (chdir(GetNativePath(pathName).c_str()) != 0)
    {
        SE_LOG_ERROR("Failed to change directory to " + pathName);
        return false;
    }
#endif

    return true;
}

bool FileSystem::CreateDir(const String& pathName)
{
    if (!CheckAccess(pathName))
    {
        SE_LOG_ERROR("Access denied to " + pathName);
        return false;
    }

    // Create each of the parents if necessary
    String parentPath = GetParentPath(pathName);
    if (parentPath.length() > 1 && !DirExists(parentPath))
    {
        if (!CreateDir(parentPath))
            return false;
    }

#ifdef _WIN32
    bool success = (CreateDirectoryW(GetWideNativePath(RemoveTrailingSlash(pathName)).c_str(), nullptr) == TRUE) ||
        (GetLastError() == ERROR_ALREADY_EXISTS);
#else
    bool success = mkdir(GetNativePath(RemoveTrailingSlash(pathName)).c_str(), S_IRWXU) == 0 || errno == EEXIST;
#endif

    if (success)
        SE_LOG_DEBUG("Created directory " + pathName);
    else
        SE_LOG_ERROR("Failed to create directory " + pathName);

    return success;
}

void FileSystem::SetExecuteConsoleCommands(bool enable)
{
#if SE_SYSTEMUI
    if (enable == executeConsoleCommands_)
        return;

    executeConsoleCommands_ = enable;
    if (enable)
        SubscribeToEvent(E_CONSOLECOMMAND, SE_HANDLER(FileSystem, HandleConsoleCommand));
    else
        UnsubscribeFromEvent(E_CONSOLECOMMAND);

    Console* console = GetSubsystem<Console>();
    console->RefreshInterpreters();
#else
    SE_LOG_WARNING("Engine was built without console support.");
#endif
}

int FileSystem::SystemCommand(const String& commandLine, bool redirectStdOutToLog)
{
    if (allowedPaths_.empty())
        return DoSystemCommand(commandLine, redirectStdOutToLog);
    else
    {
        SE_LOG_ERROR("Executing an external command is not allowed");
        return -1;
    }
}

int FileSystem::SystemRun(const String& fileName, const std::vector<String>& arguments, String& output)
{
    if (allowedPaths_.empty())
        return DoSystemRun(fileName, arguments, SR_READ_OUTPUT, output);
    else
    {
        SE_LOG_ERROR("Executing an external command is not allowed");
        return -1;
    }
}

int FileSystem::SystemRun(const String& fileName, const std::vector<String>& arguments)
{
    String output;
    if (allowedPaths_.empty())
        return DoSystemRun(fileName, arguments, SR_WAIT_FOR_EXIT, output);
    else
    {
        SE_LOG_ERROR("Executing an external command is not allowed");
        return -1;
    }
}

int FileSystem::SystemSpawn(const String& fileName, const std::vector<String>& arguments)
{
    String output;
    if (allowedPaths_.empty())
        return DoSystemRun(fileName, arguments, SR_DEFAULT, output);
    else
    {
        SE_LOG_ERROR("Executing an external command is not allowed");
        return -1;
    }
}

unsigned FileSystem::SystemCommandAsync(const String& commandLine)
{
#ifdef SE_THREADING
    if (allowedPaths_.empty())
    {
        unsigned requestID = nextAsyncExecID_;
        auto* cmd = new AsyncSystemCommand(nextAsyncExecID_, commandLine);
        asyncExecQueue_.push_back(cmd);
        return requestID;
    }
    else
    {
        SE_LOG_ERROR("Executing an external command is not allowed");
        return M_MAX_UNSIGNED;
    }
#else
    SE_LOG_ERROR("Can not execute an asynchronous command as threading is disabled");
    return M_MAX_UNSIGNED;
#endif
}

unsigned FileSystem::SystemRunAsync(const String& fileName, const std::vector<String>& arguments)
{
#ifdef SE_THREADING
    if (allowedPaths_.empty())
    {
        unsigned requestID = nextAsyncExecID_;
        auto* cmd = new AsyncSystemRun(nextAsyncExecID_, fileName, arguments);
        asyncExecQueue_.push_back(cmd);
        return requestID;
    }
    else
    {
        SE_LOG_ERROR("Executing an external command is not allowed");
        return M_MAX_UNSIGNED;
    }
#else
    SE_LOG_ERROR("Can not run asynchronously as threading is disabled");
    return M_MAX_UNSIGNED;
#endif
}

bool FileSystem::SystemOpen(const String& fileName, const String& mode)
{
    if (allowedPaths_.empty())
    {
        if (fileName.starts_with("http://") || fileName.starts_with("https://"))
        {
            return OpenURL(fileName);
        }

        // allow opening of http and file urls
        if (!fileName.starts_with("file://"))
        {
            if (!FileExists(fileName) && !DirExists(fileName))
            {
                SE_LOG_ERROR("File or directory " + fileName + " not found");
                return false;
            }
        }
#ifdef UWP
        bool success = false;
#elif defined(_WIN32)
        bool success = (size_t)ShellExecuteW(nullptr, !mode.empty() ? MultiByteToWide(mode).c_str() : nullptr,
            GetWideNativePath(fileName).c_str(), nullptr, nullptr, SW_SHOW) > 32;
#else
        std::vector<String> arguments;
        arguments.push_back(fileName);
        bool success = SystemRun(
#if defined(__APPLE__)
            "/usr/bin/open",
#else
            "/usr/bin/xdg-open",
#endif
            arguments) == 0;
#endif
        if (!success)
            SE_LOG_ERROR("Failed to open " + fileName + " externally");
        return success;
    }
    else
    {
        SE_LOG_ERROR("Opening a file externally is not allowed");
        return false;
    }
}

bool FileSystem::Copy(const String& srcFileName, const String& destFileName)
{
    if (!CheckAccess(GetPath(srcFileName)))
    {
        SE_LOG_ERROR("Access denied to " + srcFileName);
        return false;
    }
    if (!CheckAccess(GetPath(destFileName)))
    {
        SE_LOG_ERROR("Access denied to " + destFileName);
        return false;
    }

    auto srcFile = std::make_shared<File>(srcFileName, FILE_READ);
    if (!srcFile->IsOpen())
        return false;
    auto destFile = std::make_shared<File>(destFileName, FILE_WRITE);
    if (!destFile->IsOpen())
        return false;

    unsigned fileSize = srcFile->GetSize();
    std::shared_ptr<unsigned char> buffer(new unsigned char[fileSize], std::default_delete<unsigned char[]>());

    unsigned bytesRead = srcFile->Read(buffer.get(), fileSize);
    unsigned bytesWritten = destFile->Write(buffer.get(), fileSize);
    return bytesRead == fileSize && bytesWritten == fileSize;
}

bool FileSystem::Rename(const String& srcFileName, const String& destFileName)
{
    if (!CheckAccess(GetPath(srcFileName)))
    {
        SE_LOG_ERROR("Access denied to " + srcFileName);
        return false;
    }
    if (!CheckAccess(GetPath(destFileName)))
    {
        SE_LOG_ERROR("Access denied to " + destFileName);
        return false;
    }

#ifdef UWP
    return false;
#elif defined(_WIN32)
    return MoveFileW(GetWideNativePath(srcFileName).c_str(), GetWideNativePath(destFileName).c_str()) != 0;
#else
    return rename(GetNativePath(srcFileName).c_str(), GetNativePath(destFileName).c_str()) == 0;
#endif
}

bool FileSystem::Delete(const String& fileName)
{
    if (!CheckAccess(GetPath(fileName)))
    {
        SE_LOG_ERROR("Access denied to " + fileName);
        return false;
    }

#ifdef _WIN32
    return DeleteFileW(GetWideNativePath(fileName).c_str()) != 0;
#else
    return remove(GetNativePath(fileName).c_str()) == 0;
#endif
}

String FileSystem::GetCurrentDir() const
{
#ifdef _WIN32
    wchar_t path[MAX_PATH];
    path[0] = 0;
    GetCurrentDirectoryW(MAX_PATH, path);
    return AddTrailingSlash(WideToMultiByte(path));
#else
    char path[MAX_PATH];
    path[0] = 0;
    getcwd(path, MAX_PATH);
    return AddTrailingSlash(String(path));
#endif
}

bool FileSystem::CheckAccess(const String& pathName) const
{
    String fixedPath = AddTrailingSlash(pathName);

    // If no allowed directories defined, succeed always
    if (allowedPaths_.empty())
        return true;

    // If there is any attempt to go to a parent directory, disallow
    if (fixedPath.contains(".."))
        return false;

    // Check if the path is a partial match of any of the allowed directories
    for (auto i = allowedPaths_.begin(); i != allowedPaths_.end(); ++i)
    {
        if (fixedPath.find(*i) == 0)
            return true;
    }

    // Not found, so disallow
    return false;
}

FileTime FileSystem::GetLastModifiedTime(const String& fileName, bool creationIsModification) const
{
    if (fileName.empty() || !CheckAccess(fileName))
        return 0;

#ifdef _WIN32
    struct _stat st;
    if (!_stat(fileName.c_str(), &st))
        return static_cast<FileTime>(std::max(st.st_mtime, creationIsModification ? st.st_ctime : time_t{}));
    else
        return 0;
#else
    struct stat st{};
    if (!stat(fileName.c_str(), &st))
        return static_cast<FileTime>(st.st_mtime);
    else
        return 0;
#endif
}

bool FileSystem::FileExists(const String& fileName) const
{
    if (!CheckAccess(GetPath(fileName)))
        return false;

#ifdef __ANDROID__
    if (SE_IS_ASSET(fileName))
    {
        SDL_RWops* rwOps = SDL_RWFromFile(SE_ASSET(fileName), "rb");
        if (rwOps)
        {
            SDL_RWclose(rwOps);
            return true;
        }
        else
            return false;
    }
#endif

    String fixedName = GetNativePath(RemoveTrailingSlash(fileName));

#ifdef _WIN32
    DWORD attributes = GetFileAttributesW(MultiByteToWide(fixedName).c_str());
    if (attributes == INVALID_FILE_ATTRIBUTES || attributes & FILE_ATTRIBUTE_DIRECTORY)
        return false;
#else
    struct stat st{};
    if (stat(fixedName.c_str(), &st) || st.st_mode & S_IFDIR)
        return false;
#endif

    return true;
}

bool FileSystem::DirExists(const String& pathName) const
{
    if (!CheckAccess(pathName))
        return false;

#ifndef _WIN32
    // Always return true for the root directory
    if (pathName == "/")
        return true;
#endif

    String fixedName = GetNativePath(RemoveTrailingSlash(pathName));

#ifdef __ANDROID__
    if (SE_IS_ASSET(fixedName))
    {
        // Split the pathname into two components: the longest parent directory path and the last name component
        String assetPath(SE_ASSET((fixedName + "/")));
        String parentPath;
        unsigned pos = assetPath.find_last_of('/', assetPath.length() - 2);
        if (pos != String::npos)
        {
            parentPath = assetPath.substr(0, pos);
            assetPath = assetPath.substr(pos + 1);
        }
        assetPath.resize(assetPath.length() - 1);

        bool exist = false;
        int count;
        char** list = SDL_Android_GetFileList(parentPath.c_str(), &count);
        for (int i = 0; i < count; ++i)
        {
            exist = assetPath == list[i];
            if (exist)
                break;
        }
        SDL_Android_FreeFileList(&list, &count);
        return exist;
    }
#endif

#if defined(_WIN32) && !defined(UWP)
    DWORD attributes = GetFileAttributesW(MultiByteToWide(fixedName).c_str());
    if (attributes == INVALID_FILE_ATTRIBUTES || !(attributes & FILE_ATTRIBUTE_DIRECTORY))
        return false;
#else
    struct stat st{};
    if (stat(fixedName.c_str(), &st) || !(st.st_mode & S_IFDIR))
        return false;
#endif

    return true;
}

void FileSystem::ScanDir(std::vector<String>& result, const String& pathName, const String& filter, ScanFlags flags) const
{
    if (!flags.Test(SCAN_APPEND))
        result.clear();

    if (CheckAccess(pathName))
    {
        String initialPath = AddTrailingSlash(pathName);
        ScanDirInternal(result, initialPath, initialPath, filter, flags);
    }
}

void FileSystem::ScanDirTree(DirectoryNode& result, const String& pathName, const String& filter, ScanFlags flags) const
{
    if (!flags.Test(SCAN_APPEND))
        result.Children.clear();

    if (CheckAccess(pathName))
    {
        String initialPath = AddTrailingSlash(pathName);
        ScanDirInternalTree(result, initialPath, initialPath, filter, flags);
    }
}

String FileSystem::GetProgramDir() const
{
#ifdef UWP
    return AddTrailingSlash(WideToMultiByte(Windows::ApplicationModel::Package::Current->InstalledLocation->Path->Data()));
#elif defined(__ANDROID__)
    // This is an internal directory specifier pointing to the assets in the .apk
    // Files from this directory will be opened using special handling
    return APK;
#elif defined(IOS) || defined(TVOS)
    return AddTrailingSlash(SDL_IOS_GetResourceDir());
#elif DESKTOP
    return GetPath(GetProgramFileName());
#else
    return GetCurrentDir();
#endif
}

String FileSystem::GetProgramFileName() const
{
#ifndef DESKTOP
    SE_LOG_ERROR("Program name is not available on current platform.");
#elif defined(_WIN32)
    wchar_t exeName[MAX_PATH];
    exeName[0] = 0;
    GetModuleFileNameW(nullptr, exeName, MAX_PATH);
    return WideToMultiByte(exeName);
#elif defined(__APPLE__)
    char exeName[MAX_PATH];
    memset(exeName, 0, MAX_PATH);
    unsigned size = MAX_PATH;
    _NSGetExecutablePath(exeName, &size);
    return String(exeName);
#elif defined(__linux__)
    char exeName[MAX_PATH];
    memset(exeName, 0, MAX_PATH);
    pid_t pid = getpid();
    String link(String::CtorSprintf{}, "/proc/%d/exe", pid);
    readlink(link.c_str(), exeName, MAX_PATH);
    return String(exeName);
#endif
    return "";
}

String FileSystem::GetUserDocumentsDir() const
{
#if defined(UWP)
    return AddTrailingSlash(WideToMultiByte(Windows::Storage::ApplicationData::Current->LocalFolder->Path->Data()));
#elif defined(__ANDROID__)
    return AddTrailingSlash(SDL_Android_GetFilesDir());
#elif defined(IOS) || defined(TVOS)
    return AddTrailingSlash(SDL_IOS_GetDocumentsDir());
#elif defined(_WIN32)
    wchar_t pathName[MAX_PATH];
    pathName[0] = 0;
    SHGetSpecialFolderPathW(nullptr, pathName, CSIDL_PERSONAL, 0);
    return AddTrailingSlash(WideToMultiByte(pathName));
#else
    char pathName[MAX_PATH];
    pathName[0] = 0;
    strcpy(pathName, getenv("HOME"));
    return AddTrailingSlash(String(pathName));
#endif
}

String FileSystem::GetENV(const String& envName) const
{
    String path;
#ifdef __linux__
    const char* env = getenv(envName.c_str());
    if (env)
        path = std::move(env);
#elif _WIN32
    char buffer[1024];
    DWORD bufferSize = 1024;

    // Call GetEnvironmentVariable to retrieve the value
    if (GetEnvironmentVariableA(envName.c_str(), buffer, bufferSize))
        path = std::move(buffer);
#else
    SE_LOG_WARNING("Could not get environment: {}", envName);
#endif
    return path;
}

String FileSystem::GetAppPreferencesDir(const String& org, const String& app) const
{
    String dir;
#ifdef __linux__
    auto env  = GetENV("XDG_DATA_HOME");
    if (env.empty())
        env  = GetENV("HOME"); 
    dir = format("{}/.local/share/", env);

    //__ANDROID__ Windows
#elif HAVE_SDL
    char* prefPath = SDL_GetPrefPath(org.c_str(), app.c_str());
    if (prefPath)
    {
        dir = GetInternalPath(String(prefPath));
        SDL_free(prefPath);
    }
#elif _WIN32
    dir = GetENV("LOCALAPPDATA");
#else
    SE_LOG_WARNING("Could not get application preferences directory. this platform don't support");
    return dir;
#endif
    if (dir.empty())
        SE_LOG_WARNING("Could not get application preferences directory");

    dir += format("{}/{}/", org, app);

    return dir;
}

void FileSystem::RegisterPath(const String& pathName)
{
    if (pathName.empty())
        return;

    allowedPaths_.insert(AddTrailingSlash(pathName));
}

bool FileSystem::SetLastModifiedTime(const String& fileName, FileTime newTime)
{
    if (fileName.empty() || !CheckAccess(fileName))
        return false;

#ifdef _WIN32
    struct _stat oldTime;
    struct _utimbuf newTimes;
    if (_stat(fileName.c_str(), &oldTime) != 0)
        return false;
    newTimes.actime = oldTime.st_atime;
    newTimes.modtime = newTime;
    return _utime(fileName.c_str(), &newTimes) == 0;
#else
    struct stat oldTime{};
    struct utimbuf newTimes{};
    if (stat(fileName.c_str(), &oldTime) != 0)
        return false;
    newTimes.actime = oldTime.st_atime;
    newTimes.modtime = newTime;
    return utime(fileName.c_str(), &newTimes) == 0;
#endif
}

bool FileSystem::Reveal(const String& path)
{
#ifdef _WIN32
    return SystemCommand(format("start explorer.exe /select,{}", GetNativePath(path))) == 0;
#elif defined(__APPLE__)
    return SystemCommand(format("open -R {}", GetNativePath(path))) == 0;
#elif defined(__linux__)
    return SystemCommand(format("dbus-send "
        "--session --print-reply --dest=org.freedesktop.FileManager1 --type=method_call "
        "/org/freedesktop/FileManager1 org.freedesktop.FileManager1.ShowItems array:string:\"file://{}\" string:\"\"", GetNativePath(path)).c_str()) == 0;
#else
    return false;
#endif
}

void FileSystem::ScanDirInternal(std::vector<String>& result, const String& path, const String& startPath,
    const String& filter, ScanFlags flags) const
{
    const bool recursive = flags.Test(SCAN_RECURSIVE);

    String pathTmp = AddTrailingSlash(path);
    String deltaPath;
    if (pathTmp.length() > startPath.length())
        deltaPath = pathTmp.substr(startPath.length());

    const String filterExtension = GetExtensionFromFilter(filter);

#ifdef __ANDROID__
    if (SE_IS_ASSET(pathTmp))
    {
        String assetPath(SE_ASSET(pathTmp));
        assetPath = RemoveTrailingSlash(assetPath);       // AssetManager.list() does not like trailing slash
        int count;
        char** list = SDL_Android_GetFileList(assetPath.c_str(), &count);
        for (int i = 0; i < count; ++i)
        {
            String fileName(list[i]);
            if (!(flags & SCAN_HIDDEN) && fileName.starts_with("."))
                continue;

#ifdef ASSET_DIR_INDICATOR
            // Patch the directory name back after retrieving the directory flag
            bool isDirectory = fileName.ends_with(ASSET_DIR_INDICATOR);
            if (isDirectory)
            {
                fileName.resize(fileName.length() - sizeof(ASSET_DIR_INDICATOR) / sizeof(char) + 1);
                if (flags & SCAN_DIRS)
                    result.push_back(deltaPath + fileName);
                if (recursive)
                    ScanDirInternal(result, pathTmp + fileName, startPath, filter, flags);
            }
            else if (flags & SCAN_FILES)
#endif
            {
                if (filterExtension.empty() || fileName.ends_with(filterExtension))
                    result.push_back(deltaPath + fileName);
            }
        }
        SDL_Android_FreeFileList(&list, &count);
        return;
    }
#endif
#ifdef _WIN32
    WIN32_FIND_DATAW info;
    HANDLE handle = FindFirstFileW(MultiByteToWide((pathTmp + "*")).c_str(), &info);
    if (handle != INVALID_HANDLE_VALUE)
    {
        do
        {
            String fileName = WideToMultiByte(info.cFileName);
            if (!fileName.empty())
            {
                if (info.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN && !(flags & SCAN_HIDDEN))
                    continue;
                if (info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                {
                    if (flags & SCAN_DIRS)
                        result.emplace_back(deltaPath + fileName);
                    if (recursive && fileName != "." && fileName != "..")
                        ScanDirInternal(result, pathTmp + fileName, startPath, filter, flags);
                }
                else if (flags & SCAN_FILES)
                {
                    if (filterExtension.empty() || fileName.ends_with(filterExtension))
                        result.emplace_back(deltaPath + fileName);
                }
            }
        }
        while (FindNextFileW(handle, &info));

        FindClose(handle);
    }
#else
    DIR* dir;
    struct dirent* de;
    struct stat st{};
    dir = opendir(GetNativePath(pathTmp).c_str());
    if (dir)
    {
        while ((de = readdir(dir)))
        {
            /// \todo Filename may be unnormalized Unicode on Mac OS X. Re-normalize as necessary
            String fileName(de->d_name);
            bool normalEntry = fileName != "." && fileName != "..";
            if (normalEntry && !(flags & SCAN_HIDDEN) && fileName.starts_with("."))
                continue;
            String pathAndName = pathTmp + fileName;
            if (!stat(pathAndName.c_str(), &st))
            {
                if (st.st_mode & S_IFDIR)
                {
                    if (flags & SCAN_DIRS)
                        result.push_back(deltaPath + fileName);
                    if (recursive && normalEntry)
                        ScanDirInternal(result, pathTmp + fileName, startPath, filter, flags);
                }
                else if (flags & SCAN_FILES)
                {
                    if (filterExtension.empty() || fileName.ends_with(filterExtension))
                        result.push_back(deltaPath + fileName);
                }
            }
        }
        closedir(dir);
    }
#endif
}


void FileSystem::ScanDirInternalTree(DirectoryNode& result, const Se::String& path, const Se::String& startPath,
    const Se::String& filter, Se::ScanFlags flags) const
{
    const bool recursive = flags.Test(Se::SCAN_RECURSIVE);

    Se::String pathTmp = Se::AddTrailingSlash(path);
    Se::String deltaPath;
    if (pathTmp.length() > startPath.length())
        deltaPath = pathTmp.substr(startPath.length());

    const Se::String filterExtension = GetExtensionFromFilter(filter);

#ifdef __ANDROID__
    if (SE_IS_ASSET(pathTmp))
    {
        String assetPath(SE_ASSET(pathTmp));
        assetPath = RemoveTrailingSlash(assetPath);       // AssetManager.list() does not like trailing slash
        int count;
        char** list = SDL_Android_GetFileList(assetPath.c_str(), &count);
        for (int i = 0; i < count; ++i)
        {
            String fileName(list[i]);
            if (!(flags & SCAN_HIDDEN) && fileName.starts_with("."))
                continue;

#ifdef ASSET_DIR_INDICATOR
            // Patch the directory name back after retrieving the directory flag
            bool isDirectory = fileName.ends_with(ASSET_DIR_INDICATOR);
            if (isDirectory)
            {
                fileName.resize(fileName.length() - sizeof(ASSET_DIR_INDICATOR) / sizeof(char) + 1);
                if (flags & SCAN_DIRS) {
                    auto& node = result.Children.emplace_back();
                    node.FullPath = deltaPath + fileName;
                    node.FileName = fileName;
                    node.IsDirectory = true;
                    //result.push_back(deltaPath + fileName);
                }
                if (recursive)
                    ScanDirInternalTree(result, pathTmp + fileName, startPath, filter, flags);
            }
            else if (flags & SCAN_FILES)
#endif
            {
                if (filterExtension.empty() || fileName.ends_with(filterExtension)) {
                    auto& node = result.Children.emplace_back();
                    node.FullPath = deltaPath + fileName;
                    node.FileName = fileName;
                    node.IsDirectory = false;
                    //result.push_back(deltaPath + fileName);
                }
            }
        }
        SDL_Android_FreeFileList(&list, &count);
        return;
    }
#endif
#ifdef _WIN32
    WIN32_FIND_DATAW info;
    HANDLE handle = FindFirstFileW(MultiByteToWide((pathTmp + "*")).c_str(), &info);
    if (handle != INVALID_HANDLE_VALUE)
    {
        do
        {
            String fileName = WideToMultiByte(info.cFileName);
            if (!fileName.empty())
            {
                if (info.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN && !(flags & SCAN_HIDDEN))
                    continue;
                if (info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                {
                    if (flags & SCAN_DIRS && recursive && fileName != "." && fileName != "..") {

                        auto& node = result.Children.emplace_back();
                        node.FullPath = deltaPath + fileName;
                        node.FileName = fileName;
                        node.Flags |= FSIF_DIRECTORY;

                        if (recursive)
                            ScanDirInternalTree(node, pathTmp + fileName, startPath, filter, flags);
                    }
                }
                else if (flags & SCAN_FILES)
                {
                    if (filterExtension.empty() || fileName.ends_with(filterExtension)) {
                        auto& node = result.Children.emplace_back();
                        node.FullPath = deltaPath + fileName;
                        node.FileName = fileName;
                        if (node.Flags.Test(FSIF_DIRECTORY)) 
                            node.Flags.Unset(FSIF_DIRECTORY); //IsDirectory = false;
                    }
                }
            }
        }
        while (FindNextFileW(handle, &info));

        FindClose(handle);
    }
#else
    DIR* dir;
    struct dirent* de;
    struct stat st{};
    dir = opendir(GetNativePath(pathTmp).c_str());
    if (dir)
    {
        while ((de = readdir(dir)))
        {
            /// \todo Filename may be unnormalized Unicode on Mac OS X. Re-normalize as necessary
            String fileName(de->d_name);
            bool normalEntry = fileName != "." && fileName != "..";
            if (normalEntry && !(flags & SCAN_HIDDEN) && fileName.starts_with("."))
                continue;
            String pathAndName = pathTmp + fileName;
            if (!stat(pathAndName.c_str(), &st))
            {
                if (st.st_mode & S_IFDIR)
                {
                    if (recursive && normalEntry) {
                        auto& node = result.Children.emplace_back();
                        node.FullPath = deltaPath + fileName;
                        node.FileName = fileName;
                        node.Flags |= FSIF_DIRECTORY;
                        ScanDirInternalTree(node, pathTmp + fileName, startPath, filter, flags);
                    }
                }
                else if (flags & SCAN_FILES)
                {
                    if (filterExtension.empty() || fileName.ends_with(filterExtension)) {
                        auto& node = result.Children.emplace_back();
                        node.FullPath = deltaPath + fileName;
                        node.FileName = fileName;
                        if (node.Flags.Test(FSIF_DIRECTORY)) 
                            node.Flags.Unset(FSIF_DIRECTORY); //IsDirectory = false;
                    }
                        
                }
            }
        }
        closedir(dir);
    }
#endif
}

// void FileSystem::HandleBeginFrame(StringHash eventType, VariantMap& eventData)
// {
//     // Go through the execution queue and post + remove completed requests
//     for (auto i = asyncExecQueue_.begin(); i != asyncExecQueue_.end();)
//     {
//         AsyncExecRequest* request = *i;
//         if (request->IsCompleted())
//         {
//             using namespace AsyncExecFinished;

//             VariantMap& newEventData = GetEventDataMap();
//             newEventData[P_REQUESTID] = request->GetRequestID();
//             newEventData[P_EXITCODE] = request->GetExitCode();
//             SendEvent(E_ASYNCEXECFINISHED, newEventData);

//             delete request;
//             i = asyncExecQueue_.erase(i);
//         }
//         else
//             ++i;
//     }
// }

// void FileSystem::HandleConsoleCommand(StringHash eventType, VariantMap& eventData)
// {
//     using namespace ConsoleCommand;
//     if (eventData[P_ID].GetString() == GetTypeName())
//         SystemCommand(eventData[P_COMMAND].GetString(), true);
// }

TemporaryDir::TemporaryDir(FileSystem* fs, const String& path)
    : fs_(fs)
    , path_(AddTrailingSlash(path))
{
    fs_->CreateDirsRecursive(path_);
}

TemporaryDir::~TemporaryDir()
{
    if (fs_)
        fs_->RemoveDir(path_, true);
}

TemporaryDir::TemporaryDir(TemporaryDir&& rhs)
{
    *this = std::move(rhs);
}

TemporaryDir& TemporaryDir::operator=(TemporaryDir&& rhs)
{
    fs_ = rhs.fs_;
    path_ = std::move(rhs.path_);
    rhs.fs_ = nullptr;
    return *this;
}

void SplitPath(const String& fullPath, String& pathName, String& fileName, String& extension, bool lowercaseExtension)
{
    String fullPathCopy = GetInternalPath(fullPath);

    std::size_t extPos = fullPathCopy.find_last_of('.');
    std::size_t pathPos = fullPathCopy.find_last_of('/');

    if (extPos != String::npos && (pathPos == String::npos || extPos > pathPos))
    {
        extension = fullPathCopy.substr(extPos);
        if (lowercaseExtension)
            extension.to_lower();
        fullPathCopy = fullPathCopy.substr(0, extPos);
    }
    else
        extension.clear();

    pathPos = fullPathCopy.find_last_of('/');
    if (pathPos != String::npos)
    {
        fileName = fullPathCopy.substr(pathPos + 1);
        pathName = fullPathCopy.substr(0, pathPos + 1);
    }
    else
    {
        fileName = fullPathCopy;
        pathName.clear();
    }
}

String GetPath(const String& fullPath)
{
    String path, file, extension;
    SplitPath(fullPath, path, file, extension);
    return path;
}

String GetFileName(const String& fullPath)
{
    String path, file, extension;
    SplitPath(fullPath, path, file, extension);
    return file;
}

String GetExtension(const String& fullPath, bool lowercaseExtension)
{
    String path, file, extension;
    SplitPath(fullPath, path, file, extension, lowercaseExtension);
    return extension;
}

String GetFileNameAndExtension(const String& fileName, bool lowercaseExtension)
{
    String path, file, extension;
    SplitPath(fileName, path, file, extension, lowercaseExtension);
    return file + extension;
}

String ReplaceExtension(const String& fullPath, const String& newExtension)
{
    String path, file, extension;
    SplitPath(fullPath, path, file, extension);
    return path + file + newExtension;
}

String AddTrailingSlash(const String& pathName)
{
    String ret = pathName.trimmed();
    ret.replace('\\', '/');
    if (!ret.empty() && ret.back() != '/')
        ret += '/';
    return ret;
}

String RemoveTrailingSlash(const String& pathName)
{
    String ret = pathName.trimmed();
    ret.replace('\\', '/');
    if (!ret.empty() && ret.back() == '/')
        ret.resize(ret.length() - 1);
    return ret;
}

String GetParentPath(const String& path)
{
    std::size_t pos = RemoveTrailingSlash(path).find_last_of('/');
    if (pos != String::npos)
        return path.substr(0, pos + 1);
    else
        return String();
}

String GetInternalPath(const String& pathName)
{
    return pathName.replaced('\\', '/');
}

String GetNativePath(const String& pathName)
{
#ifdef _WIN32
    return pathName.replaced('/', '\\');
#else
    return pathName;
#endif
}

std::wstring GetWideNativePath(const String& pathName)
{
    std::wstring result;
#ifdef _WIN32
    result = MultiByteToWide(pathName.c_str());

    result = std::regex_replace(result, std::wregex(L"/"), L"\\");
    //result.replace(L'/', L'\\');
#endif
    return result;
}

bool IsAbsolutePath(const String& pathName)
{
    if (pathName.empty())
        return false;

    String path = GetInternalPath(pathName);

    if (path.at(0) == '/')
        return true;

#ifdef _WIN32
    if (path.length() > 1 && IsAlpha(path[0]) && path[1] == ':')
        return true;
#endif

    return false;
}

bool FileSystem::CreateDirs(const String& root, const String& subdirectory)
{
    String folder = AddTrailingSlash(GetInternalPath(root));
    String sub = GetInternalPath(subdirectory);
    std::vector<String> subs = sub.split('/');

    for (unsigned i = 0; i < subs.size(); i++)
    {
        folder += subs[i];
        folder += "/";

        if (DirExists(folder))
            continue;

        CreateDir(folder);

        if (!DirExists(folder))
            return false;
    }

    return true;

}

bool FileSystem::CreateDirsRecursive(const String& directoryIn)
{
    String directory = AddTrailingSlash(GetInternalPath(directoryIn));

    if (DirExists(directory))
        return true;

    if (FileExists(directory))
        return false;

    String parentPath = directory;

    std::vector<String> paths;

    paths.push_back(directory);

    for (;;)
    {
        parentPath = GetParentPath(parentPath);

        if (!parentPath.length())
            break;

        if (DirExists(parentPath))
            break;

        paths.push_back(parentPath);
    }

    if (!paths.size())
        return false;

    for (auto i = (int) (paths.size() - 1); i >= 0; i--)
    {
        const String& pathName = paths[i];

        if (FileExists(pathName))
            return false;

        if (DirExists(pathName))
            continue;

        if (!CreateDir(pathName))
            return false;

        // double check
        if (!DirExists(pathName))
            return false;
    }

    return true;

}

bool FileSystem::RemoveDir(const String& directoryIn, bool recursive)
{
    String directory = AddTrailingSlash(directoryIn);

    if (!DirExists(directory))
        return false;

    std::vector<String> results;

    // ensure empty if not recursive
    if (!recursive)
    {
        //std::erase(results)

        ScanDir(results, directory, "*", SCAN_DIRS | SCAN_FILES | SCAN_HIDDEN | SCAN_RECURSIVE);
        while (erase_first(results, ".") != results.end()) {}
        while (erase_first(results, "..") != results.end()) {}

        if (results.size())
            return false;

#ifdef WIN32
        return RemoveDirectoryW(GetWideNativePath(directory).c_str()) != 0;
#else
        return remove(GetNativePath(directory).c_str()) == 0;
#endif
    }

    // delete all files at this level
    ScanDir(results, directory, "*", SCAN_FILES | SCAN_HIDDEN);
    for (unsigned i = 0; i < results.size(); i++)
    {
        if (!Delete(directory + results[i]))
            return false;
    }
    results.clear();

    // recurse into subfolders
    ScanDir(results, directory, "*", SCAN_DIRS);
    for (unsigned i = 0; i < results.size(); i++)
    {
        if (results[i] == "." || results[i] == "..")
            continue;

        if (!RemoveDir(directory + results[i], true))
            return false;
    }

    return RemoveDir(directory, false);

}

bool FileSystem::CopyDir(const String& directoryIn, const String& directoryOut, std::vector<String>* copiedFiles)
{
    if (FileExists(directoryOut))
        return false;

    std::vector<String> results;
    ScanDir(results, directoryIn, "*", SCAN_FILES | SCAN_RECURSIVE);

    bool success = true;
    for (unsigned i = 0; i < results.size(); i++)
    {
        const String srcFile = AddTrailingSlash(directoryIn) + results[i];
        const String dstFile = AddTrailingSlash(directoryOut) + results[i];

        const String dstPath = GetPath(dstFile);

        if (!CreateDirsRecursive(dstPath))
        {
            success = false;
            continue;
        }

        if (!Copy(srcFile, dstFile))
        {
            success = false;
            continue;
        }

        if (copiedFiles)
            copiedFiles->push_back(dstFile);
    }

    return success;

}

bool IsAbsoluteParentPath(const String& absParentPath, const String& fullPath)
{
    if (!IsAbsolutePath(absParentPath) || !IsAbsolutePath(fullPath))
        return false;

    String path1 = AddTrailingSlash(GetSanitizedPath(absParentPath));
    String path2 = AddTrailingSlash(GetSanitizedPath(GetPath(fullPath)));

    if (path2.starts_with(path1))
        return true;

    return false;
}

String GetSanitizedPath(const String& path)
{
    String sanitized = GetInternalPath(path);
    std::vector<String> parts = sanitized.split('/');

    bool hasTrailingSlash = path.ends_with("/") || path.ends_with("\\");

#ifndef _WIN32

    bool absolute = IsAbsolutePath(path);
    sanitized = String::joined(parts, "/");
    if (absolute)
        sanitized = "/" + sanitized;

#else

    sanitized = String::joined(parts, "/");

#endif

    if (hasTrailingSlash)
        sanitized += "/";

    return sanitized;

}

// String GetSanitizedName(const String& name)
// {
//     static const std::u32string forbiddenSymbols = U"<>:\"/\\|?*";

//     // std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> conv;
//     // std::u32string utf32 = conv.from_bytes(name);
//     // std::u32string unicodeString{std::u32string::CtorConvert{}, name};
    
//     std::u32string s32(name.begin(), name.end());
//     for (char32_t ch = 0; ch < 31; ++ch)
//         unicodeString.replace(ch, ' ');
//     for (char32_t ch : forbiddenSymbols)
//         unicodeString.replace(ch, '_');
//     return { String::CtorConvert{}, unicodeString };
// }

bool GetRelativePath(const String& fromPath, const String& toPath, String& output)
{
    output = String::EMPTY;

    String from = GetSanitizedPath(fromPath);
    String to = GetSanitizedPath(toPath);

    std::vector<String> fromParts = from.split('/');
    std::vector<String> toParts = to.split('/');

    if (!fromParts.size() || !toParts.size())
        return false;

    if (fromParts == toParts)
    {
        return true;
    }

    // no common base?
    if (fromParts[0] != toParts[0])
        return false;

    int startIdx;

    for (startIdx = 0; startIdx < toParts.size(); startIdx++)
    {
        if (startIdx >= fromParts.size() || fromParts[startIdx] != toParts[startIdx])
            break;
    }

    if (startIdx == toParts.size())
    {
        if (from.ends_with("/") && to.ends_with("/"))
        {
            for (unsigned i = 0; i < fromParts.size() - startIdx; i++)
            {
                output += "../";
            }

            return true;
        }
        return false;
    }

    for (int i = 0; i < (int) fromParts.size() - startIdx; i++)
    {
        output += "../";
    }

    for (int i = startIdx; i < (int) toParts.size(); i++)
    {
        output += toParts[i] + "/";
    }

    return true;

}

String FileSystem::GetTemporaryDir() const
{
#if defined(_WIN32)
    wchar_t pathName[MAX_PATH];
    pathName[0] = 0;

    TCHAR tempPath[MAX_PATH];
    DWORD tempPathLen = GetTempPath(MAX_PATH, tempPath);
    if (tempPathLen > 0)
    {
        return AddTrailingSlash(WideToMultiByte((wchar_t*)tempPath));
    }

    assert(0 && "FileSystem::GetTemporaryDir() error");

    //GetTempPathW(SDL_arraysize(pathName), pathName);
    
#else
    if (char* pathName = getenv("TMPDIR"))
        return AddTrailingSlash(pathName);
#  ifdef P_tmpdir
    return AddTrailingSlash(P_tmpdir);
#  else
    return "/tmp/";
#  endif
#endif
    return "";
}

String FileSystem::FindResourcePrefixPath() const
{
    const auto isFileSystemRoot = [](const String& path)
    {
#if _WIN32
        return path.length() <= 3;  // Root path of any drive
#else
        return path == "/";         // Filesystem root
#endif
    };

    for (String result : {GetCurrentDir(), GetProgramDir()})
    {
        while (!isFileSystemRoot(result))
        {
            if (DirExists(result + "CoreData"))
                return result;

            result = GetParentPath(result);
        }
    }

    return String::EMPTY;
}

String FileSystem::SimplifyPath(const String& path) {
    auto split = path.split('/');

    if (split.size() < 2)
            return path;

    for (auto it = split.begin()+1; it != split.end(); ++it) {
        auto cur = it;
        auto prev = (it-1);
        //printf("%s - %s\n", (*cur).CString(), (*prev).CString());
        if((*cur == "..") && (*prev != "..")) {
            split.erase(cur);
            split.erase(prev);
            it = it-2;
        }
    }

    String ret = path.starts_with("/") ? "/" : "";
    ret += String::joined(split, "/") + "/";
    return ret;
}

FileSystem& FileSystem::Get()
{
    static FileSystem* ptr_;
    if (!ptr_) {
        SE_LOG_INFO("FileSystem initialized.");
        ptr_ = new FileSystem();
    }
    return *ptr_;
}

String ResolvePath(const String& filePath)
{
    String sanitizedName;
    String::size_type segmentStartIndex{0};
    sanitizedName.reserve(filePath.length());

    for (auto c : filePath)
    {
        if (c == '\\' || c == '/')
        {
            if (ResolvePathSegment(sanitizedName, segmentStartIndex))
            {
                sanitizedName.push_back('/');
                segmentStartIndex = sanitizedName.size();
            }
        }
        else
        {
            sanitizedName.push_back(c);
        }
    }
    ResolvePathSegment(sanitizedName, segmentStartIndex);
    // Remove trailing / if it isn't the only one
    if (sanitizedName.size() > 1)
    {
        const auto lastCharIndex = sanitizedName.size() - 1;
        if (sanitizedName.at(lastCharIndex) == '/')
        {
            sanitizedName.resize(lastCharIndex);
        }
    }
    sanitizedName = sanitizedName.trimmed();
    return sanitizedName;
}

String GetAbsolutePath(const String& path, const String& currentPath, bool addTrailingSlash)
{
    String absolutePath = IsAbsolutePath(path) ? path : (currentPath + path);
    return addTrailingSlash ? AddTrailingSlash(absolutePath) : absolutePath;
}

std::vector<String> GetAbsolutePaths(const std::vector<String>& paths, const String& currentPath, bool addTrailingSlash)
{
    std::vector<String> result;
    for (const String& path : paths)
        result.push_back(GetAbsolutePath(path, currentPath, addTrailingSlash));
    return result;
}

String GetExtensionFromFilter(const String& filter)
{
    const std::size_t dotPos = filter.find_last('.');
    if (dotPos == String::npos)
        return String::EMPTY;

    String filterExtension = filter.substr(dotPos);
    if (filterExtension.contains('*'))
        return String::EMPTY;

    return filterExtension;
}

bool MatchFileName(
    const String& fileName, const String& path, const String& extension, bool recursive, bool caseSensitive)
{
    if (!StartsWith(fileName, path, caseSensitive))
        return false;

    if (fileName.length() > path.length() && fileName[path.length()] != '/')
        return false;

    if (!extension.empty() && !EndsWith(fileName, extension, caseSensitive))
        return false;

    if (!recursive)
    {
        String relativeFileName = fileName.substr(path.length());
        if (relativeFileName.starts_with('/'))
            relativeFileName = relativeFileName.substr(1);

        if (relativeFileName.find('/') != std::string::npos)
            return false;
    }

    return true;
}

String TrimPathPrefix(const String& fileName, const String& prefixPath)
{
    if (prefixPath.length() >= fileName.length())
        return String::EMPTY;

    String result = fileName.substr(prefixPath.length());
    if (result.starts_with('/'))
        result = result.substr(1);

    return result;
}

String FindProgramPath(const String& name) {
    auto fs = FileSystem::Get();
#ifdef WIN32
    String findCmd = "where";
#else
    String findCmd = "which";
#endif
    String output;
    bool isOk = fs.SystemRun(findCmd, {name}, output) == 0;
    output.replace("\n", "");
    return isOk ? output : String::EMPTY;
}

void TreeNodeAddPath(DirectoryNode* parent, const Se::String& path)
{
    std::vector<Se::String> components = path.split('/');

    DirectoryNode *current = parent;
    parent->Flags.Set(FSIF_DIRECTORY);

    // if (components.size() > 1)
    //     parent->Flags |= FSIF_DIRECTORY;

    for (std::size_t i = 0; i < components.size(); ++i)
    {
        std::string name = components[i];

        auto it = std::find_if(current->Children.begin(), current->Children.end(), 
            [name](const DirectoryNode& node) {
                return     (name.size() == node.FileName.size())
                        && (name == node.FileName);
        });

        if (it != current->Children.end()) {
            current = &(*it);
            continue;
        }
         
        DirectoryNode& child = current->Children.emplace_back();
        child.FileName = name;
        child.parent = current;
        if (parent->Flags.Test(FSIF_ARCHIVE) || parent->Flags.Test(FSIF_READONLY) )
        child.Flags = FSIF_READONLY; //IsArchived = true;

        if (i < components.size()-1)
            child.Flags |= FSIF_DIRECTORY;
//            child.IsDirectory = true;

        if (current != parent)
            child.FullPath = child.parent->FullPath + "/";
        child.FullPath += name;     

        current = &current->Children.back();
    }
}

void TreeNodeScanTree(DirectoryNode& result, const String& pathName, const String& filter, ScanFlags flags)
{
    result.Children.clear();

    std::vector<String> filesList;

    for (auto entry : filesList) {
        TreeNodeAddPath(&result, entry);
    }
}

template <typename ForwardIterator, typename T>
ForwardIterator FindLast(ForwardIterator first, ForwardIterator last,  T pred)
{
    ForwardIterator result = last; // Initialize result to last (not found)

    for (ForwardIterator it = first; it != last; ++it) {
        if (pred(*it)) {
            result = it; // Update result to the current iterator
        }
    }

    return result; // Return the last found iterator or last if not found
}

void SortTreeByName(Se::DirectoryNode& node)
{
    if (node.Children.size() < 2)
        return;

    auto sortByFileName = [](const Se::DirectoryNode& a, const Se::DirectoryNode& b) {
        auto lf = a.FileName; lf.to_lower();
        auto rf = b.FileName; rf.to_lower();
        return (lf < rf);
    };

    // sort by type
    std::sort(node.Children.begin(), node.Children.end(), [](const Se::DirectoryNode& lnode, const Se::DirectoryNode& rnode){
        return lnode.Flags.Test(FSIF_DIRECTORY) > rnode.Flags.Test(FSIF_DIRECTORY);
    });

    // sort folders
    auto it = FindLast(node.Children.begin(), node.Children.end(), [](const Se::DirectoryNode& n) {
        return n.Flags.Test(FSIF_DIRECTORY);
    });
    std::sort(node.Children.begin(), it, sortByFileName);

    // sort files
    it = std::find_if(it, node.Children.end(), [](const Se::DirectoryNode& n) {
        return !n.Flags.Test(FSIF_DIRECTORY);
    });
    std::sort(it, node.Children.end(), sortByFileName);

    for (auto& node : node.Children)
        if (node.Flags.Test(FSIF_DIRECTORY))
            SortTreeByName(node);

}

}
