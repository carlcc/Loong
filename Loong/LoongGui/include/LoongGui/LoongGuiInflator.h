//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#pragma once

#include <memory>
#include <string>

namespace Loong::Gui {

class LoongGuiWidget;

class LoongGuiInflator {
public:
    std::shared_ptr<LoongGuiWidget> Inflate(const std::string& vfsPath);

    template <class T>
    std::shared_ptr<T> InflateAs(const std::string& vfsPath)
    {
        auto sp = Inflate(vfsPath);
        return std::static_pointer_cast<T>(sp);
    }
};

}