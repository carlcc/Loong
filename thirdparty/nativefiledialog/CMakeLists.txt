


if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set(NFD_PLATFORM_FILES src/nfd_gtk.c)
    FIND_PACKAGE(PkgConfig REQUIRED)
    PKG_CHECK_MODULES(NDF_PLATFORM_DEPS REQUIRED gtk+-3.0)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(NFD_PLATFORM_FILES src/nfd_cocoa.m)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    set(NFD_PLATFORM_FILES src/nfd_win.cpp)
else ()
    message(FATAL_ERROR "NFD Unsupported platform: ${CMAKE_SYSTEM_NAME}")
endif ()

# src/nfd_zenity.c
add_library(nfd STATIC
        src/common.h
        src/nfd_common.c
        src/nfd_common.h
        src/simple_exec.h
        ${NFD_PLATFORM_FILES}
        )
target_include_directories(nfd
PUBLIC
        src/include
	${NDF_PLATFORM_DEPS_INCLUDE_DIRS}
        )
target_link_libraries(
        nfd PUBLIC ${NDF_PLATFORM_DEPS_LIBRARIES}
)
