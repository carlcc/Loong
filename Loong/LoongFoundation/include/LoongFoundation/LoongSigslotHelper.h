//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#pragma once

#include "LoongFoundation/LoongMacros.h"
#include "LoongFoundation/sigslot.h"
#include <functional>
#include <memory>

namespace Loong::Foundation {

using LoongHasSlots = sigslot::has_slots<>;

template <class... Args>
class LoongSignalListener : public LoongHasSlots {
public:
    template <class Callable>
    explicit LoongSignalListener(Callable&& callable)
        : cb_(std::forward<Callable>(callable))
    {
    }
    LoongSignalListener(const LoongSignalListener& callable) = delete;
    LoongSignalListener(LoongSignalListener&& callable) = delete;
    LoongSignalListener& operator=(const LoongSignalListener& callable) = delete;
    LoongSignalListener& operator=(LoongSignalListener&& callable) = delete;
    ~LoongSignalListener() override = default;

    void Invoke(Args... args)
    {
        cb_(std::forward<Args>(args)...);
    }

private:
    std::function<void(Args...)> cb_ { [](Args...) {} };
};

template <class Ret, class Cls, class is_mutable, class... Args>
struct CallableDetail {
    using IsMutable = is_mutable;

    enum { NArgs = sizeof...(Args) };

    using ReturnType = Ret;

    template <size_t i>
    struct Arg {
        typedef typename std::tuple_element<i, std::tuple<Args...>>::type type;
    };

    using Listener = LoongSignalListener<Args...>;
};

template <class Ld>
struct CallableToSignalListener
    : CallableToSignalListener<decltype(&Ld::operator())> {
};

template <class Ret, class Cls, class... Args>
struct CallableToSignalListener<Ret (Cls::*)(Args...)>
    : CallableDetail<Ret, Cls, std::true_type, Args...> {
};

template <class Ret, class Cls, class... Args>
struct CallableToSignalListener<Ret (Cls::*)(Args...) const>
    : CallableDetail<Ret, Cls, std::false_type, Args...> {
};

}

#define LOONG_DECLARE_SIGNAL(sigName, ...)                                                           \
protected:                                                                                           \
    sigslot::signal<__VA_ARGS__> sigName##Signal_;                                                   \
                                                                                                     \
public:                                                                                              \
    template <class ReceiverType, class... Args>                                                     \
    void Subscribe##sigName(ReceiverType* pclass, void (ReceiverType::*pmemfun)(Args...))            \
    {                                                                                                \
        sigName##Signal_.connect(pclass, pmemfun);                                                   \
    }                                                                                                \
    template <class Callable>                                                                        \
    LG_NODISCARD auto Subscribe##sigName(Callable&& callable)                                        \
    {                                                                                                \
        using Listener = typename ::Loong::Foundation::CallableToSignalListener<Callable>::Listener; \
        auto listener = std::make_unique<Listener>(std::forward<Callable>(callable));                \
        sigName##Signal_.connect(listener.get(), &Listener::Invoke);                                 \
        return listener;                                                                             \
    }                                                                                                \
    template <class ReceiverType, class... Args>                                                     \
    void Subscribe##sigName(ReceiverType& pclass, void (ReceiverType::*pmemfun)(Args...))            \
    {                                                                                                \
        sigName##Signal_.connect(&pclass, pmemfun);                                                  \
    }                                                                                                \
    void Unsubscribe##sigName(sigslot::has_slots_interface* pclass)                                  \
    {                                                                                                \
        sigName##Signal_.disconnect(pclass);                                                         \
    }                                                                                                \
    void Unsubscribe##sigName() { sigName##Signal_.disconnect_all(); }
