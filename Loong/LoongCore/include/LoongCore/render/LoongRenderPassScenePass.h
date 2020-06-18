//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#pragma once

#include "LoongCore/render/LoongRenderPass.h"
#include "LoongFoundation/LoongMath.h"

namespace Loong::Resource {
class LoongGpuModel;
};

namespace Loong::Core {

class LoongRenderPassScenePass : public LoongRenderPass {
public:
    void Render(const Context& context) override;

    void SetDefaultMaterial(const std::shared_ptr<Resource::LoongMaterial>& mat) { defaultMaterial_ = mat; }

    void SetCameraMaterial(const std::shared_ptr<Resource::LoongMaterial>& mat) { cameraMaterial_ = mat; }

    void SetCameraModel(const std::shared_ptr<Resource::LoongGpuModel>& mdl) { cameraModel_ = mdl; }

    void SetRenderCamera(bool b) { shouldRenderCamera_ = b; }

protected:
    std::shared_ptr<Resource::LoongMaterial> defaultMaterial_ { nullptr };
    std::shared_ptr<Resource::LoongMaterial> cameraMaterial_ { nullptr };
    std::shared_ptr<Resource::LoongGpuModel> cameraModel_ { nullptr };
    bool shouldRenderCamera_ { false };
};

}