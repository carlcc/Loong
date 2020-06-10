//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#pragma once

namespace Loong::Core {
class LoongActor;
class LoongScene;
}

namespace Loong::Editor {
class LoongEditor;
}

namespace Loong::Editor::LoongEditorTemplates {

void ShowActorContextMenu(Core::LoongActor* actor, Core::LoongScene* scene, LoongEditor* editor);

void FillActorMenu(Core::LoongActor* actor, Core::LoongScene* scene, LoongEditor* editor);

}