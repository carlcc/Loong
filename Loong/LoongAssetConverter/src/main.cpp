#include <iostream>
#include "LoongFoundation/LoongLogger.h"
#include "Flags.h"

int Convert()
{
    return 0;
}

int main(int argc, char* argv[])
{
    using namespace Loong::Foundation;
    using namespace Loong::AssetConverter;

    auto listener = Logger::Get().SubscribeLog([](const LogItem& logItem) {
        std::cout << "[" << logItem.level << "][" << logItem.location << "]: " << logItem.message << std::endl;
    });

    if (!Flags::ParseCommandLine(argc, argv)) {
        LOONG_ERROR("Parse command line failed!");
        return -1;
    }

    return Convert();
}