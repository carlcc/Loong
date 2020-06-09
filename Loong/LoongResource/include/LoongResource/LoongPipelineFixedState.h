//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#pragma once

#include <cstdint>

namespace Loong::Resource {

class LoongPipelineFixedState {
    enum StateFlag : uint32_t {
        kDepthWrite = (1u << 0u),
        kColorWrite = (1u << 1u),
        kEnableBlend = (1u << 2u),
        kEnableCullFace = (1u << 3u),
        kEnableDepthTest = (1u << 4u),
        kCullBack = (1u << 5u),
        kCullFront = (1u << 6u),
        kCullFrontAndBack = (1u << 7u),
    };

public:
    bool IsDepthWriteEnabled() const { return state_ & kDepthWrite; }
    bool IsColorWriteEnabled() const { return state_ & kColorWrite; }
    bool IsBlendEnabled() const { return state_ & kEnableBlend; }
    bool IsFaceCullEnabled() const { return state_ & kEnableCullFace; }
    bool IsDepthTestEnabled() const { return state_ & kEnableDepthTest; }
    bool IsBackCullEnabled() const { return state_ & kCullBack; }
    bool IsFrontCullEnabled() const { return state_ & kCullFront; }
    bool IsFrontAndBackCullEnabled() const { return state_ & kCullFrontAndBack; }

    void SetDepthWriteEnabled(bool b)
    {
        if (b) {
            state_ |= kDepthWrite;
        } else {
            state_ &= ~kDepthWrite;
        }
    }
    void SetColorWriteEnabled(bool b)
    {
        if (b) {
            state_ |= kColorWrite;
        } else {
            state_ &= ~kColorWrite;
        }
    }
    void SetBlendEnabled(bool b)
    {
        if (b) {
            state_ |= kEnableBlend;
        } else {
            state_ &= ~kEnableBlend;
        }
    }
    void SetFaceCullEnabled(bool b)
    {
        if (b) {
            state_ |= kEnableCullFace;
        } else {
            state_ &= ~kEnableCullFace;
        }
    }
    void SetDepthTestEnabled(bool b)
    {
        if (b) {
            state_ |= kEnableDepthTest;
        } else {
            state_ &= ~kEnableDepthTest;
        }
    }
    void SetBackCullEnabled(bool b)
    {
        if (b) {
            state_ |= kCullBack;
        } else {
            state_ &= ~kCullBack;
        }
    }
    void SetFrontCullEnabled(bool b)
    {
        if (b) {
            state_ |= kCullFront;
        } else {
            state_ &= ~kCullFront;
        }
    }
    void SetFrontAndBackCullEnabled(bool b)
    {
        if (b) {
            state_ |= kCullFrontAndBack;
        } else {
            state_ &= ~kCullFrontAndBack;
        }
    }

private:
    uint32_t state_ { 0 };

    friend LoongPipelineFixedState operator^(const LoongPipelineFixedState& a, const LoongPipelineFixedState& b);
};

inline LoongPipelineFixedState operator^(const LoongPipelineFixedState& a, const LoongPipelineFixedState& b)
{
    LoongPipelineFixedState result;
    result.state_ = a.state_ ^ b.state_;
    return result;
}

}