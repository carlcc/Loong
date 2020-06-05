cmake_minimum_required(VERSION 3.2)
project(imgui)

add_library(imgui STATIC
        imconfig.h
        imgui.cpp
        imgui.h
        imgui_demo.cpp
        imgui_draw.cpp
        imgui_internal.h
        imgui_widgets.cpp
        imstb_rectpack.h
        imstb_textedit.h
        imstb_truetype.h
        imgui_stdlib.cpp
        imgui_stdlib.h
        )

target_include_directories(imgui
PUBLIC
        .
        )