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
#include "LoongResource/LoongTexture.h"
#include "LoongResource/loader/LoongTextureLoader.h"
#include "utils/IconsFontAwesome5.h"
#include <imgui.h>
#include <iostream>
#include <memory>

int LoadFonts()
{
    // load font for ImGui
    const char* kTextFontPath = "/Fonts/wqymicroheimono.ttf";
    auto size1 = Loong::FS::LoongFileSystem::GetFileSize(kTextFontPath);
    if (size1 < 0) {
        LOONG_ERROR("Cannot load font file '{}', exit...", kTextFontPath);
        return 1;
    }
    const char* kIconFontPath = "/Fonts/" FONT_ICON_FILE_NAME_FAS;
    auto size2 = Loong::FS::LoongFileSystem::GetFileSize(kIconFontPath);
    if (size2 < 0) {
        LOONG_ERROR("Cannot load font file '{}', exit...", kIconFontPath);
        return 1;
    }

    static std::vector<uint8_t> buffer(size1 + size2);
    if (!Loong::FS::LoongFileSystem::LoadFileContent(kTextFontPath, buffer.data(), size1)) {
        LOONG_ERROR("Cannot load font file '{}', exit...", kTextFontPath);
        return 1;
    }
    if (!Loong::FS::LoongFileSystem::LoadFileContent(kIconFontPath, buffer.data() + size1, size2)) {
        LOONG_ERROR("Cannot load font file '{}', exit...", kIconFontPath);
        return 1;
    }

    // static const ImWchar ranges[] = {
    //     0x0020,
    //     0x00FF, // Basic Latin + Latin Supplement
    //     0x2000,
    //     0x206F, // General Punctuation
    //     0x3000,
    //     0x30FF, // CJK Symbols and Punctuations, Hiragana, Katakana
    //     0x31F0,
    //     0x31FF, // Katakana Phonetic Extensions
    //     0xFF00,
    //     0xFFEF, // Half-width characters
    //     0x4e00,
    //     0x9FAF, // CJK Ideograms
    //     0,
    // };
    // static ImWchar ranges[] = { 0x20, 0x52f, 0x1ab0, 0x2189, 0x2c60, 0x2e44, 0xa640, 0xab65, 0 };
    static ImWchar fontAwesomeIconRanges[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };
    ImFontConfig cfg;
    cfg.MergeMode = false;
    cfg.FontDataOwnedByAtlas = true;
    cfg.PixelSnapH = true;
    cfg.OversampleH = 1; //or 2 is the same
    cfg.OversampleV = 1;
    auto& io = ImGui::GetIO();
    ImGui::GetIO().Fonts->AddFontFromMemoryTTF(buffer.data(), int(size1), 16, &cfg, io.Fonts->GetGlyphRangesChineseFull());
    cfg.MergeMode = true;
    cfg.FontDataOwnedByAtlas = false;
    io.Fonts->AddFontFromMemoryTTF(buffer.data() + size1, int(size2), 16, &cfg, fontAwesomeIconRanges);

    return 0;
}

int StartApp(int argc, char** argv)
{

    Loong::App::LoongApp::WindowConfig config {};
    config.title = "LoongEditor";
    std::shared_ptr<Loong::App::LoongApp> app = std::make_shared<Loong::App::LoongApp>(config);

    if (0 != LoadFonts()) {
        return 1;
    }

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
    auto listener = Loong::Foundation::Logger::Get().SubscribeLog([](const Loong::Foundation::LogItem& logItem) {
        std::cout << "[" << logItem.level << "][" << logItem.location << "]: " << logItem.message << std::endl;
    });
    Loong::App::ScopedDriver appDriver;

    auto path = Loong::Foundation::LoongPathUtils::GetParent(argv[0]) + "/Resources";
    Loong::FS::ScopedDriver fsDriver(argv[0]);
    Loong::FS::LoongFileSystem::MountSearchPath("/Users/chenchen02/gitrepo/Loong/Resources");
//    path = Loong::Foundation::LoongPathUtils::Normalize(argv[0]) + "/../../Resources";
//    Loong::FS::LoongFileSystem::MountSearchPath(path);

    Loong::Resource::ScopedDriver resourceDriver;

    StartApp(argc, argv);

    return 0;
}