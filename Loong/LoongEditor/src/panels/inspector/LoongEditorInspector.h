//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#pragma once

namespace Loong::Core {
class LoongCCamera;
class LoongCModelRenderer;
class LoongCLight;
class LoongCSky;
}
namespace Loong::Foundation {
class Transform;
}

namespace Loong::Editor {

class LoongEditorInspector {
public:
    static void Inspect(Core::LoongCCamera* camera);
    static void Inspect(Core::LoongCLight* light);
    static void Inspect(Core::LoongCModelRenderer* model);
    static void Inspect(Core::LoongCSky* sky);
    static void Inspect(Foundation::Transform& transform);
};

}