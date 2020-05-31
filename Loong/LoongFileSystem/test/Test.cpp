//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#include "LoongFileSystem/LoongFileSystem.h"
#include <cassert>
#include <iostream>

using namespace Loong;

#ifdef WIN32
const char kSeparator = '\\';
#else
const char kSeparator = '/';
#endif

const std::string GetDir(const std::string& path)
{
    auto index = path.rfind(kSeparator);
    assert(index != std::string::npos);

    return path.substr(0, index);
}

int main(int argc, const char* argv[])
{
    LoongFileSystem::Initialize(argv[0]);

    auto path = GetDir(__FILE__);
    LoongFileSystem::MountSearchPath(path);

    std::vector<std::string> kRightAnswer = {
        "CMakeLists.txt", "Test.cpp"
    };
    auto files = LoongFileSystem::ListFiles("/");
    assert(files.has_value());
    int index = 0;

    auto& answer = files.value();
    assert(answer == kRightAnswer);

    LoongFileSystem::EnumerateFiles("/", [&kRightAnswer](const std::string& name) -> bool {
        assert(name == kRightAnswer[0]);
        std::cout << name << std::endl;
        return true;
    });

    LoongFileSystem::Uninitialize();
}