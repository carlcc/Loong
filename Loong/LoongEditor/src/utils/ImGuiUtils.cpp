//
// Copyright (c) carlcc. All rights reserved.
//

#include "ImGuiUtils.h"
#include "../LoongEditor.h"
#include "../LoongEditorConstants.h"
#include "../panels/LoongEditorMaterialEditorPanel.h"
#include "LoongFileTreeNode.h"
#include "LoongFoundation/LoongPathUtils.h"
#include "LoongFoundation/LoongStringUtils.h"

namespace Loong::Editor::ImGuiUtils {

void HandleOpenFile(const LoongFileTreeNode* fileNode, LoongEditor* editor)
{
    if (fileNode == nullptr) {
        return;
    }
    auto suffix = Foundation::LoongPathUtils::GetFileExtension(fileNode->fileName);
    if (Constants::kMaterialFileSuffixes.count(suffix) > 0) {
        // TODO: Use signal
        auto* materialPanel = editor->GetPanel<LoongEditorMaterialEditorPanel>();
        materialPanel->OpenMaterial(fileNode);
        return;
    }
}

}