//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#pragma once

#include <cstdlib>
#include <iostream>

#ifdef NDEBUG

#define LOONG_ASSERT(condition, msg)
#define LOONG_VERIFY(condition, msg)

#else
#define LOONG_ASSERT(condition, msg)                                   \
    do {                                                               \
        if (condition)                                                 \
            break;                                                     \
        std::cerr << "Assert failure '" #condition "' at " << __FILE__ \
                  << ":" << __LINE__ << ": " << msg << std::endl;      \
        abort();                                                       \
    } while (false)

#define LOONG_VERIFY(condition, msg)                                   \
    do {                                                               \
        if (condition)                                                 \
            break;                                                     \
        std::cerr << "Verify failure '" #condition "' at " << __FILE__ \
                  << ":" << __LINE__ << ": " << msg << std::endl;      \
    } while (false)

#endif