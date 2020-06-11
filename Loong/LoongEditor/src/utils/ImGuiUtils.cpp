//
// Copyright (c) carlcc. All rights reserved.
//

#include "ImGuiUtils.h"
#include "../LoongEditor.h"
#include "../LoongEditorConstants.h"
#include "../panels/LoongEditorMaterialEditorPanel.h"
#include "LoongFileTreeNode.h"
#include "LoongFoundation/LoongStringUtils.h"

namespace Loong::Editor::ImGuiUtils {

void HandleOpenFile(const LoongFileTreeNode* fileNode, LoongEditor* editor)
{
    if (fileNode == nullptr) {
        return;
    }
    if (Foundation::LoongStringUtils::EndsWith(fileNode->fileName, Constants::kMaterialFileSuffix)) {
        // TODO: Use signal
        auto* materialPanel = editor->GetPanel<LoongEditorMaterialEditorPanel>();
        materialPanel->OpenMaterial(fileNode);
        return;
    }
}

}