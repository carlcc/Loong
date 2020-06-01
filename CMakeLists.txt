cmake_minimum_required(VERSION 3.2)

project(Loong)

add_subdirectory(thirdparty)

set(CMAKE_CXX_STANDARD 17)

function(set_loong_common_target_properties TARGET)

    if(COMMAND custom_configure_target)
        custom_configure_target(${TARGET})
        if(TARGET_CONFIGURATION_COMPLETE)
            return()
        endif()
    endif()

    get_target_property(TARGET_TYPE ${TARGET} TYPE)

    if(MSVC)
        # For msvc, enable link-time code generation for release builds (I was not able to
        # find any way to set these settings through interface library BuildSettings)
        if(TARGET_TYPE STREQUAL STATIC_LIBRARY)

            foreach(REL_CONFIG ${RELEASE_CONFIGURATIONS})
                set_target_properties(${TARGET} PROPERTIES
                    STATIC_LIBRARY_FLAGS_${REL_CONFIG} /LTCG
                )
            endforeach()

        else()

            foreach(REL_CONFIG ${RELEASE_CONFIGURATIONS})
                set_target_properties(${TARGET} PROPERTIES
                    LINK_FLAGS_${REL_CONFIG} "/LTCG /OPT:REF /INCREMENTAL:NO"
                )
            endforeach()

            if(PLATFORM_UNIVERSAL_WINDOWS)
                # On UWP, disable incremental link to avoid linker warnings
                foreach(DBG_CONFIG ${DEBUG_CONFIGURATIONS})
                    set_target_properties(${TARGET} PROPERTIES
                        LINK_FLAGS_${DBG_CONFIG} "/INCREMENTAL:NO"
                    )
                endforeach()
            endif()
        endif()
    else()
        set_target_properties(${TARGET} PROPERTIES
            CXX_VISIBILITY_PRESET hidden # -fvisibility=hidden
            C_VISIBILITY_PRESET hidden # -fvisibility=hidden
            VISIBILITY_INLINES_HIDDEN TRUE

            # Without -fPIC option GCC fails to link static libraries into dynamic library:
            #  -fPIC
            #      If supported for the target machine, emit position-independent code, suitable for
            #      dynamic linking and avoiding any limit on the size of the global offset table.
            POSITION_INDEPENDENT_CODE ON
        )

        if(NOT MINGW_BUILD)
            # Do not disable extensions when building with MinGW!
            set_target_properties(${TARGET} PROPERTIES
                CXX_EXTENSIONS OFF
            )
        endif()
    endif()

endfunction()


function(add_loong_app APP_NAME IDE_FOLDER SOURCE INCLUDE ASSETS)

    set(ALL_ASSETS ${ASSETS})
    add_target_platform_app(${APP_NAME} "${SOURCE}" "${INCLUDE}" "${ALL_ASSETS}")

    set_source_files_properties(${ALL_ASSETS} PROPERTIES
            VS_DEPLOYMENT_LOCATION "."
            MACOSX_PACKAGE_LOCATION "Resources"
            )

    if(PLATFORM_WIN32)
        set_target_properties(${APP_NAME} PROPERTIES
                VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/assets"
                )

        copy_required_dlls(${APP_NAME})
        add_custom_command(TARGET PlayGround POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "\"$<TARGET_FILE:gainput>\""
                "\"$<TARGET_FILE_DIR:${APP_NAME}>\"")

        append_loong_app_base_win32_source(PlayGround)
    endif()

    target_include_directories(${APP_NAME}
            PRIVATE
            src
            )

    target_link_libraries(${APP_NAME}
            PRIVATE
            # On Linux we must have Diligent-NativeAppBase go first, otherwise the linker
            # will fail to resolve Diligent::CreateApplication() function.
            Diligent-NativeAppBase
            Diligent-BuildSettings
            LoonAppBase
            )
    set_common_target_properties(${APP_NAME})

    if(MSVC)
        # Disable MSVC-specific warnings
        # - w4201: nonstandard extension used: nameless struct/union
        target_compile_options(${APP_NAME} PRIVATE /wd4201)
    endif()

    set_target_properties(${APP_NAME} PROPERTIES
            FOLDER ${IDE_FOLDER}
            )

    source_group("src" FILES ${SOURCE} ${INCLUDE})
    source_group("assets" FILES ${ALL_ASSETS})

    target_sources(${APP_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/readme.md")
    set_source_files_properties(
            "${CMAKE_CURRENT_SOURCE_DIR}/readme.md" PROPERTIES HEADER_FILE_ONLY TRUE
    )

    if(PLATFORM_WIN32 OR PLATFORM_LINUX)
        # Copy assets to target folder
        add_custom_command(TARGET ${APP_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_directory
                "${CMAKE_CURRENT_SOURCE_DIR}/assets"
                "\"$<TARGET_FILE_DIR:${APP_NAME}>\"")
    endif()

endfunction()

add_subdirectory(Loong)
