//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#pragma once

#include "LoongCore/scene/LoongComponent.h"
#include "LoongFoundation/LoongSigslotHelper.h"
#include "LoongResource/LoongGpuModel.h"
#include <memory>
#include <vector>

namespace Loong::Resource {
class LoongMaterial;
};

namespace Loong::Core {

class LoongCModelRenderer : public LoongComponent {
    using MaterialRef = std::shared_ptr<Resource::LoongMaterial>;

public:
    enum class CullMode {
        kDisabled = 0,
        kCullModel = 1,
        kCullMesh = 2,
        kCullCustom = 3,
    };

    explicit LoongCModelRenderer(LoongActor* owner)
        : LoongComponent(owner)
    {
    }

    std::string GetName() override { return "Model Component"; };

    void SetModel(std::shared_ptr<Resource::LoongGpuModel> model)
    {
        if (model_ != model) {
            auto oldModel = model_;
            model_ = std::move(model);
            if (model_ != nullptr) {
                materials_.resize(model_->GetMaterialNames().size());
            } else {
                materials_.clear();
            }
            ModelChangedSignal_.emit(model_.get(), oldModel.get());
        }
    }

    std::shared_ptr<Resource::LoongGpuModel> GetModel() const { return model_; }

    const std::vector<MaterialRef>& GetMaterials() const
    {
        return materials_;
    }

    void SetMaterial(int index, const MaterialRef& material)
    {
        materials_[index] = material;
    }

    void SetCullMode(CullMode mode) { cullMode_ = mode; }

    CullMode GetCullMode() const { return cullMode_; }

    LOONG_DECLARE_SIGNAL(ModelChanged, Resource::LoongGpuModel*, Resource::LoongGpuModel*); // new model, old model

private:
    std::shared_ptr<Resource::LoongGpuModel> model_ { nullptr };
    std::vector<MaterialRef> materials_ {};
    CullMode cullMode_ { CullMode::kCullModel };
};

}