//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#include "LoongGui/LoongGuiInflator.h"
#include "LoongFileSystem/LoongFileSystem.h"
#include "LoongFoundation/LoongAssert.h"
#include "LoongFoundation/LoongFormat.h"
#include "LoongFoundation/LoongLogger.h"
#include "LoongFoundation/LoongStringUtils.h"
#include "LoongGui/LoongGuiButton.h"
#include "LoongGui/LoongGuiImage.h"
#include "LoongGui/LoongGuiWindow.h"
#include "LoongResource/LoongResourceManager.h"
#include <pugixml.hpp>
#include <unordered_map>

namespace Loong::Gui {

using GuiWidgetCreator = std::function<std::shared_ptr<LoongGuiWidget>(LoongGuiContainer*)>;

#define REGISTER_GUI_CREATOR(GuiWidgetType) creators_.insert({ GuiWidgetType::GetTypeNameStatic(), [](LoongGuiContainer* parent) { return parent == nullptr ? Gui::MakeGuiWidget<GuiWidgetType>() : parent->AddChild<GuiWidgetType>(); } });

class LoongGuiClassRegistry {
public:
    static const LoongGuiClassRegistry& Get()
    {
        static LoongGuiClassRegistry r;
        return r;
    }

    LG_NODISCARD std::shared_ptr<LoongGuiWidget> Create(const std::string& name, LoongGuiContainer* parent) const
    {
        auto it = creators_.find(name);
        if (it == creators_.end()) {
            LOONG_ERROR("Unknown GUI widget '{}'", name);
            return nullptr;
        }
        return it->second(parent);
    }

private:
    LoongGuiClassRegistry()
    {
        REGISTER_GUI_CREATOR(LoongGuiButton);
        creators_.insert({ LoongGuiWindow::GetTypeNameStatic(),
            [](LoongGuiContainer* parent) -> std::shared_ptr<LoongGuiWidget> {
                if (parent != nullptr) {
                    LOONG_ERROR("Window should not have a parent");
                    return nullptr;
                }
                return Gui::MakeGuiWidget<LoongGuiWindow>();
            } });
        REGISTER_GUI_CREATOR(LoongGuiImage);
    }

private:
    std::unordered_map<std::string, GuiWidgetCreator> creators_ {};
};

enum class ProcessResult : uint8_t {
    kOk,
    kError,
    kNotProcessed
};

static std::shared_ptr<LoongGuiWidget> ParseXmlNode(pugi::xml_node node, LoongGuiContainer* parent);

#define CHECK_COMMA_SEPARATED(outputVar, val, substrCount, errorMsg)                         \
    std::vector<std::string_view> outputVar = Foundation::LoongStringUtils::Split(val, ','); \
    if (outputVar.size() != substrCount) {                                                   \
        LOONG_ERROR(errorMsg);                                                               \
        return ProcessResult::kError;                                                        \
    }
#define CHECK_TRUE_FALSE(isTrue, key, val)                                    \
    bool isTrue = strcasecmp(val, "true") == 0;                               \
    if (!isTrue && strcasecmp(val, "false") != 0) {                           \
        LOONG_ERROR("{}'s value is neither 'true' nor 'false', ignore", key); \
        return ProcessResult::kOk;                                            \
    }

static ProcessResult ParseWidgetAttributes(pugi::xml_attribute& attr, LoongGuiWidget* widget)
{
    if (strcmp(attr.name(), "label") == 0) {
        widget->SetLabel(attr.value());
        return ProcessResult::kOk;
    } else if (strcmp(attr.name(), "size") == 0) {
        CHECK_COMMA_SEPARATED(sizeProps, attr.value(), 2, "size property should be width and height separated by a comma(,)");

        auto w = Foundation::LoongStringUtils::ParseTo<float>(sizeProps[0]);
        auto h = Foundation::LoongStringUtils::ParseTo<float>(sizeProps[1]);
        widget->SetSize({ w, h });
        return ProcessResult::kOk;
    } else if (strcmp(attr.name(), "pos") == 0) {
        CHECK_COMMA_SEPARATED(posProps, attr.value(), 2, "pos property should be x and y separated by a comma(,)");

        auto x = Foundation::LoongStringUtils::ParseTo<float>(posProps[0]);
        auto y = Foundation::LoongStringUtils::ParseTo<float>(posProps[1]);
        widget->SetPosition({ x, y });
        return ProcessResult::kOk;
    } else if (strcmp(attr.name(), "name") == 0) {
        widget->SetName(attr.value());
        return ProcessResult::kOk;
    }

    return ProcessResult::kNotProcessed;
}

static ProcessResult ParseWindowAttributes(pugi::xml_attribute& attr, LoongGuiWidget* widget)
{
    if (!widget->IsInstanceOf<LoongGuiWindow>()) {
        return ProcessResult::kNotProcessed;
    }
    auto* win = static_cast<LoongGuiWindow*>(widget); // NOLINT(cppcoreguidelines-pro-type-static-cast-downcast)
    if (strcmp(attr.name(), "resizable") == 0) {
        CHECK_TRUE_FALSE(isTrue, attr.name(), attr.value());
        win->SetResizable(isTrue);
        return ProcessResult::kOk;
    } else if (strcmp(attr.name(), "closable") == 0) {
        CHECK_TRUE_FALSE(isTrue, attr.name(), attr.value());
        win->SetClosable(isTrue);
        return ProcessResult::kOk;
    } else if (strcmp(attr.name(), "movable") == 0) {
        CHECK_TRUE_FALSE(isTrue, attr.name(), attr.value());
        win->SetMovable(isTrue);
        return ProcessResult::kOk;
    } else if (strcmp(attr.name(), "scrollable") == 0) {
        CHECK_TRUE_FALSE(isTrue, attr.name(), attr.value());
        win->SetScrollable(isTrue);
        return ProcessResult::kOk;
    } else if (strcmp(attr.name(), "dockable") == 0) {
        CHECK_TRUE_FALSE(isTrue, attr.name(), attr.value());
        win->SetDockable(isTrue);
        return ProcessResult::kOk;
    } else if (strcmp(attr.name(), "hasTitleBar") == 0) {
        CHECK_TRUE_FALSE(isTrue, attr.name(), attr.value());
        win->SetHasTitleBar(isTrue);
        return ProcessResult::kOk;
    } else if (strcmp(attr.name(), "hasBackground") == 0) {
        CHECK_TRUE_FALSE(isTrue, attr.name(), attr.value());
        win->SetHasBackground(isTrue);
        return ProcessResult::kOk;
    } else if (strcmp(attr.name(), "allowInputs") == 0) {
        CHECK_TRUE_FALSE(isTrue, attr.name(), attr.value());
        win->SetAllowInputs(isTrue);
        return ProcessResult::kOk;
    } else if (strcmp(attr.name(), "collapsable") == 0) {
        CHECK_TRUE_FALSE(isTrue, attr.name(), attr.value());
        win->SetCollapsable(isTrue);
        return ProcessResult::kOk;
    }

    return ProcessResult::kNotProcessed;
}

static ProcessResult ParseImageAttributes(pugi::xml_attribute& attr, LoongGuiWidget* widget)
{
    if (!widget->IsInstanceOf<LoongGuiImage>()) {
        return ProcessResult::kNotProcessed;
    }
    auto* img = static_cast<LoongGuiImage*>(widget); // NOLINT(cppcoreguidelines-pro-type-static-cast-downcast)
    if (strcmp(attr.name(), "texture") == 0) {
        Resource::LoongResourceManager::GetTextureAsync(attr.value()).Then([img](const tpl::Task<Resource::LoongTextureRef>& task) {
            auto tex = task.GetFuture().GetValue();
            img->SetTexture(tex);
        });
        return ProcessResult::kOk;
    }
    return ProcessResult::kNotProcessed;
}

static ProcessResult ProcessChildren(pugi::xml_node node, LoongGuiWidget* widget)
{
    if (node.begin() == node.end()) {
        return ProcessResult::kOk;
    }

    if (!widget->IsInstanceOf<LoongGuiContainer>()) {
        LOONG_WARNING("'{}' is not a container, it should not have any child!");
        return ProcessResult::kOk;
    }
    auto* container = static_cast<LoongGuiContainer*>(widget); // NOLINT(cppcoreguidelines-pro-type-static-cast-downcast)

    for (auto child : node.children()) {
        if (nullptr == ParseXmlNode(child, container)) {
            return ProcessResult::kError;
        }
    }
    return ProcessResult::kOk;
}

static ProcessResult ParseAttributes(pugi::xml_node node, LoongGuiWidget* widget)
{
    for (auto& attr : node.attributes()) {
        if (auto res = ParseWidgetAttributes(attr, widget); res == ProcessResult::kError) {
            return ProcessResult::kError;
        } else if (res == ProcessResult::kOk) {
            continue;
        }
        if (auto res = ParseWindowAttributes(attr, widget); res == ProcessResult::kError) {
            return ProcessResult::kError;
        } else if (res == ProcessResult::kOk) {
            continue;
        }
        if (auto res = ParseImageAttributes(attr, widget); res == ProcessResult::kError) {
            return ProcessResult::kError;
        } else if (res == ProcessResult::kOk) {
            continue;
        }

        LOONG_WARNING("Unknown attribute '{}' for node '{}'", attr.name(), node.name());
    }

    if (auto res = ProcessChildren(node, widget); res == ProcessResult::kError) {
        return ProcessResult::kError;
    }
    return ProcessResult::kOk;
}

static std::shared_ptr<LoongGuiWidget> ParseXmlNode(pugi::xml_node node, LoongGuiContainer* parent)
{
    auto widget = LoongGuiClassRegistry::Get().Create(node.name(), parent);
    if (widget == nullptr) {
        return nullptr;
    }

    ParseAttributes(node, widget.get());
    return widget;
}

std::shared_ptr<LoongGuiWidget> LoongGuiInflator::Inflate(const std::string& vfsPath)
{
    auto fileSize = FS::LoongFileSystem::GetFileSize(vfsPath);
    std::vector<uint8_t> buffer;
    buffer.resize(fileSize);
    fileSize = FS::LoongFileSystem::LoadFileContent(vfsPath, buffer.data(), buffer.size());

    if (fileSize != buffer.size()) {
        LOONG_ERROR("Load GUI file '{}' failed: Read file failed", vfsPath);
        return nullptr;
    }

    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_buffer(buffer.data(), buffer.size());
    if (!result) {
        LOONG_ERROR("Load GUI file '{}' failed: parse xml failed", vfsPath);
        return nullptr;
    }

    auto node = doc.root().first_child();
    if (node.empty()) {
        LOONG_ERROR("Load GUI file '{}' failed: empty gui file", vfsPath);
        return nullptr;
    }
    return ParseXmlNode(node, nullptr);
}
}