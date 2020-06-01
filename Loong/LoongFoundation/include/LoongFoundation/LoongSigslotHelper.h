//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#pragma once

#include "LoongFoundation/sigslot.h"

namespace Loong::Foundation {

using LoongHasSlots = sigslot::has_slots<>;

}

#define LOONG_DECLARE_SIGNAL(sigName, ...)                                                \
protected:                                                                                \
    sigslot::signal<__VA_ARGS__> sigName##Signal_;                                        \
                                                                                          \
public:                                                                                   \
    template <class ReceiverType, class... Args>                                          \
    void Subscribe##sigName(ReceiverType* pclass, void (ReceiverType::*pmemfun)(Args...)) \
    {                                                                                     \
        sigName##Signal_.connect(pclass, pmemfun);                                        \
    }                                                                                     \
    template <class ReceiverType, class... Args>                                          \
    void Subscribe##sigName(ReceiverType& pclass, void (ReceiverType::*pmemfun)(Args...)) \
    {                                                                                     \
        sigName##Signal_.connect(&pclass, pmemfun);                                       \
    }                                                                                     \
    void Unsubscribe##sigName(sigslot::has_slots_interface* pclass)                       \
    {                                                                                     \
        sigName##Signal_.disconnect(pclass);                                              \
    }                                                                                     \
    void Unsubscribe##sigName() { sigName##Signal_.disconnect_all(); }
