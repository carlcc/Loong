//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#pragma once

#include "LoongEditorRenderPanel.h"

namespace Loong::Core {
class LoongCModelRenderer;
}
namespace Loong::Resource {
class LoongMaterial;
class LoongGpuModel;
}

namespace Loong::Editor {

class LoongFileTreeNode;

class LoongEditorMaterialEditorPanel : public LoongEditorRenderPanel {
public:
    LoongEditorMaterialEditorPanel(LoongEditor* editor, const std::string& name, bool opened, const LoongEditorPanelConfig& cfg);

    void Render(const Foundation::LoongClock& clock) override;

    void OpenMaterial(const LoongFileTreeNode* fileNode);

protected:
    void UpdateImpl(const Foundation::LoongClock& clock) override;

private:
    void UpdateProperies(const Foundation::LoongClock& clock);

    void AdjustPreviewModelAndCamera();

    void OnPreviewActorModelChanged(Resource::LoongGpuModel* newModel, Resource::LoongGpuModel* oldModel);

private:
    std::shared_ptr<Resource::LoongMaterial> materialBackUp_ { nullptr };
    std::shared_ptr<Resource::LoongMaterial> currentMaterial_ { nullptr };
    std::shared_ptr<Core::LoongScene> previewScene_ { nullptr };
    Core::LoongActor* previewActor_ { nullptr };
    Core::LoongCModelRenderer* previewModel_ { nullptr };
    std::string materialFileFullPath_ {};
};

}