//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#pragma once

#include <functional>
#include <optional>
#include <string>
#include <vector>

namespace Loong::FS {

class LoongFileSystem {
public:
    static bool Initialize(const std::string& argv0);

    static bool Uninitialize();

    static bool MountSearchPath(const std::string& sysPath, const std::string& mountPoint = "/", bool isAppend = true);

    static bool UnmountSearchPath(const std::string& sysPath);

    static std::vector<std::string> GetSearchPaths();

    static bool MakeDir(const std::string& path);

    // If file is a directory, then it must be empty
    static bool Delete(const std::string& file);

    static bool Exists(const std::string& file);

    static bool IsDir(const std::string& file);

    static bool IsSymbolLink(const std::string& file);

    static std::optional<std::string> GetMountPoint(const std::string& realDir);

    // if you look for "maps/level1.map", and C:\\mygame is in your search
    // path and C:\\mygame\\maps\\level1.map exists, then "C:\mygame" is returned.
    static std::optional<std::string> GetRealDir(const std::string& file);

    static std::optional<std::vector<std::string>> ListFiles(const std::string& dir);

    // Return true to stop  enumerating, the parameters: FileName
    using EnumerateCallback = std::function<bool(std::string)>;
    static bool EnumerateFiles(const std::string& dir, const EnumerateCallback& cb);

    enum class FileType {
        kRegular,
        kDirectory,
        kSymbolLink,
        kUnkown,
    };
    struct FileStat {
        int64_t filesize; // size in bytes, -1 for non-files and unknown
        int64_t modtime; // last modification time
        int64_t createtime; // like modtime, but for file creation time
        int64_t accesstime; // like modtime, but for file access time
        FileType filetype;
        bool readonly; // Readonly or writable?
    };
    static bool GetFileStat(const std::string& name, FileStat& stat);

    static int64_t GetFileSize(const std::string& name)
    {
        FileStat stat {};
        if (!GetFileStat(name, stat)) {
            return -1;
        }
        return stat.filesize;
    }

    static int64_t LoadFileContent(const std::string& path, void* buffer, uint64_t bufferSize);

    enum class ErrorCode {
        kOk,
        kOtherError,
        kOutOfMemory,
        kNotInitialized,
        kIsInitialized,
        kArgv0IsNull,
        kUnsupported,
        kPastEof,
        kFilesStillOpen,
        kInvalidArgument,
        kNotMounted,
        kNotFound,
        kSymlinkForbidden,
        kNoWriteDir,
        kOpenForReading,
        kOpenForWriting,
        kNotAFile,
        kReadOnly,
        kCorrupt,
        kSymlinkLoop,
        kIo,
        kPermission,
        kNoSpace,
        kBadFilename,
        kBusy,
        kDirNotEmpty,
        kOsError,
        kDuplicate,
        kBadPassword,
        kAppCallback,
    };

    static ErrorCode GetLastErrorCode();

    static const char* GetLastError();

    static const char* GetErrorText(ErrorCode code);
};

}