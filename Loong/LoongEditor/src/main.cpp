#include "glad/glad.h"

#include "LoongApp/Driver.h"
#include "LoongFileSystem/Driver.h"
#include "LoongResource/Driver.h"

#include "LoongApp/LoongApp.h"
#include "LoongEditor.h"
#include "LoongFoundation/LoongLogger.h"
#include "LoongFoundation/LoongPathUtils.h"
#include <iostream>

void StartApp()
{

    Loong::App::LoongApp::WindowConfig config {};
    config.title = "LoongEditor";
    std::shared_ptr<Loong::App::LoongApp> app = std::make_shared<Loong::App::LoongApp>(config);

    Loong::Editor::LoongEditor editor(app.get());

    app->Run();
}

int main(int argc, char** argv)
{
    Loong::App::ScopedDriver appDriver;

    auto path = Loong::Foundation::LoongPathUtils::GetParent(argv[0]) + "/Resources";
    Loong::FS::ScopedDriver fsDriver(argv[0]);
    Loong::FS::LoongFileSystem::MountSearchPath(path);

    Loong::Resource::ScopedDriver resourceDriver;

    auto listener = Loong::Foundation::Logger::Get().SubscribeLog([](const Loong::Foundation::LogItem& logItem) {
        std::cout << "[" << logItem.level << "][" << logItem.location << "]: " << logItem.message << std::endl;
    });

    StartApp();

    return 0;
}