//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#include "LoongFileSystem/LoongFileSystem.h"
#include <physfs.h>

namespace Loong {

bool LoongFileSystem::Initialize(const std::string& argv0)
{
    return 0 != PHYSFS_init(argv0.empty() ? nullptr : argv0.c_str());
}

bool LoongFileSystem::Uninitialize()
{
    return 0 != PHYSFS_deinit();
}

bool LoongFileSystem::MountSearchPath(const std::string& sysPath, const std::string& mountPoint, bool isAppend)
{
    return 0 != PHYSFS_mount(sysPath.c_str(), mountPoint.c_str(), int(isAppend));
}

bool LoongFileSystem::UnmountSearchPath(const std::string& sysPath)
{
    return 0 != PHYSFS_unmount(sysPath.c_str());
}

std::vector<std::string> LoongFileSystem::GetSearchPaths()
{
    std::vector<std::string> result;

    auto** list = PHYSFS_getSearchPath();
    if (list == nullptr) {
        return result;
    }
    for (char** path = list; *path != nullptr; path++) {
        result.push_back(*path);
    }
    PHYSFS_freeList(list);
    return result;
}

bool LoongFileSystem::MakeDir(const std::string& path)
{
    return 0 != PHYSFS_mkdir(path.c_str());
}

bool LoongFileSystem::Delete(const std::string& file)
{
    return 0 != PHYSFS_delete(file.c_str());
}

bool LoongFileSystem::Exists(const std::string& file)
{
    return 0 != PHYSFS_exists(file.c_str());
}

bool LoongFileSystem::IsDir(const std::string& file)
{
    return 0 != PHYSFS_isDirectory(file.c_str());
}

bool LoongFileSystem::IsSymbolLink(const std::string& file)
{
    return 0 != PHYSFS_isSymbolicLink(file.c_str());
}

std::optional<std::string> LoongFileSystem::GetMountPoint(const std::string& realDir)
{
    auto* path = PHYSFS_getMountPoint(realDir.c_str());
    if (path == nullptr) {
        return {};
    }
    return std::make_optional<std::string>(path);
}

std::optional<std::string> LoongFileSystem::GetRealDir(const std::string& file)
{
    auto* realDir = PHYSFS_getRealDir(file.c_str());
    if (realDir == nullptr) {
        return {};
    }
    return std::make_optional<std::string>(realDir);
}

std::optional<std::vector<std::string>> LoongFileSystem::ListFiles(const std::string& dir)
{
    auto** files = PHYSFS_enumerateFiles(dir.c_str());
    if (files == nullptr) {
        return {};
    }

    std::vector<std::string> result;
    for (char** f = files; *f != nullptr; f++) {
        result.push_back(*f);
    }
    PHYSFS_freeList(files);
    return result;
}

bool LoongFileSystem::EnumerateFiles(const std::string& dir, const EnumerateCallback& cb)
{
    struct CB {
        const EnumerateCallback& cb;
    };
    CB localCb { cb };
    return 0 != PHYSFS_enumerate(
               dir.c_str(), [](void* data, const char* dir, const char* name) -> PHYSFS_EnumerateCallbackResult {
                   (void)dir;
                   CB& cb = *reinterpret_cast<CB*>(data);
                   return cb.cb(name) ? PHYSFS_EnumerateCallbackResult::PHYSFS_ENUM_STOP : PHYSFS_EnumerateCallbackResult::PHYSFS_ENUM_OK;
               },
               &localCb);
}

bool LoongFileSystem::GetFileStat(const std::string& name, FileStat& stat)
{
    PHYSFS_Stat physfsStat;
    if (PHYSFS_stat(name.c_str(), &physfsStat) == 0) {
        return false;
    }
    stat.filesize = physfsStat.filesize;
    stat.modtime = physfsStat.modtime;
    stat.createtime = physfsStat.createtime;
    stat.accesstime = physfsStat.accesstime;
    stat.filetype = FileType(physfsStat.filetype);
    stat.readonly = (bool)physfsStat.readonly;
    return true;
}

int64_t LoongFileSystem::LoadFileContent(const std::string& path, void* bufferVoid, uint64_t bufferSize)
{
    auto* file = PHYSFS_openRead(path.c_str());
    if (file == nullptr) {
        return -1;
    }
    auto* buffer = static_cast<uint8_t*>(bufferVoid);

    int64_t totalCount = 0;
    while (bufferSize > 0) {
        auto cnt = PHYSFS_readBytes(file, buffer, bufferSize);
        if (cnt == -1) {
            return -1;
        }
        totalCount += cnt;
        buffer += cnt;
        bufferSize -= cnt;

        if (0 != PHYSFS_eof(file)) {
            break;
        }
    }
    return totalCount;
}

LoongFileSystem::ErrorCode LoongFileSystem::GetLastErrorCode()
{
    return ErrorCode(PHYSFS_getLastErrorCode());
}

const char* LoongFileSystem::GetLastError()
{
    return GetErrorText(GetLastErrorCode());
}

const char* LoongFileSystem::GetErrorText(ErrorCode code)
{
    return PHYSFS_getErrorByCode(PHYSFS_ErrorCode(code));
}

}