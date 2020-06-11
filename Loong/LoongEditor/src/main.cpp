#include "glad/glad.h"

#include "LoongApp/Driver.h"
#include "LoongFileSystem/Driver.h"
#include "LoongResource/Driver.h"

#include "LoongApp/LoongApp.h"
#include "LoongEditor.h"
#include "LoongEditorContext.h"
#include "LoongEditorProjectManager.h"
#include "LoongFoundation/LoongLogger.h"
#include "LoongFoundation/LoongPathUtils.h"
#include "LoongRenderer/LoongRenderer.h"
#include <imgui.h>
#include <iostream>
#include <memory>

int StartApp(int argc, char** argv)
{

    Loong::App::LoongApp::WindowConfig config {};
    config.title = "LoongEditor";
    std::shared_ptr<Loong::App::LoongApp> app = std::make_shared<Loong::App::LoongApp>(config);

    // load font for ImGui
    const char* kFontPath = "/Fonts/wqymicroheimono.ttf";
    auto size = Loong::FS::LoongFileSystem::GetFileSize(kFontPath);
    if (size < 0) {
        LOONG_ERROR("Cannot load font file '{}', exit...", kFontPath);
        return 1;
    }

    std::vector<uint8_t> buffer(size);
    if (!Loong::FS::LoongFileSystem::LoadFileContent(kFontPath, buffer.data(), size)) {
        LOONG_ERROR("Cannot load font file '{}', exit...", kFontPath);
        return 1;
    }
    ImGui::GetIO().Fonts->AddFontFromMemoryTTF(buffer.data(), int(size), 16);

    std::shared_ptr<Loong::Editor::LoongEditorContext> context;
    {
        Loong::Editor::LoongEditorProjectManager projectManager(app.get());
        if (!projectManager.Initialize()) {
            LOONG_ERROR("Initialized project manager failed!");
            return 2;
        }
        app->Run();

        auto projectFile = projectManager.GetSelectedPath();
        if (projectFile.empty()) {
            LOONG_INFO("No project file selected, close!");
            return 0; // The user closed project manager without select any project
        }

        context = std::make_shared<Loong::Editor::LoongEditorContext>(projectFile);
        // Set the project directory as write dir, and mount it to root
        Loong::FS::LoongFileSystem::SetWriteDir(context->GetProjectDirPath());
        Loong::FS::LoongFileSystem::MountSearchPath(context->GetProjectDirPath());
    }

    Loong::Editor::LoongEditor editor(app.get(), context);
    if (!editor.Initialize()) {
        LOONG_ERROR("Initialized editor failed!");
        return 2;
    }

    return app->Run();
}

int main(int argc, char** argv)
{
    Loong::App::ScopedDriver appDriver;

    auto path = Loong::Foundation::LoongPathUtils::GetParent(argv[0]) + "/Resources";
    Loong::FS::ScopedDriver fsDriver(argv[0]);
    Loong::FS::LoongFileSystem::MountSearchPath(path);
    path = Loong::Foundation::LoongPathUtils::Normalize(argv[0]) + "/../../Resources";
    Loong::FS::LoongFileSystem::MountSearchPath(path);

    Loong::Resource::ScopedDriver resourceDriver;

    auto listener = Loong::Foundation::Logger::Get().SubscribeLog([](const Loong::Foundation::LogItem& logItem) {
        std::cout << "[" << logItem.level << "][" << logItem.location << "]: " << logItem.message << std::endl;
    });
    StartApp(argc, argv);

    return 0;
}